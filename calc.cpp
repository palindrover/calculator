#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <numbers>
#include <unistd.h>

/*

Грамматика

expr -> term ( ('+' | '-') term )*
term -> unary ( ('*' | '/' | '%') unary )*
unary -> ('+' | '-')* power
power -> primary [ '^' unary ]
primary -> Number | Ident | '(' expr ')'

*/

bool interactive = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);

enum Tag
{
	Number,
	Ident,
	Plus,
	Minus,
	Star,
	Slash,
	Caret,
	Mod,
	LParen,
	RParen,
	End,
	Error
};

const std::unordered_map<std::string, double> Idents
{
	{"pi", std::numbers::pi},
	{"e", std::numbers::e}
};

class Token
{
public:
	
	std::size_t pos, len;
	Tag tag;
	std::string text;
};

static inline bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

static inline bool isLetter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

std::vector<Token> lexer(const std::string& str)
{
	std::vector<Token> tokens;

	for(std::size_t i = 0; i < str.size(); i++)
	{
		switch (str[i]) 
		{
		case '(':
			tokens.push_back(Token{i, 1, LParen, "("});
			break;

		case ')':
			tokens.push_back(Token{i, 1, RParen, ")"});
			break;

		case '+':
			tokens.push_back(Token{i, 1, Plus, "+"});
			break;

		case '-':
			tokens.push_back(Token{i, 1, Minus, "-"});
			break;

		case '*':
			tokens.push_back(Token{i, 1, Star, "*"});
			break;

		case '/':
			tokens.push_back(Token{i, 1, Slash, "/"});
			break;

		case '^':
			tokens.push_back(Token{i, 1, Caret, "^"});
			break;

		case '%':
			tokens.push_back(Token{i, 1, Mod, "%"});
			break;

		case ' ':
			
		case '\t':
			break;

		default:
			if(isLetter(str[i]))
			{
				std::size_t start = i;

				while(i < str.size() && (isLetter(str[i]) || isDigit(str[i]) || 
							str[i] == '_')) i++;

				tokens.push_back(Token{start, i - start, Ident, 
						str.substr(start, i - start)});
				i--;
			}
			else if(isDigit(str[i]))
			{
				std::size_t start = i;

				while(i < str.size() && isDigit(str[i])) i++;

				if(i < str.size() && str[i] == '.')
				{
					i++;
					while(i < str.size() && isDigit(str[i]))i++;
				}

				if(i < str.size() && (str[i] == 'e' || str[i] == 'E'))
				{
					std::size_t beforeExp = i;
					i++;

					if(i < str.size() && (str[i] == '+' || str[i] == '-')) i++;

					if(i < str.size() && isDigit(str[i]))
						while(i < str.size() && isDigit(str[i])) i++;
					else
						i = beforeExp;
				}
				
				tokens.push_back(Token{start, i - start, Number,
							str.substr(start, i - start)});
				i--;
			}
			else tokens.push_back(Token{i, 1, Error, std::string(1, str[i])});
		}
	}
	tokens.push_back(Token{str.size(), 0, End, ""});

	return tokens;
}

std::string tagName(Tag tag)
{
	switch (tag)
	{
	case Number: return "Number";
	case Ident: return "Ident";
	case Plus: return "Plus";
	case Minus: return "Minus";
	case Star: return "Star";
	case Slash: return "Slash";
	case Caret: return "Caret";
	case Mod: return "Mod";
	case LParen: return "LParen";
	case RParen: return "RParen";
	case End: return "End";
	case Error: return "Error";
	}
	return "";
}

std::string tagChar(Tag tag)
{
	switch (tag)
	{
	case Number: return "Number";
	case Ident: return "Ident";
	case Plus: return "+";
	case Minus: return "-";
	case Star: return "*";
	case Slash: return "/";
	case Caret: return "^";
	case Mod: return "%";
	case LParen: return "(";
	case RParen: return ")";
	case End: return "End";
	case Error: return "Error";
	}
	return "";
}

struct ParseError : std::runtime_error
{
	std::size_t pos;
	ParseError(const std::string& msg, std::size_t pos)
		: std::runtime_error(msg), pos(pos) {}
};

double checked(double x, std::size_t pos)
{
	if(std::isnan(x) || std::isinf(x)) throw ParseError("переполнение", pos);
	return x;
}

class Parser
{
public:
	explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}
	double parse()
	{
		double value = parseExpr();
		if(current().tag != End)
			throw ParseError("лишние символы после выражения", current().pos);
		return value;
	}

private:
	std::vector<Token> tokens;
	std::size_t pos = 0;

	const Token& current()
	{
		return tokens[pos];
	}

	void advance()
	{
		if(current().tag != End) pos++;
	}

	void expect(Tag tag)
	{
		if(current().tag != tag)
			throw ParseError(std::string("ожидалось ") + tagChar(tag), current().pos);
		else advance();
	}

	double parsePower()
	{
		double result = parsePrimary();

		if(current().tag == Caret)
		{
			std::size_t operatorPos = current().pos;
			advance();
			result = std::pow(result, parseUnary());
			if(std::isnan(result) || std::isinf(result))
				throw ParseError("невозможное возведение в степень", operatorPos);
		}

		return result;
	}

	double parseUnary()
	{
		int cnt = 0;
		while(current().tag == Plus || current().tag == Minus)
		{
			if(current().tag == Minus) cnt++;
			advance();
		}
		return cnt % 2 ? -1 * parsePower() : parsePower();
	}

	double parseTerm()
	{
		double result = parseUnary();

		while(current().tag == Star || current().tag == Slash || current().tag == Mod)
		{
			Tag operatorTag = current().tag;
			std::size_t operatorPos = current().pos;
			advance();
			double rhs = parseUnary();

			switch (operatorTag) 
			{
			case Star:
				result = checked(result * rhs, operatorPos);
				break;
			case Slash:
				if(rhs == 0) throw ParseError("деление на ноль", operatorPos);
				result = checked(result / rhs, operatorPos);
				break;
			case Mod:
				if(rhs == 0) throw ParseError("деление на ноль", operatorPos);
				result = checked(std::fmod(result, rhs), operatorPos);
				break;
			default:
				break;
			}
		}

		return result;
	}

	double parseExpr()
	{
		double result = parseTerm();

		while(current().tag == Plus || current().tag == Minus)
		{
			Tag operatorTag = current().tag;
			std::size_t operatorPos = current().pos;
			advance();
			double rhs = parseTerm();

			switch (operatorTag)
			{
			case Plus:
				result = checked(result + rhs, operatorPos);
				break;
			case Minus:
				result = checked(result - rhs, operatorPos);
				break;
			default:
				break;
			}
		}

		return result;
	}

	double parsePrimary()
	{
		double value;

		switch (current().tag)
		{
		case Number:
			value = checked(std::strtod(current().text.c_str(), nullptr), current().pos);
			advance();
			return value;

		case Ident:
		{
			auto it = Idents.find(current().text);
			if(it == Idents.end())throw ParseError(std::string("неизвестная константа ") +
					current().text, current().pos);
			value = it->second;
			advance();
			return value;
		}

		case LParen:
			advance();
			value = parseExpr();
			expect(RParen);
			return value;

		case Error:
			throw ParseError(std::string("неизвестный символ ") + current().text, 
					current().pos);

		case End:
			throw ParseError("неожиданный конец выражения", current().pos);

		default:
			throw ParseError("ожидалось число или (", current().pos);
		}
	}

};

int main()
{
	std::string s;
	std::cout << std::setprecision(15);

	while(std::getline(std::cin, s) && s != "exit")
	{
		try
		{
			Parser parser(lexer(s));
			double result = parser.parse();

			if(interactive)
				std::cout << "\x1b[A\r" << s << " = " << result << "\n\n";
			else
				std::cout << result << "\n\n";
		}
		catch (const ParseError& e)
		{
			if(interactive)
				std::cout /*<< s << '\n'*/ << std::string(e.pos, ' ') << "^\n" 
					<< "Ошибка: " << e.what() << "\n\n";
			else
				std::cout << s << '\n' << std::string(e.pos, ' ') << "^\n" 
					<< "Ошибка: " << e.what() << "\n\n";

		}
	}

	return 0;
}
