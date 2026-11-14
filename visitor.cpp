#include "visitor.h"
#include "errors.h"

double checked(double x, std::size_t pos, std::size_t len)
{
	if(std::isnan(x) || std::isinf(x)) throw EvalError("переполнение", pos, len);
	return x;
}

void PrintVisitor::operator()(const NumberNode& node) const {out << node.text;}
void PrintVisitor::operator()(const IdentNode& node) const {out << node.text;}
void PrintVisitor::operator()(const UnaryNode& node) const
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
void PrintVisitor::operator()(const BinaryNode& node) const
{
	out << "(" << opSymbol(node.op) << " ";
	std::visit(*this, node.left->v);
	out << " ";
	std::visit(*this, node.right->v);
	out << ")";
}
void PrintVisitor::operator()(const AssignNode& node) const 
{
	out << node.name << " = " << node.value;
}


double EvalVisitor::operator()(const NumberNode& node) const 
{
	return checked(std::strtod(node.text.c_str(), nullptr), node.pos, node.len);
}
double EvalVisitor::operator()(const IdentNode& node)const
{
	if(auto it = env.find(node.text); it != env.end()) return it->second;
	if(auto it = vars.find(node.text); it != vars.end()) return it->second;
	throw EvalError("неизвестный индентификатор " + node.text, node.pos, node.len);
}
double EvalVisitor::operator()(const UnaryNode& node) const
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
double EvalVisitor::operator()(const BinaryNode& node) const
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

double EvalVisitor::operator()(const AssignNode& node) const
{
	if(env.contains(node.name)) 
		throw EvalError("нельзя изменить константу " + node.name, node.pos, node.len);
	double val = std::visit(*this, node.value->v);
	vars[node.name] = val;
	return val;
}


void print(const Node& node, std::ostream& out)
{
	std::visit(PrintVisitor{out}, node.v);
}

double eval(const Node& node, const Env& env, Env& vars)
{
	return std::visit(EvalVisitor{env, vars}, node.v);
}
