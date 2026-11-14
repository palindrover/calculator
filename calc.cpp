#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <unistd.h>
#include "enums.h"
#include "lexer.h"
#include "errors.h"
#include "node.h"
#include "visitor.h"
#include "parser.h"

/*

Грамматика

statement -> Ident '=' expr
expr -> term ( ('+' | '-') term )*
term -> unary ( ('*' | '/' | '%') unary )*
unary -> ('+' | '-')* power
power -> primary [ '^' unary ]
primary -> Number | Ident | '(' expr ')'

*/

bool interactive = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);

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

			double result = eval(*tree, Idents, Vars);

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
