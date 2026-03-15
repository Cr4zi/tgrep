#include "lexer.h"

static token_t *new_token() {
    token_t *token = (token_t *)malloc(sizeof(token_t));
    if(!token) {
        perror("Failed allocating memory for token\n");
        exit(1);
    }

    token->next = NULL;
    token->prev = NULL;

    token->power = -1;

    return token;
}

static void lexer_append(lexer_t *lexer, token_t *token) {
    if(!lexer->end) {
        lexer->end = token;
        lexer->start = token;
        return;
    }

    token->prev = lexer->end;
    token->next = NULL;
    lexer->end->next = token;
    lexer->end = token;
}

static void consume(const char **str) {
    (*str)++;
}

static char peek(const char **str) {
    return (*str)[1];
}

static ssize_t parse_num(const char **str) {
    ssize_t res = 0;
    if(**str != '(') {
        perror("Expected (\n");
        exit(1);
    }
    consume(str);
    
    while(**str >= '0' && **str <= '9') {
        res *= 10;
        res += (**str - '0');

        consume(str);
    }


    return res;
}

lexer_t *lexer_new(const char **str) {
    lexer_t *lexer = (lexer_t *)malloc(sizeof(lexer_t));
    token_t *token = NULL;
    
    if(!lexer) {
        perror("Failed allocating memory for lexer\n");
        exit(1);
    }
    
    while(**str != '\0') {
        token = new_token();
        switch(**str) {
            case '(': case ')': case '*': case '+':
                token->type = Op;
                token->ch = **str;
                break;
            case '^':
                token->type = Op;
                token->ch = **str;
                consume(str); // consume ^
                if(**str >= '0' && **str <= '9')
                    token->power = **str - '0';
                else // ch = '('
                    token->power = parse_num(str);
                break;
            case '\\':
                token->type = Atomic;
                char fut_ch = peek(str);
                switch(fut_ch) {
                    case '(': case ')': case '*': case '+': case '^':
                        token->ch = fut_ch;
                        consume(str);
                        break;
                    default:
                        token->ch = **str;
                }
                break;
            default:
                token->type = Atomic;
                token->ch = **str;
        }

        lexer_append(lexer, token);
        consume(str);
    }

    token = new_token();
    token->type = Eof;
    lexer_append(lexer, token);

    return lexer;
}

void lexer_free(lexer_t *lexer) {
    if(!lexer || !lexer->start)
        return;

    token_t *curr = lexer->start;
    while(curr) {
        token_t *next = curr->next;

        free(curr);
        curr = next;
    }

    free(lexer);
}

void print_lexer(lexer_t *lexer) {
    if(!lexer) {
        printf("NULL");
        return;
    }

    for(token_t *token = lexer->start; token; token = token->next){
        switch(token->type) {
            case Atomic:
                printf("Atomic: %c -> ", token->ch);
                break;
            case Op:
                printf("Op: %c", token->ch);
                if(token->ch == '^')
                    printf(" %zd", token->power);
                printf(" -> ");
                break;
            case Eof:
                printf("EOF\n");
        }
    }
}
