lexer grammar UnitsLexer;


// reserved characters

NamespaceSeparator  : ':' ;
ExponentOperator    : '^' ;
ProductOperator     : '*' ;
QuotientOperator    : '/' ;


DecimalLiteral      : '0'
                    | '-' '0'
                    | '1'..'9' '0'..'9'*
                    | '-' '1'..'9' '0'..'9'*
                    ;

Identifier          :  ('a'..'z' | 'A'..'Z' | '_')+ ;


EXPRWS      : [ \t\r\n]+ -> skip ;  // skip whitespace
