#ifndef LEXER_H
#define LEXER_H

#include "types.h"
#include "arena.h"

struct Span {
	u32 start;
	u32 end;

	Span() : start(0), end(0) {}

	Span(u32 _start, u32 _end) : start(_start), end(_end) {}
};

enum TokenType {
	TRUE,
    FALSE,
    NIL,
    NUMBER,
    STRING,

    ADD,
    SUB,
    MUL,
    DIV,
    NEG,
    MOD,

    EQUAL,
    LESS,
    GREATER,

    NOT,
    AND,
    OR,

    REFERENCE,
    STORE,
    LOAD,

    DUP,
    DROP,
    SWAP,
    ROT,

    NEWOBJECT,
    GETPROP,
    SETPROP,

    LAMBDA_START,
    LAMBDA_END,

    EXECUTE,
    CONDEXEC,
    WHILE
};

struct Token {
	char *file;
	Span span;
	u32 line;
	u32 column;
	char *raw;
	TokenType type;

	Token() {}
};

struct Lexer {
private:
	Allocator allocator;

	char *file;
	char *source;
	u32 length;
	u32 start;
	u32 curr;
	bool _has_token = false;
	bool rescan;
	u32 line;
	u32 column;
	Token token;

	bool more();

	char peek();

	char next();

	bool accept(const char *valid);

	void accept_run(const char *valid);

	void ignore();

	char* current();

	void emit(TokenType type);

	void skip_whitespace();

	void find_next_token();

	void scan_next();

public:
	Lexer(Allocator allocator, char *file, char *source);

	bool has_token();

	Token get_token();

	void eat_token();

	Span span();
};

#endif