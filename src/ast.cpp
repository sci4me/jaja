#include <stdio.h>

#include "ast.h"

static void indent(u32 level) {
	for(u32 i = 0; i < level; i++) {
		printf("    ");
	}
}

static void print_instruction(Node *node, u32 op, u32 level) {
	indent(level);

	switch(op) {
		case AST_OP_EQ:
			printf("eq\n");
			break; 				
		case AST_OP_LT:
			printf("lt\n");
			break; 				
		case AST_OP_GT:
			printf("gt\n");
			break; 				
		case AST_OP_COND_EXEC:
			printf("cond_exec\n");
			break; 		
		case AST_OP_EXEC:
			printf("exec\n");	
			break; 			
		case AST_OP_AND:
			printf("and\n");
			break;				
		case AST_OP_OR:
			printf("or\n");
			break;				
		case AST_OP_NOT:
			printf("not\n");
			break;				
		case AST_OP_ADD:
			printf("add\n");
			break;				
		case AST_OP_SUB:
			printf("sub\n");
			break;				
		case AST_OP_MUL:
			printf("mul\n");
			break;				
		case AST_OP_DIV:
			printf("div\n");
			break;				
		case AST_OP_NEG:
			printf("neg\n");
			break;				
		case AST_OP_MOD:
			printf("mod\n");
			break; 				
		case AST_OP_NEW_OBJECT:
			printf("new_object\n");
			break; 		
		case AST_OP_GET_PROP:
			printf("get_prop\n");
			break;			
		case AST_OP_SET_PROP:
			printf("set_prop\n");
			break;			
		case AST_OP_DUP:
			printf("dup\n");
			break;				
		case AST_OP_DROP:
			printf("drop\n");
			break;				
		case AST_OP_SWAP:
			printf("swap\n");
			break;				
		case AST_OP_ROT:
			printf("rot\n");
			break;				
		case AST_OP_LOAD:
			printf("load\n");
			break;				
		case AST_OP_STORE:
			printf("store\n");
			break;			
		case AST_OP_WHILE:
			printf("while\n");
			break;
		default:
            assert(false);
			break;	
	}
}

void Node::print_as_bytecode(u32 level) {
	indent(level);
    switch(type) {
        case NODE_TRUE:
			printf("true\n");
            break;
        case NODE_FALSE:
            printf("false\n");
            break;
        case NODE_NIL:
            printf("nil\n");
            break;
        case NODE_NUMBER:
            printf("%lli\n", number);
            break;
        case NODE_STRING:
            printf("\"%s\"\n", string);
            break;
        case NODE_REFERENCE:
            printf("%s\n", string);
            break;
        case NODE_INSTRUCTION:
            print_instruction(this, op, level);
            break;
        case NODE_LAMBDA:
            printf("lambda {\n");
            FOR((&lambda), i) {
	        	lambda.data[i]->print_as_bytecode(level + 1);
	        }
        	printf("}\n");
            break;
        default:
            assert(false);
            break;
    }
}