#include "parser.h"

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

std::unique_ptr<Node> Parser::parse()
{
	std::unique_ptr<Node> value = parseExpr();
	if(current().tag != Tag::End)
		throw ParseError("лишние символы после выражения", current().pos,
				current().len);
	return value;
}

const Token& Parser::current()
{
	return tokens[pos];
}

void Parser::advance()
{
	if(current().tag != Tag::End) pos++;
}

void Parser::expect(Tag tag)
{
	if(current().tag != tag)
		throw ParseError(std::string("ожидалось ") + tagChar(tag), current().pos,
				current().len);
	else advance();
}

std::unique_ptr<Node> Parser::parsePower()
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

std::unique_ptr<Node> Parser::parseUnary()
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

std::unique_ptr<Node> Parser::parseTerm()
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

std::unique_ptr<Node> Parser::parseExpr()
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

std::unique_ptr<Node> Parser::parsePrimary()
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
