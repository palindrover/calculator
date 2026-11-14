#pragma once

#include <stdexcept>

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
