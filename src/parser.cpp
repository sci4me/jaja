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
			result = new ConstantNode(AST_CONST_TRUE, 1);
			break;
		case FALSE:
			result = new ConstantNode(AST_CONST_FALSE, 1);
			break;
		case NIL:
			result = new ConstantNode(AST_CONST_NIL, 1);
			break;
		case NUMBER:
			result = new ConstantNode(AST_CONST_NUMBER, strtoll(t.raw, 0, 0));
			break;
		case STRING:
			result = new ConstantNode(AST_CONST_STRING, t.raw);
			break;
		case REFERENCE:
			result = new ConstantNode(AST_CONST_REFERENCE, t.raw);
			break;
		case ADD:
			result = new InstructionNode(AST_OP_ADD);
			break;
		case SUB:
			result = new InstructionNode(AST_OP_SUB);
			break;
		case MUL:
			result = new InstructionNode(AST_OP_MUL);
			break;
		case DIV:
			result = new InstructionNode(AST_OP_DIV);
			break;
		case NEG:
			result = new InstructionNode(AST_OP_NEG);
			break;
		case MOD:
			result = new InstructionNode(AST_OP_MOD);
			break;
		case EQUAL:
			result = new InstructionNode(AST_OP_EQ);
			break;
		case LESS:
			result = new InstructionNode(AST_OP_LT);
			break;
		case GREATER:
			result = new InstructionNode(AST_OP_GT);
			break;
		case NOT:
			result = new InstructionNode(AST_OP_NOT);
			break;
		case AND:
			result = new InstructionNode(AST_OP_AND);
			break;
		case OR:
			result = new InstructionNode(AST_OP_OR);
			break;
		case STORE:
			result = new InstructionNode(AST_OP_STORE);
			break;
		case LOAD:
			result = new InstructionNode(AST_OP_LOAD);
			break;
		case DUP:
			result = new InstructionNode(AST_OP_DUP);
			break;
		case DROP:
			result = new InstructionNode(AST_OP_DROP);
			break;
		case SWAP:
			result = new InstructionNode(AST_OP_SWAP);
			break;
		case ROT:
			result = new InstructionNode(AST_OP_ROT);
			break;
		case NEWOBJECT:
			result = new InstructionNode(AST_OP_NEW_OBJECT);
			break;
		case GETPROP:
			result = new InstructionNode(AST_OP_GET_PROP);
			break;
		case SETPROP:
			result = new InstructionNode(AST_OP_SET_PROP);
			break;
		case LAMBDA_START:
			result = parse_lambda();
			break;
		case EXECUTE:
			result = new InstructionNode(AST_OP_EXEC);
			break;
		case CONDEXEC:
			result = new InstructionNode(AST_OP_COND_EXEC);
			break;
		case WHILE:
			result = new InstructionNode(AST_OP_WHILE);
			break;
		default:
			// TODO handle this
			fprintf(stderr, "Unexpected token: %s\n", t.raw);
			exit(1);
			break;
	}

	return result;
}

LambdaNode* Parser::parse_lambda() {
	auto result = new LambdaNode();

	while(lexer.has_token()) {
		if(lexer.get_token().type == LAMBDA_END) {
			break;
		} else {
			result->body.push(parse_any());
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
		result->push(parse_any());
	}

	return result;	
}