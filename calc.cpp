#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <cstddef>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
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

using Env = std::unordered_map<std::string, double>;

bool interactive = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);

enum class Tag
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

enum class BinOp
{
	Add,
	Sub,
	Mul,
	Div,
	Mod,
	Pow
};

enum class UnOp
{
	Neg,
	Plus
};

const Env Idents
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
			tokens.push_back(Token{i, 1, Tag::LParen, "("});
			break;

		case ')':
			tokens.push_back(Token{i, 1, Tag::RParen, ")"});
			break;

		case '+':
			tokens.push_back(Token{i, 1, Tag::Plus, "+"});
			break;

		case '-':
			tokens.push_back(Token{i, 1, Tag::Minus, "-"});
			break;

		case '*':
			tokens.push_back(Token{i, 1, Tag::Star, "*"});
			break;

		case '/':
			tokens.push_back(Token{i, 1, Tag::Slash, "/"});
			break;

		case '^':
			tokens.push_back(Token{i, 1, Tag::Caret, "^"});
			break;

		case '%':
			tokens.push_back(Token{i, 1, Tag::Mod, "%"});
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

				tokens.push_back(Token{start, i - start, Tag::Ident, 
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
				
				tokens.push_back(Token{start, i - start, Tag::Number,
							str.substr(start, i - start)});
				i--;
			}
			else tokens.push_back(Token{i, 1, Tag::Error, std::string(1, str[i])});
		}
	}
	tokens.push_back(Token{str.size(), 0, Tag::End, ""});

	return tokens;
}

std::string tagName(Tag tag)
{
	using enum Tag;
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
	using enum Tag;
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

struct PosError : std::runtime_error
{
	std::size_t pos, len;
	PosError(const std::string& msg, std::size_t pos, std::size_t len)
		: std::runtime_error(msg), pos(pos), len(len) {}
};

struct ParseError : PosError
{
	using PosError::PosError;
};

struct EvalError : PosError
{
	using PosError::PosError;
};

double checked(double x, std::size_t pos, std::size_t len)
{
	if(std::isnan(x) || std::isinf(x)) throw EvalError("переполнение", pos, len);
	return x;
}


static inline UnOp tagToUnary(Tag tag)
{
	switch (tag) 
	{
	case Tag::Minus:
		return UnOp::Neg;
	case Tag::Plus:
		return UnOp::Plus;
	default:
		break;
	}
	throw std::logic_error("неизвестный оператор");
}

static inline BinOp tagToBinary(Tag tag)
{
	switch (tag) 
	{
	case Tag::Plus:
		return BinOp::Add;
	case Tag::Minus:
		return BinOp::Sub;
	case Tag::Star:
		return BinOp::Mul;
	case Tag::Slash:
		return BinOp::Div;
	case Tag::Mod:
		return BinOp::Mod;
	case Tag::Caret:
		return BinOp::Pow;
	default:
		break;
	}
	throw std::logic_error("неизвестный оператор");
}

static inline char opSymbol(BinOp op)
{
	using enum BinOp;
	switch(op)
	{
	case Add:
		return '+';
	case Sub:
		return '-';
	case Mul:
		return '*';
	case Div:
		return '/';
	case Mod:
		return '%';
	case Pow:
		return '^';
		break;
	}
	throw std::logic_error("неизвестный оператор");
}

struct Node;

struct BinaryNode
{
	BinOp op;
	std::unique_ptr<Node> left, right;
	std::size_t pos, len;
};

struct UnaryNode
{
	UnOp op;
	std::unique_ptr<Node> operand;
	std::size_t pos, len;
};

struct NumberNode
{
	std::string text;
	std::size_t pos, len;
};

struct IdentNode
{
	std::string text;
	std::size_t pos, len;
};

struct Node
{
	std::variant<NumberNode, IdentNode, UnaryNode, BinaryNode> v;
};

struct PrintVisitor
{
	std::ostream& out;

	void operator()(const NumberNode& node) const {out << node.text;}
	void operator()(const IdentNode& node) const {out << node.text;}
	void operator()(const UnaryNode& node) const
	{
		switch(node.op)
		{
		case UnOp::Neg:
			out << "(-";
			std::visit(*this, node.operand->v);
			out << ")";
			break;
		case UnOp::Plus:
			std::visit(*this, node.operand->v);
		}
	}
	void operator()(const BinaryNode& node) const
	{
		out << "(" << opSymbol(node.op) << " ";
		std::visit(*this, node.left->v);
		out << " ";
		std::visit(*this, node.right->v);
		out << ")";
	}
};

struct EvalVisitor
{
	const Env& env;

	double operator()(const NumberNode& node) const 
	{
		return checked(std::strtod(node.text.c_str(), nullptr), node.pos, node.len);
	}
	double operator()(const IdentNode& node)const
	{
		auto it = env.find(node.text);
		if(it == env.end()) throw EvalError(std::string("неизвестный идентификатор ") + 
				node.text, node.pos, node.len);
		return it->second;
	}
	double operator()(const UnaryNode& node) const
	{
		double val = std::visit(*this, node.operand->v);
		switch (node.op) 
		{
		case UnOp::Plus:
			return val;
		case UnOp::Neg:
			return -val;
		}
		throw std::logic_error("неизвестный оператор");
	}
	double operator()(const BinaryNode& node) const
	{
		using enum BinOp;
		
		double leftVal = std::visit(*this, node.left->v), 
			   rightVal = std::visit(*this, node.right->v);

		switch (node.op)
		{
		case Add:
			return checked(leftVal + rightVal, node.pos, node.len);
		case Sub:
			return checked(leftVal - rightVal, node.pos, node.len);
		case Mul:
			return checked(leftVal * rightVal, node.pos, node.len);
		case Div:
			if(rightVal == 0) throw EvalError("деление на ноль", node.pos, node.len);
			return checked(leftVal / rightVal, node.pos, node.len);
		case Mod:
			if(rightVal == 0) throw EvalError("деление на ноль", node.pos, node.len);
			return checked(std::fmod(leftVal, rightVal), node.pos, node.len);
		case Pow:
			{
				double result = std::pow(leftVal, rightVal);
				if(std::isnan(result)) 
					throw EvalError("результат не определён", node.pos, node.len);
				return checked(result, node.pos, node.len);
			}
		}

		throw std::logic_error("неизвестный оператор");
	}
};

class Parser
{
public:
	explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}
	std::unique_ptr<Node> parse()
	{
		std::unique_ptr<Node> value = parseExpr();
		if(current().tag != Tag::End)
			throw ParseError("лишние символы после выражения", current().pos,
					current().len);
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
		if(current().tag != Tag::End) pos++;
	}

	void expect(Tag tag)
	{
		if(current().tag != tag)
			throw ParseError(std::string("ожидалось ") + tagChar(tag), current().pos,
					current().len);
		else advance();
	}

	std::unique_ptr<Node> parsePower()
	{
		std::unique_ptr<Node> result = parsePrimary();

		if(current().tag == Tag::Caret)
		{
			BinOp op = tagToBinary(current().tag);
			std::size_t opPos = current().pos, opLen = current().len;
			advance();

			result = std::make_unique<Node>(Node{BinaryNode{op, std::move(result),
					parseUnary(), opPos, opLen}});
		}

		return result;
	}

	std::unique_ptr<Node> parseUnary()
	{
		int cnt = 0;
		std::size_t opPos = current().pos, opLen = 1;

		while(current().tag == Tag::Plus || current().tag == Tag::Minus)
		{
			if(current().tag == Tag::Minus) cnt++;
			opLen = current().pos - opPos;
			advance();
		}

		return std::make_unique<Node>(Node{UnaryNode{cnt % 2 ? UnOp::Neg : UnOp::Plus,
				parsePower(), opPos, opLen}});
	}

	std::unique_ptr<Node> parseTerm()
	{
		std::unique_ptr<Node> result = parseUnary();

		while(current().tag == Tag::Star || current().tag == Tag::Slash ||
				current().tag == Tag::Mod)
		{
			BinOp op = tagToBinary(current().tag);
			std::size_t opPos = current().pos, opLen = current().len;
			advance();

			result = std::make_unique<Node>(Node{BinaryNode{op, std::move(result), 
					parseUnary(), opPos, opLen}});
		}

		return result;
	}

	std::unique_ptr<Node> parseExpr()
	{
		std::unique_ptr<Node> result = parseTerm();

		while(current().tag == Tag::Plus || current().tag == Tag::Minus)
		{
			BinOp op = tagToBinary(current().tag);
			std::size_t opPos = current().pos, opLen = current().len;
			advance();
			
			result = std::make_unique<Node>(Node{BinaryNode{op, std::move(result), 
					parseTerm(), opPos, opLen}});
		}

		return result;
	}

	std::unique_ptr<Node> parsePrimary()
	{
		std::unique_ptr<Node> node;

		switch (current().tag)
		{
		case Tag::Number:
			node = std::make_unique<Node>(Node{NumberNode{current().text,
					current().pos, current().len}});
			advance();
			return node;

		case Tag::Ident:
			node = std::make_unique<Node>(Node{IdentNode{current().text,
					current().pos, current().len}});
			advance();
			return node;

		case Tag::LParen:
			advance();
			node = parseExpr();
			expect(Tag::RParen);
			return node;

		case Tag::Error:
			throw ParseError(std::string("неизвестный символ ") + current().text, 
					current().pos, current().len);

		case Tag::End:
			throw ParseError("неожиданный конец выражения", current().pos,
					current().len);

		default:
			throw ParseError("ожидалось число или (", current().pos, current().len);
		}
	}

};

void print(const Node& node, std::ostream& out)
{
	std::visit(PrintVisitor{out}, node.v);
}

double eval(const Node& node, const Env& env)
{
	return std::visit(EvalVisitor{env}, node.v);
}

int main()
{
	std::string s;
	std::cout << std::setprecision(15);

	while(std::getline(std::cin, s) && s != "exit")
	{
		try
		{
			Parser parser(lexer(s));
			std::unique_ptr<Node> tree = parser.parse();

			double result = eval(*tree, Idents);

			if(interactive)	
				std::cout << "\x1b[A\r" << s << " = " << result << "\n\n";
			else
				std::cout << result << "\n\n";
		}
		catch (const PosError& e)
		{
			if(interactive)
				std::cout << std::string(e.pos, ' ') << std::string(e.len, '^') << "\n" 
					<< "Ошибка: " << e.what() << "\n\n";
			else
				std::cout << s << '\n' << std::string(e.pos, ' ') << 
					std::string(e.len, '^') << "\n" << "Ошибка: " << e.what() << "\n\n";
		}
	}

	return 0;
}
