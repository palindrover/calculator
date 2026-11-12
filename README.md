## Грамматика

expr &rarr; term ( ('+' | '-') term )*

term &rarr; unary ( ('\*' | '/' | '%') unary )*

unary &rarr; ('+' | '-')* power

power &rarr; primary \[ '^' unary \]

primary &rarr; Number | '(' expr ')'
