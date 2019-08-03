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

#define OP_EQ 				0x00
#define OP_LT 				0x01
#define OP_GT 				0x02
#define OP_COND_EXEC 		0x03
#define OP_EXEC 			0x04
#define OP_AND				0x05
#define OP_OR				0x06
#define OP_NOT				0x07
#define OP_ADD				0x08
#define OP_SUB				0x09
#define OP_MUL				0x0A
#define OP_DIV				0x0B
#define OP_NEG				0x0C
#define OP_MOD 				0x0D
#define OP_NEW_OBJECT 		0x0E
#define OP_GET_PROP			0x0F
#define OP_SET_PROP			0x10
#define OP_DUP				0x11
#define OP_DROP				0x12
#define OP_SWAP				0x13
#define OP_ROT				0x14
#define OP_LOAD				0x15
#define OP_STORE			0x16
#define OP_WHILE			0x17

struct Instruction : public Node {
	u8 op;

	Instruction(u8 _op) : op(_op) {}

	void print_as_bytecode(u32 level = 0);
};

#define CONST_TRUE 			0
#define CONST_FALSE 		1
#define CONST_NIL 			2
#define CONST_NUMBER 		3
#define CONST_STRING		4
#define CONST_REFERENCE		5

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