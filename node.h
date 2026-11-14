#pragma once

#include <string>
#include <variant>
#include <memory>
#include "enums.h"

struct Node;

struct AssignNode
{
	std::string name;
	std::unique_ptr<Node> value;
	std::size_t pos, len;
};

struct VarNode
{
	std::string name;
	std::size_t pos, len;
};

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
	std::variant<NumberNode, IdentNode, UnaryNode, BinaryNode, VarNode, AssignNode> v;
};
