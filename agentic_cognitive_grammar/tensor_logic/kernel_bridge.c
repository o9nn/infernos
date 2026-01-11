/*
 * OpenCog Kernel Bridge for Tensor Logic
 *
 * Provides integration between the Tensor Logic neural-symbolic
 * reasoning engine and the Inferno OS OpenCog kernel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tensor_logic.h"

/*
 * Kernel data structure mappings
 * These match the structures in os/port/portdat.h
 */

typedef struct KernelTruthValue KernelTruthValue;
typedef struct KernelAtom KernelAtom;
typedef struct KernelAtomSpace KernelAtomSpace;
typedef struct KernelGoal KernelGoal;
typedef struct KernelCognitiveState KernelCognitiveState;
typedef struct KernelOpenCog KernelOpenCog;

struct KernelTruthValue {
    float strength;
    float confidence;
    float count;
};

struct KernelAtom {
    unsigned long id;
    int type;
    char *name;
    KernelTruthValue tv;
    KernelAtom **outgoing;
    int arity;
    KernelAtom *next;
};

struct KernelAtomSpace {
    KernelAtom **atoms;
    int natoms;
    int maxatoms;
    unsigned long next_id;
    KernelAtomSpace *parent;
};

struct KernelGoal {
    unsigned long id;
    char *description;
    float urgency;
    float importance;
    KernelTruthValue satisfaction;
    KernelGoal *subgoals;
    KernelGoal *next;
};

struct KernelCognitiveState {
    KernelAtomSpace *local_space;
    KernelGoal *active_goals;
    void *reasoner;
    float attention_level;
    float motivation;
    void *context;
    long long think_time;
    int cognitive_load;
};

struct KernelOpenCog {
    KernelAtomSpace *global_space;
    void *global_reasoner;
    KernelGoal *system_goals;
    void *pm;
    int cognitive_processes;
    long long total_atoms;
    long long reasoning_cycles;
    float system_attention;
    int distributed_nodes;
};

/* Global tensor logic engine */
static TensorLogicEngine *g_tensor_engine = NULL;
static TensorAtomSpace *g_tensor_atomspace = NULL;

/*
 * Convert kernel truth value to tensor truth value
 */
TensorTruthValue* convert_kernel_tv_to_tensor(KernelTruthValue *ktv) {
    if (!ktv) return NULL;
    return tensor_tv_create(ktv->strength, ktv->confidence);
}

/*
 * Convert tensor truth value to kernel truth value
 */
void convert_tensor_tv_to_kernel(TensorTruthValue *ttv, KernelTruthValue *ktv) {
    if (!ttv || !ktv) return;
    ktv->strength = ttv->strength;
    ktv->confidence = ttv->confidence;
    ktv->count = ttv->evidence;
}

/*
 * Sync kernel atom to tensor atom
 */
TensorAtom* sync_kernel_atom_to_tensor(KernelAtom *katom) {
    if (!katom || !g_tensor_atomspace) return NULL;

    /* Check if atom already exists in tensor space */
    TensorAtom *tatom = tensor_atom_find(g_tensor_atomspace, katom->name);
    if (tatom) {
        /* Update existing atom's truth value */
        TensorTruthValue *new_tv = convert_kernel_tv_to_tensor(&katom->tv);
        if (new_tv) {
            TensorTruthValue *merged = tensor_tv_merge(&tatom->tv, new_tv);
            if (merged) {
                tatom->tv = *merged;
                free(merged);
            }
            tensor_tv_destroy(new_tv);
        }
        return tatom;
    }

    /* Create new tensor atom */
    TensorTruthValue *ttv = convert_kernel_tv_to_tensor(&katom->tv);
    tatom = tensor_atom_create(g_tensor_atomspace, katom->type, katom->name, ttv);
    tensor_tv_destroy(ttv);

    return tatom;
}

/*
 * Sync tensor atom back to kernel atom
 */
void sync_tensor_atom_to_kernel(TensorAtom *tatom, KernelAtom *katom) {
    if (!tatom || !katom) return;

    convert_tensor_tv_to_kernel(&tatom->tv, &katom->tv);
}

/*
 * Initialize kernel bridge
 */
int kernel_bridge_init(int max_atoms) {
    if (g_tensor_engine) return 0;  /* Already initialized */

    g_tensor_atomspace = tensor_atomspace_create(max_atoms);
    if (!g_tensor_atomspace) return -1;

    g_tensor_engine = tensor_logic_create(g_tensor_atomspace);
    if (!g_tensor_engine) {
        tensor_atomspace_destroy(g_tensor_atomspace);
        g_tensor_atomspace = NULL;
        return -1;
    }

    return 0;
}

/*
 * Shutdown kernel bridge
 */
void kernel_bridge_shutdown(void) {
    if (g_tensor_engine) {
        tensor_logic_destroy(g_tensor_engine);
        g_tensor_engine = NULL;
    }
    if (g_tensor_atomspace) {
        tensor_atomspace_destroy(g_tensor_atomspace);
        g_tensor_atomspace = NULL;
    }
}

/*
 * Process cognitive update using tensor logic
 * Called from opencog.c cognitive_schedule()
 */
void kernel_tensor_cognitive_update(KernelCognitiveState *cs) {
    if (!cs || !g_tensor_engine) return;

    /* Build cognitive state vector from kernel state */
    float cognitive_state[TENSOR_EMBED_DIM];

    /* Encode attention and motivation */
    cognitive_state[0] = cs->attention_level;
    cognitive_state[1] = cs->motivation;
    cognitive_state[2] = (float)cs->cognitive_load / 100.0f;

    /* Encode goal state */
    KernelGoal *goal = cs->active_goals;
    int goal_idx = 3;
    while (goal && goal_idx < TENSOR_EMBED_DIM - 3) {
        cognitive_state[goal_idx++] = goal->urgency;
        cognitive_state[goal_idx++] = goal->importance;
        cognitive_state[goal_idx++] = goal->satisfaction.strength;
        goal = goal->next;
    }

    /* Fill remaining with zeros */
    for (int i = goal_idx; i < TENSOR_EMBED_DIM; i++) {
        cognitive_state[i] = 0.0f;
    }

    /* Run tensor logic cognitive update */
    tensor_cognitive_update_full(g_tensor_engine, cognitive_state, TENSOR_EMBED_DIM);

    /* Update kernel cognitive state from results */
    cs->attention_level = cognitive_state[0];
    cs->motivation = cognitive_state[1];

    /* Clamp values to valid range */
    if (cs->attention_level < 0.0f) cs->attention_level = 0.0f;
    if (cs->attention_level > 1.0f) cs->attention_level = 1.0f;
    if (cs->motivation < 0.0f) cs->motivation = 0.0f;
    if (cs->motivation > 1.0f) cs->motivation = 1.0f;

    /* Update goals based on inference */
    goal = cs->active_goals;
    goal_idx = 3;
    while (goal && goal_idx < TENSOR_EMBED_DIM - 3) {
        float new_satisfaction = cognitive_state[goal_idx + 2];
        if (new_satisfaction > goal->satisfaction.strength) {
            goal->satisfaction.strength = new_satisfaction;
            goal->satisfaction.confidence += 0.01f;
            if (goal->satisfaction.confidence > 1.0f)
                goal->satisfaction.confidence = 1.0f;
        }
        goal_idx += 3;
        goal = goal->next;
    }
}

/*
 * Compute goal gradients for process scheduling
 * Returns attention weight for process priority
 */
float kernel_tensor_compute_goal_priority(KernelGoal *goal) {
    if (!goal || !g_tensor_engine) return 0.5f;

    /* Build goal embedding */
    float goal_embedding[TENSOR_EMBED_DIM];
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        goal_embedding[i] = 0.0f;
    }

    goal_embedding[0] = goal->urgency;
    goal_embedding[1] = goal->importance;
    goal_embedding[2] = 1.0f - goal->satisfaction.strength;  /* How unsatisfied */

    /* Hash description into embedding */
    if (goal->description) {
        unsigned long hash = 5381;
        for (char *c = goal->description; *c; c++) {
            hash = ((hash << 5) + hash) + *c;
        }
        for (int i = 3; i < TENSOR_EMBED_DIM; i++) {
            goal_embedding[i] = (float)((hash >> (i % 32)) & 0xFF) / 255.0f * 0.1f;
        }
    }

    /* Compute gradient towards goal */
    float gradient[TENSOR_EMBED_DIM];
    tensor_cognitive_goal_gradient(g_tensor_engine, goal_embedding, gradient);

    /* Compute priority as magnitude of gradient */
    float priority = 0.0f;
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        priority += gradient[i] * gradient[i];
    }
    priority = sqrtf(priority);

    /* Combine with urgency and importance */
    priority = (priority + goal->urgency * 0.3f + goal->importance * 0.3f) / 1.6f;

    /* Clamp to [0, 1] */
    if (priority < 0.0f) priority = 0.0f;
    if (priority > 1.0f) priority = 1.0f;

    return priority;
}

/*
 * Run inference for a query
 */
int kernel_tensor_infer(char *query_name, float *result_strength, float *result_confidence) {
    if (!query_name || !g_tensor_engine) return -1;

    /* Find or create query atom */
    TensorAtom *query = tensor_atom_find(g_tensor_atomspace, query_name);
    if (!query) {
        TensorTruthValue *tv = tensor_tv_create(0.5f, 0.1f);
        query = tensor_atom_create(g_tensor_atomspace, 0, query_name, tv);
        tensor_tv_destroy(tv);
        if (!query) return -1;
    }

    /* Run inference */
    TensorInference *result = tensor_logic_infer(g_tensor_engine, query, 10);

    if (result && result->conclusion) {
        if (result_strength) *result_strength = result->conclusion->tv.strength;
        if (result_confidence) *result_confidence = result->conclusion->tv.confidence;
        return 0;
    }

    return -1;
}

/*
 * Add inference rule from kernel
 */
int kernel_tensor_add_rule(char *name,
                           char **premise_names, int num_premises,
                           char *conclusion_name) {
    if (!name || !premise_names || num_premises <= 0 || !conclusion_name) return -1;
    if (!g_tensor_engine || !g_tensor_atomspace) return -1;

    /* Get or create premise atoms */
    TensorAtom **premises = malloc(num_premises * sizeof(TensorAtom*));
    if (!premises) return -1;

    for (int i = 0; i < num_premises; i++) {
        premises[i] = tensor_atom_find(g_tensor_atomspace, premise_names[i]);
        if (!premises[i]) {
            TensorTruthValue *tv = tensor_tv_create(0.5f, 0.1f);
            premises[i] = tensor_atom_create(g_tensor_atomspace, 0, premise_names[i], tv);
            tensor_tv_destroy(tv);
            if (!premises[i]) {
                free(premises);
                return -1;
            }
        }
    }

    /* Get or create conclusion atom */
    TensorAtom *conclusion = tensor_atom_find(g_tensor_atomspace, conclusion_name);
    if (!conclusion) {
        TensorTruthValue *tv = tensor_tv_create(0.5f, 0.1f);
        conclusion = tensor_atom_create(g_tensor_atomspace, 0, conclusion_name, tv);
        tensor_tv_destroy(tv);
        if (!conclusion) {
            free(premises);
            return -1;
        }
    }

    /* Create and add rule */
    TensorRule *rule = tensor_rule_create(name, premises, num_premises, conclusion);
    free(premises);

    if (!rule) return -1;

    tensor_logic_add_rule(g_tensor_engine, rule);
    return 0;
}

/*
 * Sync entire kernel atomspace to tensor atomspace
 */
void kernel_tensor_sync_atomspace(KernelAtomSpace *kas) {
    if (!kas || !g_tensor_atomspace) return;

    for (int i = 0; i < kas->maxatoms; i++) {
        KernelAtom *katom = kas->atoms[i];
        while (katom) {
            sync_kernel_atom_to_tensor(katom);
            katom = katom->next;
        }
    }

    /* Update embeddings after sync */
    tensor_atomspace_update_embeddings(g_tensor_atomspace);
}

/*
 * Sync tensor results back to kernel atomspace
 */
void kernel_tensor_sync_back(KernelAtomSpace *kas) {
    if (!kas || !g_tensor_atomspace) return;

    for (int i = 0; i < kas->maxatoms; i++) {
        KernelAtom *katom = kas->atoms[i];
        while (katom) {
            TensorAtom *tatom = tensor_atom_find(g_tensor_atomspace, katom->name);
            if (tatom) {
                sync_tensor_atom_to_kernel(tatom, katom);
            }
            katom = katom->next;
        }
    }
}

/*
 * Get tensor logic statistics
 */
void kernel_tensor_get_stats(int *num_atoms, int *num_rules,
                             long long *training_steps, float *avg_attention) {
    if (!g_tensor_engine) return;

    if (num_atoms) *num_atoms = g_tensor_atomspace->natoms;
    if (num_rules) *num_rules = g_tensor_engine->num_rules;
    if (training_steps) *training_steps = g_tensor_atomspace->training_steps;

    if (avg_attention && g_tensor_atomspace->attention_scores) {
        float sum = 0.0f;
        for (int i = 0; i < g_tensor_atomspace->natoms; i++) {
            sum += g_tensor_atomspace->attention_scores[i];
        }
        *avg_attention = (g_tensor_atomspace->natoms > 0) ?
                         sum / g_tensor_atomspace->natoms : 0.0f;
    }
}

/*
 * Train tensor logic on example
 */
int kernel_tensor_train(char *query_name, float target_strength, float target_confidence) {
    if (!query_name || !g_tensor_engine) return -1;

    TensorAtom *query = tensor_atom_find(g_tensor_atomspace, query_name);
    if (!query) return -1;

    TensorTruthValue *target = tensor_tv_create(target_strength, target_confidence);
    if (!target) return -1;

    tensor_logic_train_step(g_tensor_engine, query, target);
    tensor_tv_destroy(target);

    return 0;
}

/*
 * Distributed synchronization
 */
void kernel_tensor_distributed_sync(int node_id) {
    if (!g_tensor_engine) return;
    tensor_logic_sync(g_tensor_engine, node_id);
}
