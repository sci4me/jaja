#include "parser.h"

Parser::Parser(char *file, char *source) : lexer(Lexer(file, source)) {
}

Parser::~Parser() {
}

bool Parser::more() {
	return lexer.has_token();
}

void Parser::next() {
	lexer.eat_token();
}

Token Parser::current() {
	return lexer.get_token();
}

Array<Node*> Parser::parse() {
	auto result = Array<Node*>();

	

	return result;	
}