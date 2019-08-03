#ifndef AST_H
#define AST_H

#include "types.h"
#include "array.h"

struct Node {
	virtual void print_as_bytecode(u32 level = 0) {}
};

struct Lambda : public Node {
	Array<Node*> body;

	void print_as_bytecode(u32 level = 0);
};

#define AST_OP_EQ 				0x00
#define AST_OP_LT 				0x01
#define AST_OP_GT 				0x02
#define AST_OP_COND_EXEC 		0x03
#define AST_OP_EXEC 			0x04
#define AST_OP_AND				0x05
#define AST_OP_OR				0x06
#define AST_OP_NOT				0x07
#define AST_OP_ADD				0x08
#define AST_OP_SUB				0x09
#define AST_OP_MUL				0x0A
#define AST_OP_DIV				0x0B
#define AST_OP_NEG				0x0C
#define AST_OP_MOD 				0x0D
#define AST_OP_NEW_OBJECT 		0x0E
#define AST_OP_GET_PROP			0x0F
#define AST_OP_SET_PROP			0x10
#define AST_OP_DUP				0x11
#define AST_OP_DROP				0x12
#define AST_OP_SWAP				0x13
#define AST_OP_ROT				0x14
#define AST_OP_LOAD				0x15
#define AST_OP_STORE			0x16
#define AST_OP_WHILE			0x17

struct Instruction : public Node {
	u8 op;

	Instruction(u8 _op) : op(_op) {}

	void print_as_bytecode(u32 level = 0);
};

#define AST_CONST_TRUE 			0
#define AST_CONST_FALSE 		1
#define AST_CONST_NIL 			2
#define AST_CONST_NUMBER 		3
#define AST_CONST_STRING		4
#define AST_CONST_REFERENCE		5

struct Constant : public Node {
	u8 type;
	union {
		s64 number;
		char *string;
	};

	Constant(u8 _type, s64 _number) : type(_type), number(_number) {} 
	Constant(u8 _type, char *_string) : type(_type), string(_string) {}

	void print_as_bytecode(u32 level = 0);
};

#endif