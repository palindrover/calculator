#pragma once

#include <cstddef>
#include <ostream>
#include <cmath>
#include "lexer.h"
#include "node.h"
#include "errors.h"

double checked(double x, std::size_t pos, std::size_t len);

struct PrintVisitor
{
	std::ostream& out;

	void operator()(const NumberNode& node) const;
	void operator()(const IdentNode& node) const;	
	void operator()(const UnaryNode& node) const;
	void operator()(const BinaryNode& node) const;
	void operator()(const VarNode& node) const;
	void operator()(const AssignNode& node) const;
};

struct EvalVisitor
{
	const Env& env;
	Env& vars;

	double operator()(const NumberNode& node) const; 
	double operator()(const IdentNode& node) const;
	double operator()(const UnaryNode& node) const;
	double operator()(const BinaryNode& node) const;
	double operator()(const VarNode& node) const;
	double operator()(const AssignNode& node) const;
};

void print(const Node& node, std::ostream& out);

double eval(const Node& node, const Env& env, Env& vars);
