#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "array.h"
#include "ast.h"
#include "arena.h"


struct Parser {
private:
	Arena *arena;
	Lexer lexer;

	Node* parse_any();

	LambdaNode* parse_lambda();

public:
	Parser(Arena *arena, char *file, char *source);

	Array<Node*>* parse();
};

#endif