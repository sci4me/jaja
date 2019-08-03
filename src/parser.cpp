#include <stdio.h>
#include <string.h>

#include "parser.h"

Parser::Parser(char *file, char *source) : lexer(Lexer(file, source)) {
}

Node* Parser::parse_any() {
	auto t = lexer.get_token();
	lexer.eat_token();

	Node *result;

	switch(t.type) {
		case TRUE:
			result = new Constant(CONST_TRUE, 1);
			break;
		case FALSE:
			result = new Constant(CONST_FALSE, 1);
			break;
		case NIL:
			result = new Constant(CONST_NIL, 1);
			break;
		case NUMBER:
			result = new Constant(CONST_NUMBER, strtoll(t.raw, 0, 0));
			break;
		case STRING:
			result = new Constant(CONST_STRING, t.raw);
			break;
		case REFERENCE:
			result = new Constant(CONST_REFERENCE, t.raw);
			break;
		case ADD:
			result = new Instruction(OP_ADD);
			break;
		case SUB:
			result = new Instruction(OP_SUB);
			break;
		case MUL:
			result = new Instruction(OP_MUL);
			break;
		case DIV:
			result = new Instruction(OP_DIV);
			break;
		case NEG:
			result = new Instruction(OP_NEG);
			break;
		case MOD:
			result = new Instruction(OP_MOD);
			break;
		case EQUAL:
			result = new Instruction(OP_EQ);
			break;
		case LESS:
			result = new Instruction(OP_LT);
			break;
		case GREATER:
			result = new Instruction(OP_GT);
			break;
		case NOT:
			result = new Instruction(OP_NOT);
			break;
		case AND:
			result = new Instruction(OP_AND);
			break;
		case OR:
			result = new Instruction(OP_OR);
			break;
		case STORE:
			result = new Instruction(OP_STORE);
			break;
		case LOAD:
			result = new Instruction(OP_LOAD);
			break;
		case DUP:
			result = new Instruction(OP_DUP);
			break;
		case DROP:
			result = new Instruction(OP_DROP);
			break;
		case SWAP:
			result = new Instruction(OP_SWAP);
			break;
		case ROT:
			result = new Instruction(OP_ROT);
			break;
		case NEWOBJECT:
			result = new Instruction(OP_NEW_OBJECT);
			break;
		case GETPROP:
			result = new Instruction(OP_GET_PROP);
			break;
		case SETPROP:
			result = new Instruction(OP_SET_PROP);
			break;
		case LAMBDA_START:
			result = parse_lambda();
			break;
		case EXECUTE:
			result = new Instruction(OP_EXEC);
			break;
		case CONDEXEC:
			result = new Instruction(OP_COND_EXEC);
			break;
		case WHILE:
			result = new Instruction(OP_WHILE);
			break;
		default:
			// TODO handle this
			fprintf(stderr, "Unexpected token: %s\n", t.raw);
			exit(1);
			break;
	}

	return result;
}

Lambda* Parser::parse_lambda() {
	auto result = new Lambda();

	while(lexer.has_token()) {
		if(lexer.get_token().type == LAMBDA_END) {
			break;
		} else {
			result->body.add(parse_any());
		}
	}

	if(lexer.get_token().type == LAMBDA_END) {
		lexer.eat_token();
	} else {
		// TODO handle this		
		fprintf(stderr, "Unclosed lambda\n");
		exit(1);
	}

	return result;
}

Array<Node*>* Parser::parse() {
	auto result = new Array<Node*>();

	while(lexer.has_token()) {
		result->add(parse_any());
	}

	return result;	
}