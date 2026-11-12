## Грамматика

expr &rarr; term ( ('+' | '-') term )*

term &rarr; unary ( ('\*' | '/' | '%') unary )*

unary &rarr; ('+' | '-') unary | power

power &rarr; primary \[ '^' unary \]

primary &rarr; Number | Ident | '(' expr ')'
