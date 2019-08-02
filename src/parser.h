#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "array.h"
#include "ast.h"

struct Parser {
private:
	Lexer lexer;

	bool more();

	void next();

	Token current();

public:
	Parser(char *file, char *source);
	~Parser();

	Array<Node*> parse();
};

#endif