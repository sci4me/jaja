#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "lexer.h"

Lexer::Lexer(Allocator _allocator, char *_file, char *_source) : allocator(_allocator), file(_file), source(_source), start(0), curr(0) {
	length = strlen(_source);
	scan_next();
}

bool Lexer::more() {
	return curr < length;
}

char Lexer::peek() {
	return source[curr];
}

char Lexer::next() {
	auto c = peek();
	curr++;
	column++;
	return c;
}

bool Lexer::accept(const char *valid) {
	if(!more()) return false;

	auto c = peek();
	for(u32 i = 0; i < strlen(valid); i++) {
		if(valid[i] == c) {
			next();
			return true;
		}
	}

	return false;
}

void Lexer::accept_run(const char *valid) {
	while(accept(valid));
}

void Lexer::ignore() {
	start = curr;
	_has_token = false;
}

char* Lexer::current() {
	u32 len = curr - start + 1;
	auto s = (char*) ALLOC(allocator, len);
	memcpy(s, source + start, len - 1);
	s[len - 1] = 0;
	return s;
}

void Lexer::emit(TokenType type) {
	assert(!_has_token);

	auto s = current();

	token.file = file;
	token.span = span();
	token.line = line;
	token.column = column;
	token.raw = s;
	token.type = type;

	_has_token = true;
}

void Lexer::skip_whitespace() {
	while(more()) {
		switch(peek()) {
			case '\n':
				line++;
				column = 0;
			case '\t':
			case ' ':
				break;
			default:
				ignore();
				return;
		}
		next();
	}
}

void Lexer::scan_next() {
	for(;;) {
		rescan = false;
		find_next_token();

		if(!rescan) break;		
	}
}

void Lexer::find_next_token() {
	assert(!_has_token);

 	skip_whitespace();
 	if(!more()) return;

 	auto c = next();
 	switch(c) {
 		case '+':
 			emit(TokenType::ADD);
	 		break;
 		case '-':
 			emit(TokenType::SUB);
 			break;
 		case '*':
 			emit(TokenType::MUL);
 			break;
 		case '/':
 			emit(TokenType::DIV);
 			break;
 		case '%':
 			emit(TokenType::MOD);
 			break;
 	
 		case '=':
 			emit(TokenType::EQUAL);
 			break;
 		case '<':
 			emit(TokenType::LESS);
 			break;
 		case '>':
 			emit(TokenType::GREATER);
 			break;

 		case '~':
 			emit(TokenType::NOT);
 			break;
 		case '&':
 			emit(TokenType::AND);
 			break;
 		case '|':
 			emit(TokenType::OR);
 			break;

 		case ';':
 			emit(TokenType::LOAD);
 			break;
 		case ':':
 			emit(TokenType::STORE);
 			break;

 		case '$':
 			emit(TokenType::DUP);
 			break;
 		case '@':
 			emit(TokenType::DROP);
 			break;
 		case '^':
 			emit(TokenType::SWAP);
 			break;
		case '_':
			emit(TokenType::ROT);
			break;

		case '\\':
			emit(TokenType::NEWOBJECT);
			break;
		case '.':
			emit(TokenType::GETPROP);
			break;
		case ',':
			emit(TokenType::SETPROP);
			break;

		case '[':
			emit(TokenType::LAMBDA_START);
			break;
		case ']':
			emit(TokenType::LAMBDA_END);
			break;

		case '!':
			emit(TokenType::EXECUTE);
			break;
		case '?':
			emit(TokenType::CONDEXEC);
			break;
 		case '#':
 			emit(TokenType::WHILE);
 			break;

 		case '{': {
 			u32 k = 1;
 			while(k > 0 && more()) {
 				switch(peek()) {
 					case '{':
 						k++;
 						break;
 					case '}':
 						k--;
 						break;
 				}
 				next();
 			}

 			if(k != 0) {
 				// TODO handle this
 				fprintf(stderr, "Unclosed comment\n");
 				exit(1);
 			}

 			rescan = true;
 			break;
 		}

 		case '"': {
 			u32 start = curr;

 			while(more() && peek() != '"') {
 				// TODO handle escape codes

 				next();
 			}

 			u32 end = curr - 1;

 			if(!more() || next() != '"') {
 				//TODO handle this
 				fprintf(stderr, "Unclosed string\n");
 				exit(1);
 			}

 			u32 len = end - start + 2;
 			auto s = (char*) ALLOC(allocator, len);
 			memcpy(s, source + start, len);
 			s[len - 1] = 0;

 			emit(TokenType::STRING);
			// TOOD HACK remove this
			FREE(allocator, token.raw);
 			token.raw = s;
 			break;
 		}

		default:
			if(c >= '0' && c <= '9') {
				accept_run("0123456789");
				emit(TokenType::NUMBER);
			} else if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
				accept_run("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

				auto s = current();

				if(strcmp(s, "true") == 0) {
					emit(TokenType::TRUE);
				} else if(strcmp(s, "false") == 0) {
					emit(TokenType::FALSE);
				} else if(strcmp(s, "nil") == 0) {
					emit(TokenType::NIL);
				} else {
					emit(TokenType::REFERENCE);
				}

				// TOOD remove this
				// free(s);
			} else {
				// TODO handle this
				fprintf(stderr, "Unexpected character: %c\n", c);
				exit(1);
			}
			break;
 	}
}

bool Lexer::has_token() {
	return _has_token;
}

Token Lexer::get_token() {
	return token;
}

void Lexer::eat_token() {
	ignore();
	scan_next();
}

Span Lexer::span() {
	return Span(start, curr);
}