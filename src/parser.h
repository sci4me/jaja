#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "array.h"
#include "ast.h"
#include "allocator.h"

struct Parser {
private:
	Allocator allocator;
	Lexer lexer;

	Node* parse_any();
	Node* parse_lambda();

public:
	Parser(Allocator allocator, char *file, char *source);

	Array<Node*>* parse();
};

#endif