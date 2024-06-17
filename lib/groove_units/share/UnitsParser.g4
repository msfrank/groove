parser grammar UnitsParser;


options { tokenVocab = UnitsLexer; }


rsUnits         : rsSpec units ;

rsSpec          : Identifier NamespaceSeparator ;

units           : numerator ( QuotientOperator denominator )? ;

numerator       : unitSpec ( ProductOperator unitSpec )? ;
denominator     : unitSpec ( ProductOperator unitSpec )? ;

unitSpec        : Identifier+                               # listOfIdentifiers
                | Identifier* unitAndExponent               # listOfIdentifiersWithExponent
                ;

unitAndExponent : Identifier ExponentOperator DecimalLiteral ;
