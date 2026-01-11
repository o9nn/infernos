/*
 * Exhaustive Unit Tests for Tensor Logic Module
 * Tests neural-symbolic unification for AGI
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Test framework macros */
#define TEST_SUITE(name) \
    printf("\n========================================\n"); \
    printf("Test Suite: %s\n", name); \
    printf("========================================\n")

#define TEST_CASE(name) \
    printf("\n  Testing: %s... ", name); \
    test_count++

#define TEST_PASS() \
    printf("PASS\n"); \
    pass_count++

#define TEST_FAIL(msg) \
    printf("FAIL: %s\n", msg); \
    fail_count++

#define ASSERT_TRUE(cond) \
    if (!(cond)) { TEST_FAIL("Assertion failed: " #cond); return; }

#define ASSERT_FALSE(cond) \
    if (cond) { TEST_FAIL("Assertion should be false: " #cond); return; }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        printf("FAIL: Expected %ld, got %ld\n", (long)(b), (long)(a)); \
        fail_count++; return; \
    }

#define ASSERT_NOT_NULL(ptr) \
    if ((ptr) == NULL) { TEST_FAIL("Unexpected NULL: " #ptr); return; }

#define ASSERT_NULL(ptr) \
    if ((ptr) != NULL) { TEST_FAIL("Expected NULL: " #ptr); return; }

#define ASSERT_FLOAT_EQ(a, b, eps) \
    if (fabs((a) - (b)) > (eps)) { \
        printf("FAIL: Expected %f, got %f\n", (b), (a)); \
        fail_count++; return; \
    }

#define ASSERT_FLOAT_RANGE(a, min, max) \
    if ((a) < (min) || (a) > (max)) { \
        printf("FAIL: %f not in range [%f, %f]\n", (a), (min), (max)); \
        fail_count++; return; \
    }

/* Tensor Logic constants */
#define TENSOR_EMBED_DIM 64
#define TENSOR_HIDDEN_DIM 128
#define TENSOR_MAX_ATOMS 4096

/* Test counters */
static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

/* Simplified structures for standalone testing */
typedef struct TensorTruthValue TensorTruthValue;
typedef struct TensorAtom TensorAtom;
typedef struct TensorAtomSpace TensorAtomSpace;
typedef struct TensorRule TensorRule;
typedef struct TensorInference TensorInference;
typedef struct TensorLogicEngine TensorLogicEngine;
typedef struct GradientContext GradientContext;

struct TensorTruthValue {
    float strength;
    float confidence;
    float evidence;
    float embedding[TENSOR_EMBED_DIM];
    float gradient[TENSOR_EMBED_DIM];
};

struct TensorAtom {
    unsigned long id;
    int type;
    char *name;
    TensorTruthValue tv;
    float embedding[TENSOR_EMBED_DIM];
    float attention_weight;
    TensorAtom **outgoing;
    int arity;
    TensorAtom *next;
};

struct TensorAtomSpace {
    TensorAtom **atoms;
    int natoms;
    int maxatoms;
    unsigned long next_id;
    float *atom_embeddings;
    float *relation_matrix;
    float *attention_scores;
    float learning_rate;
    float momentum;
    int training_steps;
};

struct TensorRule {
    unsigned long id;
    char *name;
    float weight;
    float confidence;
    TensorAtom **premises;
    int num_premises;
    TensorAtom *conclusion;
    float premise_weights[16];
    float hidden_state[TENSOR_HIDDEN_DIM];
    float gradient[TENSOR_HIDDEN_DIM];
    TensorRule *next;
};

struct TensorInference {
    TensorRule *rule;
    TensorAtom *conclusion;
    float confidence;
    float *attention_pattern;
    TensorInference *next;
};

struct GradientContext {
    float *gradients;
    int grad_size;
    float loss;
    int num_steps;
    float *m;
    float *v;
    float beta1;
    float beta2;
    float epsilon;
};

struct TensorLogicEngine {
    TensorAtomSpace *atomspace;
    TensorRule *rules;
    int num_rules;
    TensorInference *inference_chain;
    int inference_depth;
    int max_depth;
    float *query_weights;
    float *key_weights;
    float *value_weights;
    float *output_weights;
    GradientContext *grad_ctx;
    float temperature;
    int training_mode;
};

/* Implementation functions (simplified for testing) */
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

static void init_random_embedding(float *embedding, int size) {
    for (int i = 0; i < size; i++) {
        float scale = sqrtf(2.0f / size);
        embedding[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * scale;
    }
}

static float dot_product(float *a, float *b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

TensorTruthValue* tensor_tv_create(float strength, float confidence) {
    TensorTruthValue *tv = malloc(sizeof(TensorTruthValue));
    if (!tv) return NULL;
    tv->strength = strength;
    tv->confidence = confidence;
    tv->evidence = confidence / (1.0f - confidence + 1e-10f);
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        float angle = (float)i * 3.14159f / TENSOR_EMBED_DIM;
        tv->embedding[i] = strength * cosf(angle) + confidence * sinf(angle);
        tv->gradient[i] = 0.0f;
    }
    return tv;
}

TensorTruthValue* tensor_tv_merge(TensorTruthValue *tv1, TensorTruthValue *tv2) {
    if (!tv1 || !tv2) return NULL;
    float w1 = tv1->confidence;
    float w2 = tv2->confidence;
    float total = w1 + w2 + 1e-10f;
    TensorTruthValue *result = malloc(sizeof(TensorTruthValue));
    if (!result) return NULL;
    result->strength = (w1 * tv1->strength + w2 * tv2->strength) / total;
    result->confidence = (w1 + w2) / (1.0f + w1 + w2);
    result->evidence = tv1->evidence + tv2->evidence;
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        result->embedding[i] = (w1 * tv1->embedding[i] + w2 * tv2->embedding[i]) / total;
        result->gradient[i] = 0.0f;
    }
    return result;
}

TensorTruthValue* tensor_tv_deduction(TensorTruthValue *tv1, TensorTruthValue *tv2) {
    if (!tv1 || !tv2) return NULL;
    TensorTruthValue *result = malloc(sizeof(TensorTruthValue));
    if (!result) return NULL;
    result->strength = tv1->strength * tv2->strength;
    result->confidence = tv1->confidence * tv2->confidence;
    result->evidence = fminf(tv1->evidence, tv2->evidence);
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        result->embedding[i] = tv1->embedding[i] * tv2->embedding[i];
        result->gradient[i] = 0.0f;
    }
    return result;
}

void tensor_tv_destroy(TensorTruthValue *tv) {
    if (tv) free(tv);
}

TensorAtomSpace* tensor_atomspace_create(int maxatoms) {
    if (maxatoms <= 0) return NULL;
    TensorAtomSpace *as = malloc(sizeof(TensorAtomSpace));
    if (!as) return NULL;
    as->atoms = calloc(maxatoms, sizeof(TensorAtom*));
    if (!as->atoms) { free(as); return NULL; }
    as->natoms = 0;
    as->maxatoms = maxatoms;
    as->next_id = 1;
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
    for (int i = 0; i < as->maxatoms; i++) {
        TensorAtom *atom = as->atoms[i];
        while (atom) {
            TensorAtom *next = atom->next;
            if (atom->name) free(atom->name);
            if (atom->outgoing) free(atom->outgoing);
            free(atom);
            atom = next;
        }
    }
    free(as->atoms);
    if (as->atom_embeddings) free(as->atom_embeddings);
    if (as->relation_matrix) free(as->relation_matrix);
    if (as->attention_scores) free(as->attention_scores);
    free(as);
}

TensorAtom* tensor_atom_create(TensorAtomSpace *as, int type, char *name, TensorTruthValue *tv) {
    if (!as || !name) return NULL;
    if (as->natoms >= as->maxatoms) return NULL;
    TensorAtom *atom = malloc(sizeof(TensorAtom));
    if (!atom) return NULL;
    atom->id = as->next_id++;
    atom->type = type;
    atom->name = strdup(name);
    if (!atom->name) { free(atom); return NULL; }
    if (tv) {
        atom->tv = *tv;
    } else {
        atom->tv.strength = 0.5f;
        atom->tv.confidence = 0.1f;
        atom->tv.evidence = 0.11f;
        init_random_embedding(atom->tv.embedding, TENSOR_EMBED_DIM);
        memset(atom->tv.gradient, 0, sizeof(atom->tv.gradient));
    }
    unsigned long hash = 5381;
    for (char *c = name; *c; c++) hash = ((hash << 5) + hash) + *c;
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        float h = (float)((hash >> (i % 32)) & 0xFF) / 255.0f;
        atom->embedding[i] = atom->tv.embedding[i] * 0.5f + h * 0.5f;
    }
    atom->attention_weight = 1.0f / as->maxatoms;
    atom->outgoing = NULL;
    atom->arity = 0;
    unsigned long idx = atom->id % as->maxatoms;
    atom->next = as->atoms[idx];
    as->atoms[idx] = atom;
    as->natoms++;
    return atom;
}

TensorAtom* tensor_atom_find(TensorAtomSpace *as, char *name) {
    if (!as || !name) return NULL;
    for (int i = 0; i < as->maxatoms; i++) {
        TensorAtom *atom = as->atoms[i];
        while (atom) {
            if (atom->name && strcmp(atom->name, name) == 0) return atom;
            atom = atom->next;
        }
    }
    return NULL;
}

float tensor_atom_similarity(TensorAtom *a1, TensorAtom *a2) {
    if (!a1 || !a2) return 0.0f;
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

GradientContext* gradient_context_create(int size) {
    GradientContext *ctx = malloc(sizeof(GradientContext));
    if (!ctx) return NULL;
    ctx->gradients = calloc(size, sizeof(float));
    ctx->m = calloc(size, sizeof(float));
    ctx->v = calloc(size, sizeof(float));
    if (!ctx->gradients || !ctx->m || !ctx->v) {
        if (ctx->gradients) free(ctx->gradients);
        if (ctx->m) free(ctx->m);
        if (ctx->v) free(ctx->v);
        free(ctx);
        return NULL;
    }
    ctx->grad_size = size;
    ctx->loss = 0.0f;
    ctx->num_steps = 0;
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
    int qkv_size = TENSOR_EMBED_DIM * TENSOR_HIDDEN_DIM;
    engine->query_weights = malloc(qkv_size * sizeof(float));
    engine->key_weights = malloc(qkv_size * sizeof(float));
    engine->value_weights = malloc(qkv_size * sizeof(float));
    engine->output_weights = malloc(qkv_size * sizeof(float));
    if (!engine->query_weights || !engine->key_weights ||
        !engine->value_weights || !engine->output_weights) {
        if (engine->query_weights) free(engine->query_weights);
        if (engine->key_weights) free(engine->key_weights);
        if (engine->value_weights) free(engine->value_weights);
        if (engine->output_weights) free(engine->output_weights);
        free(engine);
        return NULL;
    }
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
    TensorRule *rule = engine->rules;
    while (rule) {
        TensorRule *next = rule->next;
        if (rule->name) free(rule->name);
        if (rule->premises) free(rule->premises);
        free(rule);
        rule = next;
    }
    TensorInference *inf = engine->inference_chain;
    while (inf) {
        TensorInference *next = inf->next;
        if (inf->attention_pattern) free(inf->attention_pattern);
        free(inf);
        inf = next;
    }
    if (engine->query_weights) free(engine->query_weights);
    if (engine->key_weights) free(engine->key_weights);
    if (engine->value_weights) free(engine->value_weights);
    if (engine->output_weights) free(engine->output_weights);
    if (engine->grad_ctx) gradient_context_destroy(engine->grad_ctx);
    free(engine);
}

/*
 * TEST FUNCTIONS
 */

/* Activation Function Tests */
void test_sigmoid(void) {
    TEST_CASE("sigmoid activation");
    ASSERT_FLOAT_EQ(tensor_sigmoid(0.0f), 0.5f, 0.001f);
    ASSERT_TRUE(tensor_sigmoid(100.0f) > 0.99f);
    ASSERT_TRUE(tensor_sigmoid(-100.0f) < 0.01f);
    ASSERT_TRUE(tensor_sigmoid(1.0f) > 0.5f);
    ASSERT_TRUE(tensor_sigmoid(-1.0f) < 0.5f);
    TEST_PASS();
}

void test_tanh(void) {
    TEST_CASE("tanh activation");
    ASSERT_FLOAT_EQ(tensor_tanh(0.0f), 0.0f, 0.001f);
    ASSERT_TRUE(tensor_tanh(100.0f) > 0.99f);
    ASSERT_TRUE(tensor_tanh(-100.0f) < -0.99f);
    TEST_PASS();
}

void test_relu(void) {
    TEST_CASE("relu activation");
    ASSERT_FLOAT_EQ(tensor_relu(0.0f), 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(tensor_relu(5.0f), 5.0f, 0.001f);
    ASSERT_FLOAT_EQ(tensor_relu(-5.0f), 0.0f, 0.001f);
    TEST_PASS();
}

/* Truth Value Tests */
void test_tv_create(void) {
    TEST_CASE("truth value creation");
    TensorTruthValue *tv = tensor_tv_create(0.8f, 0.6f);
    ASSERT_NOT_NULL(tv);
    ASSERT_FLOAT_EQ(tv->strength, 0.8f, 0.001f);
    ASSERT_FLOAT_EQ(tv->confidence, 0.6f, 0.001f);
    ASSERT_TRUE(tv->evidence > 0);
    tensor_tv_destroy(tv);
    TEST_PASS();
}

void test_tv_create_boundary(void) {
    TEST_CASE("truth value boundary values");
    TensorTruthValue *tv1 = tensor_tv_create(0.0f, 0.0f);
    TensorTruthValue *tv2 = tensor_tv_create(1.0f, 1.0f);
    ASSERT_NOT_NULL(tv1);
    ASSERT_NOT_NULL(tv2);
    ASSERT_FLOAT_EQ(tv1->strength, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(tv2->strength, 1.0f, 0.001f);
    tensor_tv_destroy(tv1);
    tensor_tv_destroy(tv2);
    TEST_PASS();
}

void test_tv_embedding(void) {
    TEST_CASE("truth value embedding");
    TensorTruthValue *tv = tensor_tv_create(0.7f, 0.5f);
    ASSERT_NOT_NULL(tv);
    float norm = 0.0f;
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        norm += tv->embedding[i] * tv->embedding[i];
    }
    ASSERT_TRUE(norm > 0);
    tensor_tv_destroy(tv);
    TEST_PASS();
}

void test_tv_merge(void) {
    TEST_CASE("truth value merge");
    TensorTruthValue *tv1 = tensor_tv_create(0.8f, 0.6f);
    TensorTruthValue *tv2 = tensor_tv_create(0.4f, 0.3f);
    TensorTruthValue *merged = tensor_tv_merge(tv1, tv2);
    ASSERT_NOT_NULL(merged);
    ASSERT_TRUE(merged->strength >= 0.0f && merged->strength <= 1.0f);
    ASSERT_TRUE(merged->confidence >= 0.0f && merged->confidence <= 1.0f);
    ASSERT_FLOAT_EQ(merged->evidence, tv1->evidence + tv2->evidence, 0.001f);
    tensor_tv_destroy(tv1);
    tensor_tv_destroy(tv2);
    tensor_tv_destroy(merged);
    TEST_PASS();
}

void test_tv_merge_null(void) {
    TEST_CASE("truth value merge null handling");
    TensorTruthValue *tv = tensor_tv_create(0.5f, 0.5f);
    ASSERT_NULL(tensor_tv_merge(NULL, tv));
    ASSERT_NULL(tensor_tv_merge(tv, NULL));
    ASSERT_NULL(tensor_tv_merge(NULL, NULL));
    tensor_tv_destroy(tv);
    TEST_PASS();
}

void test_tv_deduction(void) {
    TEST_CASE("truth value deduction");
    TensorTruthValue *tv1 = tensor_tv_create(0.8f, 0.7f);
    TensorTruthValue *tv2 = tensor_tv_create(0.9f, 0.6f);
    TensorTruthValue *result = tensor_tv_deduction(tv1, tv2);
    ASSERT_NOT_NULL(result);
    ASSERT_FLOAT_EQ(result->strength, 0.8f * 0.9f, 0.001f);
    ASSERT_FLOAT_EQ(result->confidence, 0.7f * 0.6f, 0.001f);
    tensor_tv_destroy(tv1);
    tensor_tv_destroy(tv2);
    tensor_tv_destroy(result);
    TEST_PASS();
}

/* AtomSpace Tests */
void test_atomspace_create(void) {
    TEST_CASE("atomspace creation");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    ASSERT_NOT_NULL(as);
    ASSERT_NOT_NULL(as->atoms);
    ASSERT_EQ(as->natoms, 0);
    ASSERT_EQ(as->maxatoms, 100);
    ASSERT_NOT_NULL(as->atom_embeddings);
    ASSERT_NOT_NULL(as->attention_scores);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atomspace_create_invalid(void) {
    TEST_CASE("atomspace creation with invalid size");
    ASSERT_NULL(tensor_atomspace_create(0));
    ASSERT_NULL(tensor_atomspace_create(-10));
    TEST_PASS();
}

/* Atom Tests */
void test_atom_create(void) {
    TEST_CASE("atom creation");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorTruthValue *tv = tensor_tv_create(0.9f, 0.8f);
    TensorAtom *atom = tensor_atom_create(as, 0, "test_concept", tv);
    ASSERT_NOT_NULL(atom);
    ASSERT_EQ(atom->id, 1);
    ASSERT_EQ(atom->type, 0);
    ASSERT_TRUE(strcmp(atom->name, "test_concept") == 0);
    ASSERT_FLOAT_EQ(atom->tv.strength, 0.9f, 0.001f);
    ASSERT_EQ(as->natoms, 1);
    tensor_tv_destroy(tv);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_create_null_tv(void) {
    TEST_CASE("atom creation with null truth value");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorAtom *atom = tensor_atom_create(as, 0, "default_tv", NULL);
    ASSERT_NOT_NULL(atom);
    ASSERT_FLOAT_EQ(atom->tv.strength, 0.5f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.confidence, 0.1f, 0.001f);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_create_multiple(void) {
    TEST_CASE("multiple atom creation");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorAtom *a1 = tensor_atom_create(as, 0, "atom1", NULL);
    TensorAtom *a2 = tensor_atom_create(as, 1, "atom2", NULL);
    TensorAtom *a3 = tensor_atom_create(as, 2, "atom3", NULL);
    ASSERT_NOT_NULL(a1);
    ASSERT_NOT_NULL(a2);
    ASSERT_NOT_NULL(a3);
    ASSERT_EQ(a1->id, 1);
    ASSERT_EQ(a2->id, 2);
    ASSERT_EQ(a3->id, 3);
    ASSERT_EQ(as->natoms, 3);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_create_overflow(void) {
    TEST_CASE("atom creation overflow");
    TensorAtomSpace *as = tensor_atomspace_create(3);
    tensor_atom_create(as, 0, "a1", NULL);
    tensor_atom_create(as, 0, "a2", NULL);
    tensor_atom_create(as, 0, "a3", NULL);
    TensorAtom *overflow = tensor_atom_create(as, 0, "overflow", NULL);
    ASSERT_NULL(overflow);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_find(void) {
    TEST_CASE("atom find by name");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    tensor_atom_create(as, 0, "apple", NULL);
    tensor_atom_create(as, 0, "banana", NULL);
    tensor_atom_create(as, 0, "cherry", NULL);
    TensorAtom *found = tensor_atom_find(as, "banana");
    ASSERT_NOT_NULL(found);
    ASSERT_TRUE(strcmp(found->name, "banana") == 0);
    TensorAtom *notfound = tensor_atom_find(as, "grape");
    ASSERT_NULL(notfound);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_embedding(void) {
    TEST_CASE("atom embedding initialization");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorAtom *atom = tensor_atom_create(as, 0, "embedded", NULL);
    ASSERT_NOT_NULL(atom);
    float norm = 0.0f;
    for (int i = 0; i < TENSOR_EMBED_DIM; i++) {
        norm += atom->embedding[i] * atom->embedding[i];
    }
    ASSERT_TRUE(norm > 0);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_similarity_identical(void) {
    TEST_CASE("atom similarity identical atoms");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorAtom *a1 = tensor_atom_create(as, 0, "same", NULL);
    float sim = tensor_atom_similarity(a1, a1);
    ASSERT_FLOAT_EQ(sim, 1.0f, 0.001f);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_similarity_different(void) {
    TEST_CASE("atom similarity different atoms");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorAtom *a1 = tensor_atom_create(as, 0, "concept_a", NULL);
    TensorAtom *a2 = tensor_atom_create(as, 0, "concept_b", NULL);
    float sim = tensor_atom_similarity(a1, a2);
    ASSERT_TRUE(sim >= -1.0f && sim <= 1.0f);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_similarity_null(void) {
    TEST_CASE("atom similarity null handling");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorAtom *a1 = tensor_atom_create(as, 0, "test", NULL);
    ASSERT_FLOAT_EQ(tensor_atom_similarity(NULL, a1), 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(tensor_atom_similarity(a1, NULL), 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(tensor_atom_similarity(NULL, NULL), 0.0f, 0.001f);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

/* Gradient Context Tests */
void test_gradient_context_create(void) {
    TEST_CASE("gradient context creation");
    GradientContext *ctx = gradient_context_create(100);
    ASSERT_NOT_NULL(ctx);
    ASSERT_NOT_NULL(ctx->gradients);
    ASSERT_NOT_NULL(ctx->m);
    ASSERT_NOT_NULL(ctx->v);
    ASSERT_EQ(ctx->grad_size, 100);
    ASSERT_FLOAT_EQ(ctx->loss, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(ctx->beta1, 0.9f, 0.001f);
    ASSERT_FLOAT_EQ(ctx->beta2, 0.999f, 0.001f);
    gradient_context_destroy(ctx);
    TEST_PASS();
}

void test_gradient_context_zero(void) {
    TEST_CASE("gradient context zero");
    GradientContext *ctx = gradient_context_create(100);
    ctx->gradients[0] = 1.0f;
    ctx->gradients[50] = 2.0f;
    ctx->loss = 5.0f;
    gradient_context_zero(ctx);
    ASSERT_FLOAT_EQ(ctx->gradients[0], 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(ctx->gradients[50], 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(ctx->loss, 0.0f, 0.001f);
    gradient_context_destroy(ctx);
    TEST_PASS();
}

/* Logic Engine Tests */
void test_logic_engine_create(void) {
    TEST_CASE("logic engine creation");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorLogicEngine *engine = tensor_logic_create(as);
    ASSERT_NOT_NULL(engine);
    ASSERT_EQ(engine->atomspace, as);
    ASSERT_NULL(engine->rules);
    ASSERT_EQ(engine->num_rules, 0);
    ASSERT_NOT_NULL(engine->query_weights);
    ASSERT_NOT_NULL(engine->key_weights);
    ASSERT_NOT_NULL(engine->value_weights);
    ASSERT_NOT_NULL(engine->output_weights);
    ASSERT_NOT_NULL(engine->grad_ctx);
    ASSERT_FLOAT_EQ(engine->temperature, 1.0f, 0.001f);
    tensor_logic_destroy(engine);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_logic_engine_create_null(void) {
    TEST_CASE("logic engine creation with null atomspace");
    ASSERT_NULL(tensor_logic_create(NULL));
    TEST_PASS();
}

void test_logic_engine_weights_initialized(void) {
    TEST_CASE("logic engine weights initialized");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorLogicEngine *engine = tensor_logic_create(as);
    int size = TENSOR_EMBED_DIM * TENSOR_HIDDEN_DIM;
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += engine->query_weights[i] * engine->query_weights[i];
    }
    ASSERT_TRUE(sum > 0);  /* Weights should be non-zero */
    tensor_logic_destroy(engine);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

/* Integration Tests */
void test_full_workflow(void) {
    TEST_CASE("full neural-symbolic workflow");
    TensorAtomSpace *as = tensor_atomspace_create(100);
    TensorLogicEngine *engine = tensor_logic_create(as);

    /* Create atoms */
    TensorTruthValue *tv1 = tensor_tv_create(0.9f, 0.8f);
    TensorTruthValue *tv2 = tensor_tv_create(0.7f, 0.6f);
    TensorAtom *a1 = tensor_atom_create(as, 0, "human", tv1);
    TensorAtom *a2 = tensor_atom_create(as, 0, "mortal", tv2);

    ASSERT_NOT_NULL(a1);
    ASSERT_NOT_NULL(a2);

    /* Check similarity */
    float sim = tensor_atom_similarity(a1, a2);
    ASSERT_TRUE(sim >= -1.0f && sim <= 1.0f);

    /* Test deduction */
    TensorTruthValue *deduced = tensor_tv_deduction(&a1->tv, &a2->tv);
    ASSERT_NOT_NULL(deduced);
    ASSERT_TRUE(deduced->strength <= a1->tv.strength);
    ASSERT_TRUE(deduced->strength <= a2->tv.strength);

    tensor_tv_destroy(tv1);
    tensor_tv_destroy(tv2);
    tensor_tv_destroy(deduced);
    tensor_logic_destroy(engine);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_many_atoms(void) {
    TEST_CASE("stress test many atoms");
    TensorAtomSpace *as = tensor_atomspace_create(1000);
    TensorLogicEngine *engine = tensor_logic_create(as);

    for (int i = 0; i < 500; i++) {
        char name[32];
        sprintf(name, "atom_%d", i);
        TensorAtom *atom = tensor_atom_create(as, i % 5, name, NULL);
        if (i < 100) ASSERT_NOT_NULL(atom);
    }

    ASSERT_EQ(as->natoms, 500);

    tensor_logic_destroy(engine);
    tensor_atomspace_destroy(as);
    TEST_PASS();
}

void test_embedding_diversity(void) {
    TEST_CASE("embedding diversity");
    TensorAtomSpace *as = tensor_atomspace_create(100);

    TensorAtom *atoms[10];
    for (int i = 0; i < 10; i++) {
        char name[32];
        sprintf(name, "diverse_%d", i);
        atoms[i] = tensor_atom_create(as, 0, name, NULL);
    }

    /* Check that different atoms have different embeddings */
    int all_same = 1;
    for (int i = 0; i < 10 && all_same; i++) {
        for (int j = i + 1; j < 10 && all_same; j++) {
            float diff = 0.0f;
            for (int k = 0; k < TENSOR_EMBED_DIM; k++) {
                diff += fabsf(atoms[i]->embedding[k] - atoms[j]->embedding[k]);
            }
            if (diff > 0.001f) all_same = 0;
        }
    }
    ASSERT_FALSE(all_same);

    tensor_atomspace_destroy(as);
    TEST_PASS();
}

/*
 * Main test runner
 */
int main(void) {
    printf("Tensor Logic Unit Tests\n");
    printf("=======================\n");
    printf("Testing neural-symbolic unification for AGI\n");

    srand(42);  /* Fixed seed for reproducibility */

    /* Activation Function Tests */
    TEST_SUITE("Activation Functions");
    test_sigmoid();
    test_tanh();
    test_relu();

    /* Truth Value Tests */
    TEST_SUITE("Truth Value Operations");
    test_tv_create();
    test_tv_create_boundary();
    test_tv_embedding();
    test_tv_merge();
    test_tv_merge_null();
    test_tv_deduction();

    /* AtomSpace Tests */
    TEST_SUITE("AtomSpace Operations");
    test_atomspace_create();
    test_atomspace_create_invalid();

    /* Atom Tests */
    TEST_SUITE("Atom Operations");
    test_atom_create();
    test_atom_create_null_tv();
    test_atom_create_multiple();
    test_atom_create_overflow();
    test_atom_find();
    test_atom_embedding();
    test_atom_similarity_identical();
    test_atom_similarity_different();
    test_atom_similarity_null();

    /* Gradient Context Tests */
    TEST_SUITE("Gradient Context");
    test_gradient_context_create();
    test_gradient_context_zero();

    /* Logic Engine Tests */
    TEST_SUITE("Logic Engine");
    test_logic_engine_create();
    test_logic_engine_create_null();
    test_logic_engine_weights_initialized();

    /* Integration Tests */
    TEST_SUITE("Integration Tests");
    test_full_workflow();
    test_many_atoms();
    test_embedding_diversity();

    /* Print summary */
    printf("\n========================================\n");
    printf("TEST SUMMARY\n");
    printf("========================================\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", pass_count);
    printf("Failed: %d\n", fail_count);
    printf("========================================\n");

    if (fail_count == 0) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
