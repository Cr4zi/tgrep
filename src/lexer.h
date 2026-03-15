#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct token_s *token_p;
typedef struct token_s {
    enum {
        Atomic, Op, Eof 
    } type;

    unsigned char ch;
    ssize_t power;

    token_p prev, next;
} token_t;

typedef struct {
    token_t *start;
    token_t *end;
} lexer_t;

lexer_t *lexer_new(const char **str);
void lexer_free(lexer_t *lexer);
void print_lexer(lexer_t *lexer);

#endif
