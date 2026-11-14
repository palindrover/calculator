#pragma once

#include <unordered_map>
#include <string>

using Env = std::unordered_map<std::string, double>;


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

inline Env Vars {};
