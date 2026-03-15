#include "nfa.h"

static delta_t *allocate_delta() {
    delta_t *delta = (delta_t *)malloc(sizeof(delta_t));
    if(!delta) {
        perror("Malloc failed for delta\n");
        exit(1);
    }

    for(int i = 0; i < CHARSET_LENGTH; ++i)
        delta->transition[i] = NULL;

    return delta;
}

static void free_delta(delta_t *delta) {
    if(!delta)
        return;

    for(int i = 0; i < CHARSET_LENGTH; ++i)
        free(delta->transition[i]);

    free(delta);
}

static state_t *allocate_state(delta_t *delta, bool is_accepting) {
    state_t *state = (state_t *)malloc(sizeof(state_t));
    if(!state) {
        perror("Malloc failed for state\n");
        exit(1);
    }

    state->delta = delta;
    state->is_accepting = is_accepting;

    return state;
}

static void free_state(state_t *state) {
    if(!state)
        return;

    free_delta(state->delta);
    free(state);
}

static state_t **allocate_state_list(size_t size) {
    state_t **list= (state_t **)malloc(size * sizeof(state_t *));
    if(!list) {
        perror("Failed allocating array of states\n");
        exit(1);
    }

    return list;
}

static nfa_t *allocate_nfa() {
    nfa_t *nfa = (nfa_t *)malloc(sizeof(nfa_t));
    if(!nfa) {
        perror("Failed allocating nfa struct\n");
        exit(1);
    }

    return nfa;
}

static bool nfa_traverse_helper(nfa_t *nfa, state_t *source, const char *str) {
    unsigned char ch = (unsigned char)*str;
    if(!source)
        return false;

    if(ch == '\0')
        return source->is_accepting;

    bool is_accepted = false;
    state_t **transitions_ch = source->delta->transition[ch];
    if(transitions_ch) {
        size_t len = get_transition_list_size(transitions_ch);

        for(size_t i = 0; i < len; ++i)
            is_accepted |= nfa_traverse_helper(nfa, transitions_ch[i], str + 1);
    }

    state_t **transitions_eps = source->delta->transition[EPSILON_TRANSITION_INDEX];
    if(transitions_eps) {
        size_t len = get_transition_list_size(transitions_eps);
        for(size_t i = 0; i < len; ++i)
            is_accepted |= nfa_traverse_helper(nfa, transitions_eps[i], str);
    }

    return is_accepted;
}

static void move_states_to_nfa(nfa_t *target, nfa_t *from, size_t target_start_index) {
    for(size_t i = 0; i < from->size; ++i)
        target->q[i + target_start_index] = from->q[i];
}

size_t get_transition_list_size(state_t **transitions) {
    size_t len = 0;

    /* might be unnecessary */
    if(!transitions)
        return len;

    while(transitions[len++] != NULL);

    return len - 1;
}

nfa_t *nfa_create(unsigned char a) {
    if(a < 0 || a >= CHARSET_LENGTH)
        return NULL;
    
    delta_t *delta_q0 = allocate_delta();
    state_t *q0 = allocate_state(delta_q0, false);

    delta_t *delta_q1 = allocate_delta();
    state_t *q1 = allocate_state(delta_q1, true);

    state_p *transition = delta_q0->transition[a];
    transition = allocate_state_list(2);
    transition[0] = q1;
    transition[1] = NULL;

    delta_q0->transition[a] = transition;

    nfa_t *nfa = allocate_nfa();
    
    nfa->size = 2;
    nfa->q = allocate_state_list(nfa->size);
    nfa->q[0] = q0;
    nfa->q[1] = q1;

    return nfa;
}

void nfa_free(nfa_t *a) {
    if(!a)
        return;

    for(size_t i = 0; i < a->size; ++i)
        free_state(a->q[i]);

    free(a->q);
    free(a);
}

nfa_t *nfa_union(nfa_t *a, nfa_t *b) {
    delta_t *delta_q0 = allocate_delta();
    state_t *q0 = allocate_state(delta_q0, false);

    state_t **epsilon_transition = delta_q0->transition[EPSILON_TRANSITION_INDEX];
    epsilon_transition = allocate_state_list(3);

    epsilon_transition[0] = a->q[0];
    epsilon_transition[1] = b->q[0];
    epsilon_transition[2] = NULL;

    delta_q0->transition[EPSILON_TRANSITION_INDEX] = epsilon_transition;

    nfa_t *nfa = allocate_nfa();
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

nfa_t *nfa_concat(nfa_t *a, nfa_t *b) {
    nfa_t *nfa = allocate_nfa();
    nfa->size = a->size + b->size;

    nfa->q = allocate_state_list(nfa->size);
    move_states_to_nfa(nfa, a, 0);
    move_states_to_nfa(nfa, b, a->size);

    for(size_t i = 0; i < a->size; ++i) {
        if(!a->q[i]->is_accepting)
            continue;

        a->q[i]->is_accepting = false;

        state_t **epsilon_transition = a->q[i]->delta->transition[EPSILON_TRANSITION_INDEX];
        size_t len = get_transition_list_size(epsilon_transition);
        epsilon_transition = (state_t **)realloc((void *)epsilon_transition, (len + 2) * sizeof(state_t *));
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

bool nfa_traverse(nfa_t *nfa, const char *str) {
    if(!nfa)
        return false;

    return nfa_traverse_helper(nfa, nfa->q[0], str);
}
