#pragma once

#include <memory>
#include <vector>
#include "errors.h"
#include "lexer.h"
#include "node.h"

class Parser
{
public:
	explicit Parser(std::vector<Token> tokens);
	std::unique_ptr<Node> parse();

private:
	std::vector<Token> tokens;
	std::size_t pos = 0;

	const Token& current();
	void advance();
	void expect(Tag tag);

	std::unique_ptr<Node> parseState();
	std::unique_ptr<Node> parseVar();
	std::unique_ptr<Node> parsePower();
	std::unique_ptr<Node> parseUnary();
	std::unique_ptr<Node> parseTerm();
	std::unique_ptr<Node> parseExpr();
	std::unique_ptr<Node> parsePrimary();
};
