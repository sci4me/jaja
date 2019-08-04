#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "array.h"
#include "ast.h"

struct Parser {
private:
	Lexer lexer;

	Node* parse_any();

	LambdaNode* parse_lambda();

public:
	Parser(char *file, char *source);

	Array<Node*>* parse();
};

#endif