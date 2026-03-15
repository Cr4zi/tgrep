#ifndef NFA_H
#define NFA_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CHARSET_LENGTH 129
#define EPSILON_TRANSITION_INDEX CHARSET_LENGTH - 1

typedef struct state *state_p;
typedef struct {
    state_p *transition[CHARSET_LENGTH]; /* list of NULL terminated lists */ 
} delta_t;

typedef struct state {
    delta_t *delta;
    bool is_accepting;
} state_t;

typedef struct {
    size_t size;
    state_t **q; /* array of states */
} nfa_t;

size_t get_transition_list_size(state_t **transitions);

/* The function returns a Non-deterministic Finite Automata that only accepts the word 'a'*/
nfa_t *nfa_create(unsigned char a);

void nfa_free(nfa_t *a);

/* Note: Those functions make a and b unusable */
nfa_t *nfa_union(nfa_t *a, nfa_t *b);
nfa_t *nfa_concat(nfa_t *a, nfa_t *b);

bool nfa_traverse(nfa_t *nfa, const char *str);

#endif
