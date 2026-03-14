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

static bool nfa_traverse_helper(Nfa *nfa, State *source, const char *str) {
    unsigned char ch = (unsigned char)*str;
    if(!source)
        return false;

    if(ch == '\0')
        return source->is_accepting;

    bool is_accepted = false;
    State **transitions_ch = source->delta->transition[ch];
    if(transitions_ch) {
        size_t len = get_transition_list_size(transitions_ch);

        for(size_t i = 0; i < len; ++i)
            is_accepted |= nfa_traverse_helper(nfa, transitions_ch[i], str + 1);
    }

    State **transitions_eps = source->delta->transition[EPSILON_TRANSITION_INDEX];
    if(transitions_eps) {
        size_t len = get_transition_list_size(transitions_eps);
        for(size_t i = 0; i < len; ++i)
            is_accepted |= nfa_traverse_helper(nfa, transitions_eps[i], str);
    }

    return is_accepted;
}

static void move_states_to_nfa(Nfa *target, Nfa *from, size_t target_start_index) {
    for(size_t i = 0; i < from->size; ++i)
        target->q[i + target_start_index] = from->q[i];
}

size_t get_transition_list_size(State **transitions) {
    size_t len = 0;

    /* might be unnecessary */
    if(!transitions)
        return len;

    while(transitions[len++] != NULL);

    return len - 1;
}

Nfa *nfa_create(unsigned char a) {
    if(a < 0 || a >= CHARSET_LENGTH)
        return NULL;
    
    Delta *delta_q0 = allocate_delta();
    State *q0 = allocate_state(delta_q0, false);

    Delta *delta_q1 = allocate_delta();
    State *q1 = allocate_state(delta_q1, true);

    state_p *transition = delta_q0->transition[a];
    transition = allocate_state_list(2);
    transition[0] = q1;
    transition[1] = NULL;

    delta_q0->transition[a] = transition;

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

    free(a->q);
    free(a);
}

Nfa *nfa_union(Nfa *a, Nfa *b) {
    Delta *delta_q0 = allocate_delta();
    State *q0 = allocate_state(delta_q0, false);

    State **epsilon_transition = delta_q0->transition[EPSILON_TRANSITION_INDEX];
    epsilon_transition = allocate_state_list(3);

    epsilon_transition[0] = a->q[0];
    epsilon_transition[1] = b->q[0];
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

    free(a->q);
    free(a);

    free(b->q);
    free(b);
    
    return nfa;
}

Nfa *nfa_concat(Nfa *a, Nfa *b) {
    Nfa *nfa = allocate_nfa();
    nfa->size = a->size + b->size;

    nfa->q = allocate_state_list(nfa->size);
    move_states_to_nfa(nfa, a, 0);
    move_states_to_nfa(nfa, b, a->size);

    for(size_t i = 0; i < a->size; ++i) {
        if(!a->q[i]->is_accepting)
            continue;

        a->q[i]->is_accepting = false;

        State **epsilon_transition = a->q[i]->delta->transition[EPSILON_TRANSITION_INDEX];
        size_t len = get_transition_list_size(epsilon_transition);
        epsilon_transition = (State **)realloc((void *)epsilon_transition, (len + 2) * sizeof(State *));
        if(!epsilon_transition) {
            perror("Failed reallocating for epsilon transition in concat\n");
            exit(1);
        }

        epsilon_transition[len] = b->q[0];
        epsilon_transition[len + 1] = NULL;

        a->q[i]->delta->transition[EPSILON_TRANSITION_INDEX] = epsilon_transition;
    }
    
    free(a->q);
    free(a);

    free(b->q);
    free(b);
    
    return nfa;
}

bool nfa_traverse(Nfa *nfa, const char *str) {
    if(!nfa)
        return false;

    return nfa_traverse_helper(nfa, nfa->q[0], str);
}
