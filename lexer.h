#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <stdexcept>
#include "enums.h"

struct Token
{
	std::size_t pos, len;
	Tag tag;
	std::string text;
};

std::string tagName(Tag tag);
std::string tagChar(Tag tag);

UnOp tagToUnary(Tag tag);
BinOp tagToBinary(Tag tag);

char opSymbol(BinOp op);

bool isDigit(char c); 
bool isLetter(char c);

std::vector<Token> lexer(const std::string& str);
