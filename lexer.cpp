#include "lexer.h"

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

UnOp tagToUnary(Tag tag)
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

BinOp tagToBinary(Tag tag)
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

char opSymbol(BinOp op)
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


bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool isLetter(char c)
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
