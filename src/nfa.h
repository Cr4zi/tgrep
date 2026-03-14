#ifndef NFA_H
#define NFA_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CHARSET_LENGTH 129
#define EPSILON_TRANSITION_INDEX CHARSET_LENGTH - 1

typedef struct state_s *state_p;
typedef struct delta_s {
    state_p *transition[CHARSET_LENGTH]; /* list of NULL terminated lists */ 
} Delta;

typedef struct state_s {
    Delta *delta;
    bool is_accepting;
} State;

typedef struct nfa_s {
    size_t size;
    State **q; /* array of states */
} Nfa;

size_t get_transition_list_size(State **transitions);

/* The function returns a Non-deterministic Finite Automata that only accepts the word 'a'*/
Nfa *nfa_create(unsigned char a);

void nfa_free(Nfa *a);

/* Note: Those functions make a and b unusable */
Nfa *nfa_union(Nfa *a, Nfa *b);
Nfa *nfa_concat(Nfa *a, Nfa *b);

bool nfa_traverse(Nfa *nfa, const char *str);

#endif
