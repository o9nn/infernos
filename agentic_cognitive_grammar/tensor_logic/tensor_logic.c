/*
 * Tensor Logic Implementation - Neural-Symbolic Unification for AGI
 *
 * Implements the core tensor logic operations that unify deep learning
 * with symbolic reasoning for the OpenCog kernel AGI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tensor_logic.h"

/* Math constants */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
 * Activation Functions
 */
float tensor_sigmoid(float x) {
    if (x > 20.0f) return 1.0f;
    if (x < -20.0f) return 0.0f;
    return 1.0f / (1.0f + expf(-x));
}

float tensor_tanh(float x) {
    return tanhf(x);
}

float tensor_relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

float tensor_softmax_single(float x, float *values, int n) {
    float max_val = values[0];
    for (int i = 1; i < n; i++) {
        if (values[i] > max_val) max_val = values[i];
    }

    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += expf(values[i] - max_val);
    }

    return expf(x - max_val) / sum;
}

/* Softmax over an array in-place */
static void softmax_inplace(float *values, int n) {
    float max_val = values[0];
    for (int i = 1; i < n; i++) {
        if (values[i] > max_val) max_val = values[i];
    }

    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        values[i] = expf(values[i] - max_val);
        sum += values[i];
    }

    for (int i = 0; i < n; i++) {
        values[i] /= sum;
    }
}

/* Dot product of two vectors */
static float dot_product(float *a, float *b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

/* Vector addition: c = a + b */
static void vec_add(float *c, float *a, float *b, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Vector scale: a = a * scale */
static void vec_scale(float *a, float scale, int n) {
    for (int i = 0; i < n; i++) {
        a[i] *= scale;
    }
}

/* Initialize random embedding */
static void init_random_embedding(float *embedding, int size) {
    for (int i = 0; i < size; i++) {
        /* Xavier initialization */
        float scale = sqrtf(2.0f / size);
        embedding[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * scale;
    }
}

/*
 * Truth Value Operations - Probabilistic Logic Network (PLN) style
 */
TensorTruthValue* tensor_tv_create(float strength, float confidence) {
    TensorTruthValue *tv = malloc(sizeof(TensorTruthValue));
    if (!tv) return NULL;

    tv->strength = strength;
    tv->confidence = confidence;
    tv->evidence = confidence / (1.0f - confidence + 1e-10f);

    /* Initialize embedding based on truth value */
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        float angle = (float)i * M_PI / TENSOR_EMBED_DIM;
        tv->embedding[i] = strength * cosf(angle) + confidence * sinf(angle);
        tv->gradient[i] = 0.0f;
    }

    return tv;
}

TensorTruthValue* tensor_tv_merge(TensorTruthValue *tv1, TensorTruthValue *tv2) {
    if (!tv1 || !tv2) return NULL;

    /* Weighted merge based on confidence */
    float w1 = tv1->confidence;
    float w2 = tv2->confidence;
    float total = w1 + w2 + 1e-10f;

    TensorTruthValue *result = malloc(sizeof(TensorTruthValue));
    if (!result) return NULL;

    result->strength = (w1 * tv1->strength + w2 * tv2->strength) / total;
    result->confidence = (w1 + w2) / (1.0f + w1 + w2);
    result->evidence = tv1->evidence + tv2->evidence;

    /* Merge embeddings */
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        result->embedding[i] = (w1 * tv1->embedding[i] + w2 * tv2->embedding[i]) / total;
        result->gradient[i] = 0.0f;
    }

    return result;
}

TensorTruthValue* tensor_tv_revision(TensorTruthValue *tv1, TensorTruthValue *tv2) {
    if (!tv1 || !tv2) return NULL;

    /* PLN revision rule */
    float k = tv1->evidence + tv2->evidence;
    float w1 = tv1->evidence / k;
    float w2 = tv2->evidence / k;

    TensorTruthValue *result = malloc(sizeof(TensorTruthValue));
    if (!result) return NULL;

    result->strength = w1 * tv1->strength + w2 * tv2->strength;
    result->evidence = k;
    result->confidence = k / (k + 1.0f);

    /* Blend embeddings using attention-weighted combination */
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        result->embedding[i] = w1 * tv1->embedding[i] + w2 * tv2->embedding[i];
        result->gradient[i] = 0.0f;
    }

    return result;
}

TensorTruthValue* tensor_tv_deduction(TensorTruthValue *tv1, TensorTruthValue *tv2) {
    if (!tv1 || !tv2) return NULL;

    /* PLN deduction: (A->B) and (B->C) => (A->C) */
    TensorTruthValue *result = malloc(sizeof(TensorTruthValue));
    if (!result) return NULL;

    float s1 = tv1->strength;
    float s2 = tv2->strength;
    float c1 = tv1->confidence;
    float c2 = tv2->confidence;

    /* Deduction formula */
    result->strength = s1 * s2;
    result->confidence = c1 * c2 * (s1 * s2 + (1 - s1) * (1 - s2));
    result->evidence = fminf(tv1->evidence, tv2->evidence);

    /* Deduction in embedding space: element-wise product */
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        result->embedding[i] = tv1->embedding[i] * tv2->embedding[i];
        result->gradient[i] = 0.0f;
    }

    return result;
}

TensorTruthValue* tensor_tv_induction(TensorTruthValue *tv1, TensorTruthValue *tv2) {
    if (!tv1 || !tv2) return NULL;

    /* PLN induction: (A->B) and (A->C) => (B->C) */
    TensorTruthValue *result = malloc(sizeof(TensorTruthValue));
    if (!result) return NULL;

    float s1 = tv1->strength;
    float s2 = tv2->strength;

    result->strength = s2;
    result->confidence = tv1->confidence * tv2->confidence * s1;
    result->evidence = fminf(tv1->evidence, tv2->evidence) * s1;

    /* Induction in embedding space */
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        result->embedding[i] = (tv1->embedding[i] + tv2->embedding[i]) * 0.5f * s1;
        result->gradient[i] = 0.0f;
    }

    return result;
}

TensorTruthValue* tensor_tv_abduction(TensorTruthValue *tv1, TensorTruthValue *tv2) {
    if (!tv1 || !tv2) return NULL;

    /* PLN abduction: (A->B) and (C->B) => (A->C) */
    TensorTruthValue *result = malloc(sizeof(TensorTruthValue));
    if (!result) return NULL;

    float s1 = tv1->strength;
    float s2 = tv2->strength;

    result->strength = s1;
    result->confidence = tv1->confidence * tv2->confidence * s2;
    result->evidence = fminf(tv1->evidence, tv2->evidence) * s2;

    /* Abduction in embedding space */
    float sim = dot_product(tv1->embedding, tv2->embedding, TENSOR_EMBED_DIM);
    sim = tensor_sigmoid(sim);

    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        result->embedding[i] = tv1->embedding[i] * sim;
        result->gradient[i] = 0.0f;
    }

    return result;
}

void tensor_tv_destroy(TensorTruthValue *tv) {
    if (tv) free(tv);
}

/*
 * Tensor Atom Operations
 */
TensorAtom* tensor_atom_create(TensorAtomSpace *as, int type, char *name, TensorTruthValue *tv) {
    if (!as || !name) return NULL;
    if (as->natoms >= as->maxatoms) return NULL;

    TensorAtom *atom = malloc(sizeof(TensorAtom));
    if (!atom) return NULL;

    atom->id = as->next_id++;
    atom->type = type;
    atom->name = strdup(name);
    if (!atom->name) {
        free(atom);
        return NULL;
    }

    /* Initialize truth value */
    if (tv) {
        atom->tv = *tv;
    } else {
        atom->tv.strength = 0.5f;
        atom->tv.confidence = 0.1f;
        atom->tv.evidence = 0.11f;
        init_random_embedding(atom->tv.embedding, TENSOR_EMBED_DIM);
        memset(atom->tv.gradient, 0, sizeof(atom->tv.gradient));
    }

    /* Initialize embedding from name hash + truth value */
    unsigned long hash = 5381;
    for (char *c = name; *c; c++) {
        hash = ((hash << 5) + hash) + *c;
    }

    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        float h = (float)((hash >> (i % 32)) & 0xFF) / 255.0f;
        atom->embedding[i] = atom->tv.embedding[i] * 0.5f + h * 0.5f;
    }

    atom->attention_weight = 1.0f / as->maxatoms;
    atom->outgoing = NULL;
    atom->arity = 0;

    /* Add to atomspace hash table */
    unsigned long idx = atom->id % as->maxatoms;
    atom->next = as->atoms[idx];
    as->atoms[idx] = atom;
    as->natoms++;

    /* Update atomspace embeddings */
    if (as->atom_embeddings) {
        memcpy(as->atom_embeddings + atom->id * TENSOR_EMBED_DIM,
               atom->embedding, TENSOR_EMBED_DIM * sizeof(float));
    }

    return atom;
}

TensorAtom* tensor_atom_find(TensorAtomSpace *as, char *name) {
    if (!as || !name) return NULL;

    for (int i = 0; i < as->maxatoms; i++) {
        TensorAtom *atom = as->atoms[i];
        while (atom) {
            if (atom->name && strcmp(atom->name, name) == 0) {
                return atom;
            }
            atom = atom->next;
        }
    }

    return NULL;
}

TensorAtom* tensor_atom_find_by_id(TensorAtomSpace *as, unsigned long id) {
    if (!as) return NULL;

    unsigned long idx = id % as->maxatoms;
    TensorAtom *atom = as->atoms[idx];

    while (atom) {
        if (atom->id == id) return atom;
        atom = atom->next;
    }

    return NULL;
}

void tensor_atom_add_link(TensorAtom *atom, TensorAtom *target) {
    if (!atom || !target) return;

    atom->outgoing = realloc(atom->outgoing, (atom->arity + 1) * sizeof(TensorAtom*));
    if (atom->outgoing) {
        atom->outgoing[atom->arity] = target;
        atom->arity++;

        /* Update embedding based on link */
        for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
            atom->embedding[i] = (atom->embedding[i] * atom->arity +
                                  target->embedding[i]) / (atom->arity + 1);
        }
    }
}

float tensor_atom_similarity(TensorAtom *a1, TensorAtom *a2) {
    if (!a1 || !a2) return 0.0f;

    /* Cosine similarity in embedding space */
    float dot = 0.0f, norm1 = 0.0f, norm2 = 0.0f;

    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        dot += a1->embedding[i] * a2->embedding[i];
        norm1 += a1->embedding[i] * a1->embedding[i];
        norm2 += a2->embedding[i] * a2->embedding[i];
    }

    float denom = sqrtf(norm1) * sqrtf(norm2);
    if (denom < 1e-10f) return 0.0f;

    return dot / denom;
}

int tensor_atom_unify(TensorAtom *pattern, TensorAtom *target, TensorAtom ***bindings) {
    if (!pattern || !target) return 0;

    /* Check type compatibility */
    if (pattern->type != target->type) return 0;

    /* Check embedding similarity threshold */
    float sim = tensor_atom_similarity(pattern, target);
    if (sim < 0.7f) return 0;

    /* Check structural compatibility for links */
    if (pattern->arity != target->arity) return 0;

    for (int i = 0; i < pattern->arity; i++) {
        if (!tensor_atom_unify(pattern->outgoing[i], target->outgoing[i], NULL)) {
            return 0;
        }
    }

    return 1;
}

void tensor_atom_update_embedding(TensorAtom *atom, float *new_embedding) {
    if (!atom || !new_embedding) return;
    memcpy(atom->embedding, new_embedding, TENSOR_EMBED_DIM * sizeof(float));
}

void tensor_atom_destroy(TensorAtom *atom) {
    if (!atom) return;
    if (atom->name) free(atom->name);
    if (atom->outgoing) free(atom->outgoing);
    free(atom);
}

/*
 * Tensor AtomSpace Operations
 */
TensorAtomSpace* tensor_atomspace_create(int maxatoms) {
    if (maxatoms <= 0) return NULL;

    TensorAtomSpace *as = malloc(sizeof(TensorAtomSpace));
    if (!as) return NULL;

    as->atoms = calloc(maxatoms, sizeof(TensorAtom*));
    if (!as->atoms) {
        free(as);
        return NULL;
    }

    as->natoms = 0;
    as->maxatoms = maxatoms;
    as->next_id = 1;

    /* Allocate neural components */
    as->atom_embeddings = calloc(maxatoms * TENSOR_EMBED_DIM, sizeof(float));
    as->relation_matrix = calloc(maxatoms * maxatoms, sizeof(float));
    as->attention_scores = calloc(maxatoms, sizeof(float));

    as->learning_rate = 0.001f;
    as->momentum = 0.9f;
    as->training_steps = 0;

    return as;
}

void tensor_atomspace_destroy(TensorAtomSpace *as) {
    if (!as) return;

    /* Free all atoms */
    for (int i = 0; i < as->maxatoms; i++) {
        TensorAtom *atom = as->atoms[i];
        while (atom) {
            TensorAtom *next = atom->next;
            tensor_atom_destroy(atom);
            atom = next;
        }
    }

    free(as->atoms);
    if (as->atom_embeddings) free(as->atom_embeddings);
    if (as->relation_matrix) free(as->relation_matrix);
    if (as->attention_scores) free(as->attention_scores);
    free(as);
}

void tensor_atomspace_update_embeddings(TensorAtomSpace *as) {
    if (!as || !as->atom_embeddings) return;

    /* Update relation matrix based on atom embeddings */
    for (int i = 0; i < as->natoms; i++) {
        TensorAtom *ai = tensor_atom_find_by_id(as, i + 1);
        if (!ai) continue;

        for (int j = i + 1; j < as->natoms; j++) {
            TensorAtom *aj = tensor_atom_find_by_id(as, j + 1);
            if (!aj) continue;

            float sim = tensor_atom_similarity(ai, aj);
            as->relation_matrix[i * as->maxatoms + j] = sim;
            as->relation_matrix[j * as->maxatoms + i] = sim;
        }
    }
}

void tensor_atomspace_compute_attention(TensorAtomSpace *as, float *query) {
    if (!as || !query || !as->attention_scores) return;

    /* Compute attention scores for all atoms */
    for (int i = 0; i < as->natoms; i++) {
        TensorAtom *atom = tensor_atom_find_by_id(as, i + 1);
        if (!atom) {
            as->attention_scores[i] = 0.0f;
            continue;
        }

        /* Scaled dot-product attention */
        float score = dot_product(query, atom->embedding, TENSOR_EMBED_DIM);
        score /= sqrtf(TENSOR_EMBED_DIM);
        as->attention_scores[i] = score;
    }

    /* Apply softmax */
    softmax_inplace(as->attention_scores, as->natoms);

    /* Update atom attention weights */
    for (int i = 0; i < as->natoms; i++) {
        TensorAtom *atom = tensor_atom_find_by_id(as, i + 1);
        if (atom) {
            atom->attention_weight = as->attention_scores[i];
        }
    }
}

TensorAtom** tensor_atomspace_get_top_k(TensorAtomSpace *as, int k) {
    if (!as || k <= 0) return NULL;

    if (k > as->natoms) k = as->natoms;

    TensorAtom **result = malloc(k * sizeof(TensorAtom*));
    if (!result) return NULL;

    /* Simple selection of top-k by attention weight */
    int *selected = calloc(as->maxatoms, sizeof(int));

    for (int i = 0; i < k; i++) {
        float max_weight = -1.0f;
        TensorAtom *best = NULL;
        int best_idx = -1;

        for (int j = 0; j < as->natoms; j++) {
            if (selected[j]) continue;

            TensorAtom *atom = tensor_atom_find_by_id(as, j + 1);
            if (atom && atom->attention_weight > max_weight) {
                max_weight = atom->attention_weight;
                best = atom;
                best_idx = j;
            }
        }

        if (best) {
            result[i] = best;
            selected[best_idx] = 1;
        }
    }

    free(selected);
    return result;
}

/*
 * Tensor Rule Operations
 */
TensorRule* tensor_rule_create(char *name, TensorAtom **premises, int num_premises, TensorAtom *conclusion) {
    if (!name || !premises || num_premises <= 0 || !conclusion) return NULL;
    if (num_premises > 16) return NULL;

    TensorRule *rule = malloc(sizeof(TensorRule));
    if (!rule) return NULL;

    static unsigned long rule_id = 1;
    rule->id = rule_id++;
    rule->name = strdup(name);
    rule->weight = 1.0f;
    rule->confidence = 0.8f;

    rule->premises = malloc(num_premises * sizeof(TensorAtom*));
    if (!rule->premises) {
        free(rule->name);
        free(rule);
        return NULL;
    }
    memcpy(rule->premises, premises, num_premises * sizeof(TensorAtom*));
    rule->num_premises = num_premises;
    rule->conclusion = conclusion;

    /* Initialize premise weights uniformly */
    for (int i = 0; i < num_premises; i++) {
        rule->premise_weights[i] = 1.0f / num_premises;
    }
    for (int i = num_premises; i < 16; i++) {
        rule->premise_weights[i] = 0.0f;
    }

    init_random_embedding(rule->hidden_state, TENSOR_HIDDEN_DIM);
    memset(rule->gradient, 0, sizeof(rule->gradient));

    rule->next = NULL;
    return rule;
}

void tensor_rule_destroy(TensorRule *rule) {
    if (!rule) return;
    if (rule->name) free(rule->name);
    if (rule->premises) free(rule->premises);
    free(rule);
}

float tensor_rule_apply(TensorRule *rule, TensorAtomSpace *as) {
    if (!rule || !as) return 0.0f;

    /* Compute weighted combination of premise truth values */
    float combined_strength = 0.0f;
    float combined_confidence = 1.0f;

    for (int i = 0; i < rule->num_premises; i++) {
        TensorAtom *premise = rule->premises[i];
        if (!premise) continue;

        combined_strength += rule->premise_weights[i] * premise->tv.strength;
        combined_confidence *= premise->tv.confidence;
    }

    /* Apply rule to update conclusion */
    float new_strength = combined_strength * rule->weight;
    float new_confidence = combined_confidence * rule->confidence;

    rule->conclusion->tv.strength = (rule->conclusion->tv.strength + new_strength) / 2.0f;
    rule->conclusion->tv.confidence = (rule->conclusion->tv.confidence + new_confidence) / 2.0f;

    /* Update conclusion embedding using neural transformation */
    float combined_embedding[TENSOR_EMBED_DIM] = {0};

    for (int i = 0; i < rule->num_premises; i++) {
        TensorAtom *premise = rule->premises[i];
        if (!premise) continue;

        for (int j = 0; j < TENSOR_EMBED_DIM; j++) {
            combined_embedding[j] += rule->premise_weights[i] * premise->embedding[j];
        }
    }

    /* Apply non-linearity */
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        combined_embedding[i] = tensor_tanh(combined_embedding[i]);
        rule->conclusion->embedding[i] = 0.9f * rule->conclusion->embedding[i] +
                                          0.1f * combined_embedding[i];
    }

    return new_strength;
}

void tensor_rule_update_weights(TensorRule *rule, GradientContext *ctx) {
    if (!rule || !ctx) return;

    /* Update rule weight using gradient */
    float grad = 0.0f;
    for (int i = 0; i < TENSOR_HIDDEN_DIM && i < ctx->grad_size; i++) {
        grad += rule->gradient[i] * ctx->gradients[i];
    }

    rule->weight -= ctx->gradients[0] * 0.01f;  /* Simple SGD for rule weight */
    if (rule->weight < 0.0f) rule->weight = 0.0f;
    if (rule->weight > 2.0f) rule->weight = 2.0f;

    /* Update premise weights */
    for (int i = 0; i < rule->num_premises; i++) {
        rule->premise_weights[i] -= ctx->gradients[i + 1] * 0.001f;
        if (rule->premise_weights[i] < 0.01f) rule->premise_weights[i] = 0.01f;
    }

    /* Normalize premise weights */
    float sum = 0.0f;
    for (int i = 0; i < rule->num_premises; i++) {
        sum += rule->premise_weights[i];
    }
    for (int i = 0; i < rule->num_premises; i++) {
        rule->premise_weights[i] /= sum;
    }
}

/*
 * Tensor Logic Engine Operations
 */
TensorLogicEngine* tensor_logic_create(TensorAtomSpace *as) {
    if (!as) return NULL;

    TensorLogicEngine *engine = malloc(sizeof(TensorLogicEngine));
    if (!engine) return NULL;

    engine->atomspace = as;
    engine->rules = NULL;
    engine->num_rules = 0;
    engine->inference_chain = NULL;
    engine->inference_depth = 0;
    engine->max_depth = 10;

    /* Allocate neural network weights */
    int qkv_size = TENSOR_EMBED_DIM * TENSOR_HIDDEN_DIM;
    engine->query_weights = malloc(qkv_size * sizeof(float));
    engine->key_weights = malloc(qkv_size * sizeof(float));
    engine->value_weights = malloc(qkv_size * sizeof(float));
    engine->output_weights = malloc(qkv_size * sizeof(float));

    if (!engine->query_weights || !engine->key_weights ||
        !engine->value_weights || !engine->output_weights) {
        tensor_logic_destroy(engine);
        return NULL;
    }

    /* Initialize weights */
    init_random_embedding(engine->query_weights, qkv_size);
    init_random_embedding(engine->key_weights, qkv_size);
    init_random_embedding(engine->value_weights, qkv_size);
    init_random_embedding(engine->output_weights, qkv_size);

    engine->grad_ctx = gradient_context_create(qkv_size * 4);
    engine->temperature = 1.0f;
    engine->training_mode = 0;

    return engine;
}

void tensor_logic_destroy(TensorLogicEngine *engine) {
    if (!engine) return;

    /* Free rules */
    TensorRule *rule = engine->rules;
    while (rule) {
        TensorRule *next = rule->next;
        tensor_rule_destroy(rule);
        rule = next;
    }

    /* Free inference chain */
    TensorInference *inf = engine->inference_chain;
    while (inf) {
        TensorInference *next = inf->next;
        if (inf->attention_pattern) free(inf->attention_pattern);
        free(inf);
        inf = next;
    }

    /* Free weights */
    if (engine->query_weights) free(engine->query_weights);
    if (engine->key_weights) free(engine->key_weights);
    if (engine->value_weights) free(engine->value_weights);
    if (engine->output_weights) free(engine->output_weights);

    if (engine->grad_ctx) gradient_context_destroy(engine->grad_ctx);

    free(engine);
}

void tensor_logic_add_rule(TensorLogicEngine *engine, TensorRule *rule) {
    if (!engine || !rule) return;

    rule->next = engine->rules;
    engine->rules = rule;
    engine->num_rules++;
}

TensorInference* tensor_logic_infer(TensorLogicEngine *engine, TensorAtom *query, int max_steps) {
    if (!engine || !query) return NULL;

    /* Compute attention over atomspace using query */
    tensor_atomspace_compute_attention(engine->atomspace, query->embedding);

    /* Get top-k relevant atoms */
    int k = 10;
    TensorAtom **relevant = tensor_atomspace_get_top_k(engine->atomspace, k);
    if (!relevant) return NULL;

    TensorInference *chain = NULL;
    TensorInference *last = NULL;

    for (int step = 0; step < max_steps; step++) {
        /* Find applicable rules */
        TensorRule *best_rule = NULL;
        float best_score = 0.0f;

        for (TensorRule *rule = engine->rules; rule; rule = rule->next) {
            /* Check if premises are satisfied */
            float score = 0.0f;
            int satisfied = 1;

            for (int i = 0; i < rule->num_premises; i++) {
                TensorAtom *premise = rule->premises[i];
                float max_sim = 0.0f;

                for (int j = 0; j < k && relevant[j]; j++) {
                    float sim = tensor_atom_similarity(premise, relevant[j]);
                    if (sim > max_sim) max_sim = sim;
                }

                if (max_sim < 0.5f) {
                    satisfied = 0;
                    break;
                }
                score += max_sim * rule->premise_weights[i];
            }

            if (satisfied && score > best_score) {
                best_score = score;
                best_rule = rule;
            }
        }

        if (!best_rule) break;

        /* Apply best rule */
        float result_strength = tensor_rule_apply(best_rule, engine->atomspace);

        /* Create inference record */
        TensorInference *inf = malloc(sizeof(TensorInference));
        if (!inf) break;

        inf->rule = best_rule;
        inf->conclusion = best_rule->conclusion;
        inf->confidence = result_strength * best_rule->confidence;
        inf->attention_pattern = malloc(k * sizeof(float));
        if (inf->attention_pattern) {
            for (int i = 0; i < k && relevant[i]; i++) {
                inf->attention_pattern[i] = relevant[i]->attention_weight;
            }
        }
        inf->next = NULL;

        if (last) {
            last->next = inf;
        } else {
            chain = inf;
        }
        last = inf;

        /* Check if query is satisfied */
        float query_sim = tensor_atom_similarity(query, best_rule->conclusion);
        if (query_sim > 0.9f) break;
    }

    free(relevant);

    engine->inference_chain = chain;
    return chain;
}

void tensor_logic_train_step(TensorLogicEngine *engine, TensorAtom *query, TensorTruthValue *target) {
    if (!engine || !query || !target) return;

    engine->training_mode = 1;

    /* Forward pass */
    TensorInference *result = tensor_logic_infer(engine, query, 5);

    /* Compute loss */
    if (result && result->conclusion) {
        float pred_strength = result->conclusion->tv.strength;
        float target_strength = target->strength;
        float loss = (pred_strength - target_strength) * (pred_strength - target_strength);

        engine->grad_ctx->loss = loss;

        /* Backward pass */
        tensor_logic_backward(engine);
    }

    engine->training_mode = 0;
}

void tensor_logic_backward(TensorLogicEngine *engine) {
    if (!engine || !engine->inference_chain) return;

    gradient_context_zero(engine->grad_ctx);

    /* Backpropagate through inference chain */
    for (TensorInference *inf = engine->inference_chain; inf; inf = inf->next) {
        /* Compute gradients for rule */
        tensor_rule_update_weights(inf->rule, engine->grad_ctx);
    }

    /* Apply gradients to engine weights using Adam */
    int size = TENSOR_EMBED_DIM * TENSOR_HIDDEN_DIM;
    gradient_context_apply_adam(engine->grad_ctx, engine->query_weights, size);
    gradient_context_apply_adam(engine->grad_ctx, engine->key_weights, size);
    gradient_context_apply_adam(engine->grad_ctx, engine->value_weights, size);
    gradient_context_apply_adam(engine->grad_ctx, engine->output_weights, size);

    engine->atomspace->training_steps++;
}

/*
 * Gradient Context Operations
 */
GradientContext* gradient_context_create(int size) {
    GradientContext *ctx = malloc(sizeof(GradientContext));
    if (!ctx) return NULL;

    ctx->gradients = calloc(size, sizeof(float));
    ctx->m = calloc(size, sizeof(float));
    ctx->v = calloc(size, sizeof(float));

    if (!ctx->gradients || !ctx->m || !ctx->v) {
        gradient_context_destroy(ctx);
        return NULL;
    }

    ctx->grad_size = size;
    ctx->loss = 0.0f;
    ctx->num_steps = 0;

    /* Adam hyperparameters */
    ctx->beta1 = 0.9f;
    ctx->beta2 = 0.999f;
    ctx->epsilon = 1e-8f;

    return ctx;
}

void gradient_context_destroy(GradientContext *ctx) {
    if (!ctx) return;
    if (ctx->gradients) free(ctx->gradients);
    if (ctx->m) free(ctx->m);
    if (ctx->v) free(ctx->v);
    free(ctx);
}

void gradient_context_zero(GradientContext *ctx) {
    if (!ctx) return;
    memset(ctx->gradients, 0, ctx->grad_size * sizeof(float));
    ctx->loss = 0.0f;
}

void gradient_context_accumulate(GradientContext *ctx, float *grads, int size) {
    if (!ctx || !grads) return;

    int n = (size < ctx->grad_size) ? size : ctx->grad_size;
    for (int i = 0; i < n; i++) {
        ctx->gradients[i] += grads[i];
    }
}

void gradient_context_apply_adam(GradientContext *ctx, float *weights, int size) {
    if (!ctx || !weights) return;

    ctx->num_steps++;
    float lr = 0.001f;

    float bias_correction1 = 1.0f - powf(ctx->beta1, ctx->num_steps);
    float bias_correction2 = 1.0f - powf(ctx->beta2, ctx->num_steps);

    int n = (size < ctx->grad_size) ? size : ctx->grad_size;
    for (int i = 0; i < n; i++) {
        float g = ctx->gradients[i];

        /* Update biased first moment estimate */
        ctx->m[i] = ctx->beta1 * ctx->m[i] + (1.0f - ctx->beta1) * g;

        /* Update biased second raw moment estimate */
        ctx->v[i] = ctx->beta2 * ctx->v[i] + (1.0f - ctx->beta2) * g * g;

        /* Compute bias-corrected estimates */
        float m_hat = ctx->m[i] / bias_correction1;
        float v_hat = ctx->v[i] / bias_correction2;

        /* Update weights */
        weights[i] -= lr * m_hat / (sqrtf(v_hat) + ctx->epsilon);
    }
}

/*
 * Attention Mechanism
 */
void tensor_attention_forward(TensorLogicEngine *engine, TensorAtom **atoms, int n, float *output) {
    if (!engine || !atoms || !output) return;

    float *queries = malloc(n * TENSOR_HIDDEN_DIM * sizeof(float));
    float *keys = malloc(n * TENSOR_HIDDEN_DIM * sizeof(float));
    float *values = malloc(n * TENSOR_HIDDEN_DIM * sizeof(float));
    float *scores = malloc(n * n * sizeof(float));

    if (!queries || !keys || !values || !scores) {
        if (queries) free(queries);
        if (keys) free(keys);
        if (values) free(values);
        if (scores) free(scores);
        return;
    }

    /* Project atoms to Q, K, V */
    for (int i = 0; i < n; i++) {
        if (!atoms[i]) continue;

        for (int j = 0; j < TENSOR_HIDDEN_DIM; j++) {
            queries[i * TENSOR_HIDDEN_DIM + j] = 0.0f;
            keys[i * TENSOR_HIDDEN_DIM + j] = 0.0f;
            values[i * TENSOR_HIDDEN_DIM + j] = 0.0f;

            for (int k = 0; k < TENSOR_EMBED_DIM; k++) {
                queries[i * TENSOR_HIDDEN_DIM + j] +=
                    atoms[i]->embedding[k] * engine->query_weights[k * TENSOR_HIDDEN_DIM + j];
                keys[i * TENSOR_HIDDEN_DIM + j] +=
                    atoms[i]->embedding[k] * engine->key_weights[k * TENSOR_HIDDEN_DIM + j];
                values[i * TENSOR_HIDDEN_DIM + j] +=
                    atoms[i]->embedding[k] * engine->value_weights[k * TENSOR_HIDDEN_DIM + j];
            }
        }
    }

    /* Compute attention scores */
    float scale = 1.0f / sqrtf(TENSOR_HIDDEN_DIM);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scores[i * n + j] = 0.0f;
            for (int k = 0; k < TENSOR_HIDDEN_DIM; k++) {
                scores[i * n + j] += queries[i * TENSOR_HIDDEN_DIM + k] *
                                     keys[j * TENSOR_HIDDEN_DIM + k];
            }
            scores[i * n + j] *= scale / engine->temperature;
        }

        /* Apply softmax per row */
        softmax_inplace(scores + i * n, n);
    }

    /* Compute weighted values */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < TENSOR_EMBED_DIM; j++) {
            output[i * TENSOR_EMBED_DIM + j] = 0.0f;

            for (int k = 0; k < n; k++) {
                float v = 0.0f;
                for (int l = 0; l < TENSOR_HIDDEN_DIM; l++) {
                    v += values[k * TENSOR_HIDDEN_DIM + l] *
                         engine->output_weights[l * TENSOR_EMBED_DIM + j];
                }
                output[i * TENSOR_EMBED_DIM + j] += scores[i * n + k] * v;
            }
        }
    }

    free(queries);
    free(keys);
    free(values);
    free(scores);
}

void tensor_attention_backward(TensorLogicEngine *engine, float *grad_output, TensorAtom **atoms, int n) {
    if (!engine || !grad_output || !atoms) return;

    /* Simplified gradient computation */
    for (int i = 0; i < n; i++) {
        if (!atoms[i]) continue;

        for (int j = 0; j < TENSOR_EMBED_DIM; j++) {
            atoms[i]->tv.gradient[j] += grad_output[i * TENSOR_EMBED_DIM + j] * 0.1f;
        }
    }
}

/*
 * Distributed Operations
 */
void tensor_logic_sync(TensorLogicEngine *engine, int node_id) {
    if (!engine) return;

    /* Mark engine for synchronization */
    /* In a real distributed system, this would send embeddings to other nodes */
    engine->atomspace->training_steps++;
}

void tensor_logic_aggregate(TensorLogicEngine **engines, int num_engines, TensorLogicEngine *output) {
    if (!engines || num_engines <= 0 || !output) return;

    /* Average weights across all engines */
    int size = TENSOR_EMBED_DIM * TENSOR_HIDDEN_DIM;

    for (int i = 0; i < size; i++) {
        output->query_weights[i] = 0.0f;
        output->key_weights[i] = 0.0f;
        output->value_weights[i] = 0.0f;
        output->output_weights[i] = 0.0f;

        for (int e = 0; e < num_engines; e++) {
            output->query_weights[i] += engines[e]->query_weights[i];
            output->key_weights[i] += engines[e]->key_weights[i];
            output->value_weights[i] += engines[e]->value_weights[i];
            output->output_weights[i] += engines[e]->output_weights[i];
        }

        output->query_weights[i] /= num_engines;
        output->key_weights[i] /= num_engines;
        output->value_weights[i] /= num_engines;
        output->output_weights[i] /= num_engines;
    }
}

/*
 * Cognitive Integration - Connect to OpenCog kernel
 */
void tensor_cognitive_update_full(TensorLogicEngine *engine, float *cognitive_state, int state_size) {
    if (!engine || !cognitive_state) return;

    /* Create query from cognitive state */
    float query[TENSOR_EMBED_DIM];
    for (int i = 0; i < TENSOR_EMBED_DIM && i < state_size; i++) {
        query[i] = cognitive_state[i];
    }

    /* Compute attention over atomspace */
    tensor_atomspace_compute_attention(engine->atomspace, query);

    /* Get top relevant atoms and run inference */
    TensorAtom *query_atom = tensor_atom_find(engine->atomspace, "cognitive_query");
    if (!query_atom) {
        /* Create temporary query atom */
        TensorTruthValue *tv = tensor_tv_create(0.8f, 0.5f);
        query_atom = tensor_atom_create(engine->atomspace, 0, "cognitive_query", tv);
        tensor_tv_destroy(tv);
    }

    if (query_atom) {
        tensor_atom_update_embedding(query_atom, query);
        tensor_logic_infer(engine, query_atom, 5);

        /* Update cognitive state from inference results */
        if (engine->inference_chain && engine->inference_chain->conclusion) {
            TensorAtom *result = engine->inference_chain->conclusion;
            for (int i = 0; i < TENSOR_EMBED_DIM && i < state_size; i++) {
                cognitive_state[i] = 0.8f * cognitive_state[i] + 0.2f * result->embedding[i];
            }
        }
    }
}

void tensor_cognitive_goal_gradient(TensorLogicEngine *engine, float *goal_embedding, float *gradient) {
    if (!engine || !goal_embedding || !gradient) return;

    /* Compute gradient towards goal using attention */
    tensor_atomspace_compute_attention(engine->atomspace, goal_embedding);

    /* Get most relevant atoms */
    TensorAtom **top = tensor_atomspace_get_top_k(engine->atomspace, 5);
    if (!top) return;

    /* Compute gradient as direction towards goal */
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        gradient[i] = 0.0f;

        for (int j = 0; j < 5 && top[j]; j++) {
            float diff = goal_embedding[i] - top[j]->embedding[i];
            gradient[i] += diff * top[j]->attention_weight;
        }
    }

    free(top);
}
