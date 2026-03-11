#include "nfa.h"

static Delta *allocate_delta() {
    Delta *delta = (Delta *)malloc(sizeof(Delta));
    if(!delta) {
        perror("Malloc failed for delta\n");
        exit(1);
    }

    for(int i = 0; i < CHARSET_LENGTH; ++i)
        delta->transition[i] = NULL;

    return delta;
}

static void free_delta(Delta *delta) {
    if(!delta)
        return;

    for(int i = 0; i < CHARSET_LENGTH; ++i)
        free(delta->transition[i]);

    free(delta);
}

static State *allocate_state(Delta *delta, bool is_accepting) {
    State *state = (State *)malloc(sizeof(State));
    if(!state) {
        perror("Malloc failed for state\n");
        exit(1);
    }

    state->delta = delta;
    state->is_accepting = is_accepting;

    return state;
}

static void free_state(State *state) {
    if(!state)
        return;

    free_delta(state->delta);
    free(state);
}

static State **allocate_state_list(size_t size) {
    State **list= (State **)malloc(size * sizeof(State *));
    if(!list) {
        perror("Failed allocating array of states\n");
        exit(1);
    }

    return list;
}

static Nfa *allocate_nfa() {
    Nfa *nfa = (Nfa *)malloc(sizeof(Nfa));
    if(!nfa) {
        perror("Failed allocating nfa struct\n");
        exit(1);
    }

    return nfa;
}

static void state_self_loop(State *q) {
    Delta *delta = q->delta;
    for(int i = 0 ; i < CHARSET_LENGTH; ++i) {
        delta->transition[i] = (state_p *)malloc(2 * sizeof(state_p));
        if(!delta->transition[i]) {
            perror("Failed allocating array for transition\n");
            exit(1);
        }

        delta->transition[i][0] = q;
        delta->transition[i][1] = NULL;
    }
}

static void move_states_to_nfa(Nfa *target, Nfa *from, size_t target_start_index) {
    for(size_t i = 0; i < from->size; ++i)
        target->q[i + target_start_index] = from->q[i];
}

static size_t get_transition_list_size(state_p *transitions) {
    size_t len = 0;

    /* might be unnecessary */
    if(!transitions)
        return len;

    for(state_p transition = transitions[len]; transition != NULL;)
        transition = transitions[++len];

    return len;
}

Nfa *nfa_create(unsigned char a) {
    if(a < 0 || a >= CHARSET_LENGTH)
        return NULL;
    
    Delta *delta_q0 = allocate_delta();
    State *q0 = allocate_state(delta_q0, false);

    state_self_loop(q0);

    Delta *delta_q1 = allocate_delta();
    State *q1 = allocate_state(delta_q1, true);

    delta_q0->transition[a][0] = q1;

    Nfa *nfa = allocate_nfa();
    
    nfa->size = 2;
    nfa->q = allocate_state_list(nfa->size);
    nfa->q[0] = q0;
    nfa->q[1] = q1;

    return nfa;
}

void nfa_free(Nfa *a) {
    if(!a)
        return;

    for(size_t i = 0; i < a->size; ++i)
        free_state(a->q[i]);

    free(a);
}

Nfa *nfa_union(Nfa *a, Nfa *b) {
    Delta *delta_q0 = allocate_delta();
    State *q0 = allocate_state(delta_q0, false);

    state_self_loop(q0);
    state_p *epsilon_transition = delta_q0->transition[EPSILON_TRANSITION_INDEX];
    epsilon_transition = (state_p *)realloc(epsilon_transition, 3);
    if(!epsilon_transition) {
        perror("Failed reallocating for epsilon transition\n");
        exit(1);
    }

    epsilon_transition[0] = a->q[0];
    epsilon_transition[1] = b->q[1];
    epsilon_transition[2] = NULL;

    delta_q0->transition[EPSILON_TRANSITION_INDEX] = epsilon_transition;

    Nfa *nfa = allocate_nfa();
    nfa->size = 1 + a->size + b->size;
    nfa->q = allocate_state_list(nfa->size);

    size_t indx = 0;
    nfa->q[indx++] = q0;
    move_states_to_nfa(nfa, a, indx);

    indx += a->size;
    move_states_to_nfa(nfa, b, indx);
    
    return nfa;
}

Nfa *nfa_concat(Nfa *a, Nfa *b) {
    for(size_t i = 0; i < a->size; ++i) {
        if(!a->q[i]->is_accepting)
            continue;
        
        a->q[i]->is_accepting = false;

        size_t len = get_transition_list_size(a->q[i]->delta->transition[EPSILON_TRANSITION_INDEX]);
        state_p *epsilon_transition = a->q[i]->delta->transition[EPSILON_TRANSITION_INDEX];
        epsilon_transition = (state_p *)realloc(epsilon_transition, len + 1);
        if(!epsilon_transition) {
            perror("Failed reallocating for epsilon transition\n");
            exit(1);
        }

        /* The element at len - 1 is NULL thus, we change it to q0 of b */
        epsilon_transition[len - 1] = b->q[0];
        epsilon_transition[len] = NULL;
        
        a->q[i]->delta->transition[EPSILON_TRANSITION_INDEX] = epsilon_transition;
    }

    State **q = (State **)realloc(a->q, a->size + b->size);
    if(!q) {
        perror("Failed reallocating for extending a state list\n");
        exit(1);
    }
    
    move_states_to_nfa(a, b, a->size + 1);

    return a;
}
