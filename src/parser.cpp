#include <stdio.h>
#include <string.h>

#include "parser.h"

#define AST_NEW(type) (type*) arena->alloc(sizeof(type))
#define AST_NEW_CONSTANT(name) auto name = AST_NEW(ConstantNode); name->node_type = NODE_CONSTANT;
#define AST_NEW_INSTRUCTION(name) auto name = AST_NEW(InstructionNode); name->node_type = NODE_INSTRUCTION;
#define AST_NEW_LAMBDA(name) auto name = AST_NEW(LambdaNode); name->node_type = NODE_LAMBDA;

Parser::Parser(Arena *_arena, char *file, char *source) : arena(_arena), lexer(Lexer(_arena, file, source)) {
}

Node* Parser::parse_any() {
	auto t = lexer.get_token();
	lexer.eat_token();

	switch(t.type) {
		case TRUE: {
			AST_NEW_CONSTANT(x)
			x->type = AST_CONST_TRUE;
			return x;
		}
		case FALSE: {
			AST_NEW_CONSTANT(x)
			x->type = AST_CONST_FALSE;
			return x;
		}
		case NIL: {
			AST_NEW_CONSTANT(x)
			x->type = AST_CONST_NIL;
			return x;
		}
		case NUMBER: {
			AST_NEW_CONSTANT(x)
			x->type = AST_CONST_NUMBER;
			x->number = strtoll(t.raw, 0, 0);
			return x;
		}
		case STRING: {
			AST_NEW_CONSTANT(x)
			x->type = AST_CONST_STRING;
			x->string = t.raw;
			return x;
		}
		case REFERENCE: {
			AST_NEW_CONSTANT(x)
			x->type = AST_CONST_REFERENCE;
			x->string = t.raw;
			return x;
		}
		case ADD: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_ADD;
			return x;
		}
		case SUB: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_SUB;
			return x;
		}
		case MUL: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_MUL;
			return x;
		}
		case DIV: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_DIV;
			return x;
		}
		case NEG: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_NEG;
			return x;
		}
		case MOD: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_MOD;
			return x;
		}
		case EQUAL: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_EQ;
			return x;
		}
		case LESS: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_LT;
			return x;
		}
		case GREATER: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_GT;
			return x;
		}
		case NOT: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_NOT;
			return x;
		}
		case AND: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_NOT;
			return x;
		}
		case OR: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_OR;
			return x;
		}
		case STORE: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_STORE;
			return x;
		}
		case LOAD: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_LOAD;
			return x;
		}
		case DUP: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_DUP;
			return x;
		}
		case DROP: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_DROP;
			return x;
		}
		case SWAP: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_SWAP;
			return x;
		}
		case ROT: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_ROT;
			return x;
		}
		case NEWOBJECT: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_NEW_OBJECT;
			return x;
		}
		case GETPROP: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_GET_PROP;
			return x;
		}
		case SETPROP: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_SET_PROP;
			return x;
		}
		case LAMBDA_START:
			return parse_lambda();
		case EXECUTE: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_EXEC;
			return x;
		}
		case CONDEXEC: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_COND_EXEC;
			return x;
		}
		case WHILE: {
			AST_NEW_INSTRUCTION(x)
			x->op = AST_OP_WHILE;
			return x;
		}
		default:
			// TODO handle this
			fprintf(stderr, "Unexpected token: %s\n", t.raw);
			exit(1);
			break;
	}
}

LambdaNode* Parser::parse_lambda() {
	AST_NEW_LAMBDA(result)
	result->body = Array<Node*>();

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
	auto result = (Array<Node*>*) arena->alloc(sizeof(Array<Node*>));
	*result = Array<Node*>();

	while(lexer.has_token()) {
		result->push(parse_any());
	}

	return result;	
}