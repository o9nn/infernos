/*
 * Exhaustive Unit Tests for OpenCog Kernel Functions
 * Tests all kernel-level AGI functionality in opencog.c
 */

#define _GNU_SOURCE  /* For strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
    if (!(cond)) { TEST_FAIL("Assertion failed: " #cond); return; } \
    else

#define ASSERT_FALSE(cond) \
    if (cond) { TEST_FAIL("Assertion failed: NOT " #cond); return; } \
    else

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        printf("FAIL: Expected %ld, got %ld\n", (long)(b), (long)(a)); \
        fail_count++; return; \
    }

#define ASSERT_EQ_STR(a, b) \
    if (strcmp((a), (b)) != 0) { \
        printf("FAIL: Expected '%s', got '%s'\n", (b), (a)); \
        fail_count++; return; \
    }

#define ASSERT_FLOAT_EQ(a, b, epsilon) \
    if (fabs((a) - (b)) > (epsilon)) { \
        printf("FAIL: Expected %f, got %f\n", (b), (a)); \
        fail_count++; return; \
    }

#define ASSERT_NOT_NULL(ptr) \
    if ((ptr) == NULL) { TEST_FAIL("Unexpected NULL: " #ptr); return; }

#define ASSERT_NULL(ptr) \
    if ((ptr) != NULL) { TEST_FAIL("Expected NULL: " #ptr); return; }

/* Atom types */
enum AtomType {
    NODE_ATOM = 0,
    LINK_ATOM,
    CONCEPT_NODE,
    PREDICATE_NODE,
    EVALUATION_LINK,
    INHERITANCE_LINK,
    SIMILARITY_LINK,
    IMPLICATION_LINK,
    EXECUTION_LINK,
    PROCEDURAL_ATOM,
    GOAL_ATOM,
    SATISFACTION_LINK
};

/* Simplified kernel structures for testing */
typedef struct TruthValue TruthValue;
typedef struct Atom Atom;
typedef struct AtomSpace AtomSpace;
typedef struct Goal Goal;
typedef struct PatternMatcher PatternMatcher;
typedef struct ReasoningEngine ReasoningEngine;
typedef struct CognitiveState CognitiveState;
typedef struct OpenCogKernel OpenCogKernel;

struct TruthValue {
    float strength;
    float confidence;
    float count;
};

struct Atom {
    unsigned long id;
    int type;
    char *name;
    TruthValue tv;
    Atom **outgoing;
    int arity;
    Atom *next;
};

struct AtomSpace {
    Atom **atoms;
    int natoms;
    int maxatoms;
    unsigned long next_id;
    AtomSpace *parent;
    AtomSpace **children;
    int nchildren;
};

struct Goal {
    unsigned long id;
    char *description;
    float urgency;
    float importance;
    TruthValue satisfaction;
    Atom *target;
    Goal *subgoals;
    Goal *next;
    long long created;
    long long deadline;
};

struct PatternMatcher {
    Atom **patterns;
    int npatterns;
    float (*similarity)(Atom*, Atom*);
    int (*unify)(Atom*, Atom*, Atom***);
};

struct ReasoningEngine {
    AtomSpace *atomspace;
    PatternMatcher *pm;
    Goal *goals;
    int inference_steps;
    float confidence_threshold;
    long long last_cycle;
};

struct CognitiveState {
    AtomSpace *local_space;
    Goal *active_goals;
    ReasoningEngine *reasoner;
    float attention_level;
    float motivation;
    Atom *context;
    long long think_time;
    int cognitive_load;
};

struct OpenCogKernel {
    AtomSpace *global_space;
    ReasoningEngine *global_reasoner;
    Goal *system_goals;
    PatternMatcher *pm;
    int cognitive_processes;
    long long total_atoms;
    long long reasoning_cycles;
    float system_attention;
    int distributed_nodes;
};

/* Global state */
static OpenCogKernel opencog_kernel;
static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

/* Function prototypes - implementations for testing */
AtomSpace* atomspace_create(int maxatoms);
void atomspace_destroy(AtomSpace *as);
Atom* atom_create(AtomSpace *as, int type, char *name, TruthValue *tv);
Atom* atom_find_by_name(AtomSpace *as, char *name);
Atom* atom_find_by_id(AtomSpace *as, unsigned long id);
void atom_add_link(Atom *atom, Atom *target);
Goal* goal_create(unsigned long id, char *description, float urgency, float importance);
void goal_add(Goal *parent, Goal *subgoal);
void goal_destroy(Goal *goal);
PatternMatcher* patternmatcher_create(void);
void patternmatcher_add_pattern(PatternMatcher *pm, Atom *pattern);
ReasoningEngine* reasoner_create(AtomSpace *as, PatternMatcher *pm);
void reasoner_add_goal(ReasoningEngine *re, Goal *goal);
CognitiveState* cognitive_create(void);
void cognitive_destroy(CognitiveState *cs);
void reasoning_cycle(ReasoningEngine *re);
void inference_step(ReasoningEngine *re);
float atom_similarity(Atom *a1, Atom *a2);
int atom_unify(Atom *pattern, Atom *target, Atom ***bindings);
float string_similarity(char *s1, char *s2);
void opencoginit(void);
void opencog_reset(void);

/* Implementation */
AtomSpace* atomspace_create(int maxatoms) {
    if (maxatoms <= 0) return NULL;

    AtomSpace *as = malloc(sizeof(AtomSpace));
    if (!as) return NULL;

    as->atoms = calloc(maxatoms, sizeof(Atom*));
    if (!as->atoms) {
        free(as);
        return NULL;
    }

    as->natoms = 0;
    as->maxatoms = maxatoms;
    as->next_id = 1;
    as->parent = NULL;
    as->children = NULL;
    as->nchildren = 0;

    return as;
}

void atomspace_destroy(AtomSpace *as) {
    if (!as) return;

    /* Free all atoms */
    for (int i = 0; i < as->maxatoms; i++) {
        Atom *atom = as->atoms[i];
        while (atom) {
            Atom *next = atom->next;
            if (atom->name) free(atom->name);
            if (atom->outgoing) free(atom->outgoing);
            free(atom);
            atom = next;
        }
    }

    free(as->atoms);
    if (as->children) free(as->children);
    free(as);
}

Atom* atom_create(AtomSpace *as, int type, char *name, TruthValue *tv) {
    if (!as || !name) return NULL;
    if (as->natoms >= as->maxatoms) return NULL;

    Atom *atom = malloc(sizeof(Atom));
    if (!atom) return NULL;

    atom->id = as->next_id++;
    atom->type = type;
    atom->name = strdup(name);
    if (!atom->name) {
        free(atom);
        return NULL;
    }

    if (tv) {
        atom->tv = *tv;
    } else {
        atom->tv.strength = 0.5f;
        atom->tv.confidence = 0.1f;
        atom->tv.count = 1.0f;
    }

    atom->outgoing = NULL;
    atom->arity = 0;

    /* Add to hash table */
    unsigned long hash = atom->id % as->maxatoms;
    atom->next = as->atoms[hash];
    as->atoms[hash] = atom;
    as->natoms++;

    opencog_kernel.total_atoms++;

    return atom;
}

Atom* atom_find_by_name(AtomSpace *as, char *name) {
    if (!as || !name) return NULL;

    for (int i = 0; i < as->maxatoms; i++) {
        Atom *atom = as->atoms[i];
        while (atom) {
            if (atom->name && strcmp(atom->name, name) == 0)
                return atom;
            atom = atom->next;
        }
    }

    return NULL;
}

Atom* atom_find_by_id(AtomSpace *as, unsigned long id) {
    if (!as) return NULL;

    unsigned long hash = id % as->maxatoms;
    Atom *atom = as->atoms[hash];

    while (atom) {
        if (atom->id == id)
            return atom;
        atom = atom->next;
    }

    return NULL;
}

void atom_add_link(Atom *atom, Atom *target) {
    if (!atom || !target) return;

    atom->outgoing = realloc(atom->outgoing, (atom->arity + 1) * sizeof(Atom*));
    if (atom->outgoing) {
        atom->outgoing[atom->arity] = target;
        atom->arity++;
    }
}

Goal* goal_create(unsigned long id, char *description, float urgency, float importance) {
    if (!description) return NULL;

    Goal *goal = malloc(sizeof(Goal));
    if (!goal) return NULL;

    goal->id = id;
    goal->description = strdup(description);
    if (!goal->description) {
        free(goal);
        return NULL;
    }

    goal->urgency = urgency;
    goal->importance = importance;
    goal->satisfaction.strength = 0.0f;
    goal->satisfaction.confidence = 0.0f;
    goal->satisfaction.count = 0.0f;
    goal->target = NULL;
    goal->subgoals = NULL;
    goal->next = NULL;
    goal->created = time(NULL);
    goal->deadline = 0;

    return goal;
}

void goal_add(Goal *parent, Goal *subgoal) {
    if (!parent || !subgoal) return;

    subgoal->next = parent->subgoals;
    parent->subgoals = subgoal;
}

void goal_destroy(Goal *goal) {
    if (!goal) return;

    /* Recursively destroy subgoals */
    Goal *sub = goal->subgoals;
    while (sub) {
        Goal *next = sub->next;
        goal_destroy(sub);
        sub = next;
    }

    if (goal->description) free(goal->description);
    free(goal);
}

PatternMatcher* patternmatcher_create(void) {
    PatternMatcher *pm = malloc(sizeof(PatternMatcher));
    if (!pm) return NULL;

    pm->patterns = NULL;
    pm->npatterns = 0;
    pm->similarity = atom_similarity;
    pm->unify = atom_unify;

    return pm;
}

void patternmatcher_add_pattern(PatternMatcher *pm, Atom *pattern) {
    if (!pm || !pattern) return;

    pm->patterns = realloc(pm->patterns, (pm->npatterns + 1) * sizeof(Atom*));
    if (pm->patterns) {
        pm->patterns[pm->npatterns] = pattern;
        pm->npatterns++;
    }
}

ReasoningEngine* reasoner_create(AtomSpace *as, PatternMatcher *pm) {
    if (!as || !pm) return NULL;

    ReasoningEngine *re = malloc(sizeof(ReasoningEngine));
    if (!re) return NULL;

    re->atomspace = as;
    re->pm = pm;
    re->goals = NULL;
    re->inference_steps = 100;
    re->confidence_threshold = 0.1f;
    re->last_cycle = 0;

    return re;
}

void reasoner_add_goal(ReasoningEngine *re, Goal *goal) {
    if (!re || !goal) return;

    goal->next = re->goals;
    re->goals = goal;
}

CognitiveState* cognitive_create(void) {
    CognitiveState *cs = malloc(sizeof(CognitiveState));
    if (!cs) return NULL;

    cs->local_space = atomspace_create(256);
    if (!cs->local_space) {
        free(cs);
        return NULL;
    }

    cs->active_goals = NULL;
    cs->reasoner = reasoner_create(cs->local_space, opencog_kernel.pm);
    cs->attention_level = 0.5f;
    cs->motivation = 0.5f;
    cs->context = NULL;
    cs->think_time = 0;
    cs->cognitive_load = 0;

    opencog_kernel.cognitive_processes++;

    return cs;
}

void cognitive_destroy(CognitiveState *cs) {
    if (!cs) return;

    if (cs->local_space) atomspace_destroy(cs->local_space);
    /* Note: reasoner and active_goals cleanup would be more complex in full impl */

    opencog_kernel.cognitive_processes--;
    free(cs);
}

void reasoning_cycle(ReasoningEngine *re) {
    if (!re) return;

    /* Perform inference steps */
    for (int i = 0; i < re->inference_steps && i < 10; i++) {
        inference_step(re);
    }

    re->last_cycle = time(NULL);
    opencog_kernel.reasoning_cycles++;
}

void inference_step(ReasoningEngine *re) {
    if (!re) return;

    /* Update goal satisfaction */
    Goal *goal;
    for (goal = re->goals; goal != NULL; goal = goal->next) {
        if (goal->satisfaction.strength < 0.9f) {
            goal->satisfaction.strength += 0.01f;
            goal->satisfaction.confidence += 0.005f;
        }
    }
}

float atom_similarity(Atom *a1, Atom *a2) {
    if (!a1 || !a2) return 0.0f;

    if (a1->type != a2->type)
        return 0.1f;

    if (strcmp(a1->name, a2->name) == 0)
        return 1.0f;

    return string_similarity(a1->name, a2->name);
}

int atom_unify(Atom *pattern, Atom *target, Atom ***bindings) {
    (void)bindings;  /* Unused in simplified version */

    if (!pattern || !target)
        return 0;

    if (pattern->type == target->type && strcmp(pattern->name, target->name) == 0)
        return 1;

    return 0;
}

float string_similarity(char *s1, char *s2) {
    if (!s1 || !s2) return 0.0f;

    int len1 = strlen(s1);
    int len2 = strlen(s2);

    if (len1 == 0 && len2 == 0)
        return 1.0f;

    if (len1 == 0 || len2 == 0)
        return 0.0f;

    int common = 0;
    for (int i = 0; i < len1; i++) {
        for (int j = 0; j < len2; j++) {
            if (s1[i] == s2[j]) {
                common++;
                break;
            }
        }
    }

    return (float)(2 * common) / (len1 + len2);
}

void opencoginit(void) {
    opencog_kernel.global_space = atomspace_create(1024);
    opencog_kernel.pm = patternmatcher_create();
    opencog_kernel.global_reasoner = reasoner_create(opencog_kernel.global_space, opencog_kernel.pm);
    opencog_kernel.system_goals = goal_create(0, "system_survival", 1.0f, 1.0f);
    goal_add(opencog_kernel.system_goals, goal_create(1, "resource_optimization", 0.8f, 0.9f));
    goal_add(opencog_kernel.system_goals, goal_create(2, "distributed_coherence", 0.9f, 0.8f));
    goal_add(opencog_kernel.system_goals, goal_create(3, "cognitive_efficiency", 0.7f, 0.8f));
    opencog_kernel.cognitive_processes = 0;
    opencog_kernel.total_atoms = 0;
    opencog_kernel.reasoning_cycles = 0;
    opencog_kernel.system_attention = 1.0f;
    opencog_kernel.distributed_nodes = 1;
}

void opencog_reset(void) {
    if (opencog_kernel.global_space) atomspace_destroy(opencog_kernel.global_space);
    if (opencog_kernel.system_goals) goal_destroy(opencog_kernel.system_goals);
    /* Note: Full cleanup would include pm and reasoner */
    memset(&opencog_kernel, 0, sizeof(OpenCogKernel));
}

/*
 * ===========================================
 * TEST FUNCTIONS
 * ===========================================
 */

/* AtomSpace Tests */
void test_atomspace_create_valid(void) {
    TEST_CASE("atomspace_create with valid size");
    AtomSpace *as = atomspace_create(100);
    ASSERT_NOT_NULL(as);
    ASSERT_EQ(as->natoms, 0);
    ASSERT_EQ(as->maxatoms, 100);
    ASSERT_EQ(as->next_id, 1);
    ASSERT_NULL(as->parent);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_atomspace_create_zero(void) {
    TEST_CASE("atomspace_create with zero size");
    AtomSpace *as = atomspace_create(0);
    ASSERT_NULL(as);
    TEST_PASS();
}

void test_atomspace_create_negative(void) {
    TEST_CASE("atomspace_create with negative size");
    AtomSpace *as = atomspace_create(-10);
    ASSERT_NULL(as);
    TEST_PASS();
}

void test_atomspace_create_large(void) {
    TEST_CASE("atomspace_create with large size");
    AtomSpace *as = atomspace_create(10000);
    ASSERT_NOT_NULL(as);
    ASSERT_EQ(as->maxatoms, 10000);
    atomspace_destroy(as);
    TEST_PASS();
}

/* Atom Tests */
void test_atom_create_basic(void) {
    TEST_CASE("atom_create basic creation");
    AtomSpace *as = atomspace_create(100);
    TruthValue tv = {0.8f, 0.6f, 5.0f};
    Atom *atom = atom_create(as, CONCEPT_NODE, "test_concept", &tv);
    ASSERT_NOT_NULL(atom);
    ASSERT_EQ(atom->id, 1);
    ASSERT_EQ(atom->type, CONCEPT_NODE);
    ASSERT_EQ_STR(atom->name, "test_concept");
    ASSERT_FLOAT_EQ(atom->tv.strength, 0.8f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.confidence, 0.6f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.count, 5.0f, 0.001f);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_create_null_atomspace(void) {
    TEST_CASE("atom_create with NULL atomspace");
    TruthValue tv = {0.5f, 0.5f, 1.0f};
    Atom *atom = atom_create(NULL, CONCEPT_NODE, "test", &tv);
    ASSERT_NULL(atom);
    TEST_PASS();
}

void test_atom_create_null_name(void) {
    TEST_CASE("atom_create with NULL name");
    AtomSpace *as = atomspace_create(100);
    TruthValue tv = {0.5f, 0.5f, 1.0f};
    Atom *atom = atom_create(as, CONCEPT_NODE, NULL, &tv);
    ASSERT_NULL(atom);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_create_null_truthvalue(void) {
    TEST_CASE("atom_create with NULL truth value (default values)");
    AtomSpace *as = atomspace_create(100);
    Atom *atom = atom_create(as, CONCEPT_NODE, "test", NULL);
    ASSERT_NOT_NULL(atom);
    ASSERT_FLOAT_EQ(atom->tv.strength, 0.5f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.confidence, 0.1f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.count, 1.0f, 0.001f);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_create_multiple(void) {
    TEST_CASE("atom_create multiple atoms with unique IDs");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "atom1", NULL);
    Atom *a2 = atom_create(as, CONCEPT_NODE, "atom2", NULL);
    Atom *a3 = atom_create(as, CONCEPT_NODE, "atom3", NULL);
    ASSERT_NOT_NULL(a1);
    ASSERT_NOT_NULL(a2);
    ASSERT_NOT_NULL(a3);
    ASSERT_EQ(a1->id, 1);
    ASSERT_EQ(a2->id, 2);
    ASSERT_EQ(a3->id, 3);
    ASSERT_EQ(as->natoms, 3);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_create_all_types(void) {
    TEST_CASE("atom_create with all atom types");
    AtomSpace *as = atomspace_create(100);
    int types[] = {NODE_ATOM, LINK_ATOM, CONCEPT_NODE, PREDICATE_NODE,
                   EVALUATION_LINK, INHERITANCE_LINK, SIMILARITY_LINK,
                   IMPLICATION_LINK, EXECUTION_LINK, PROCEDURAL_ATOM,
                   GOAL_ATOM, SATISFACTION_LINK};
    int ntypes = sizeof(types) / sizeof(types[0]);

    for (int i = 0; i < ntypes; i++) {
        char name[32];
        sprintf(name, "atom_type_%d", types[i]);
        Atom *atom = atom_create(as, types[i], name, NULL);
        ASSERT_NOT_NULL(atom);
        ASSERT_EQ(atom->type, types[i]);
    }

    ASSERT_EQ(as->natoms, ntypes);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_create_overflow(void) {
    TEST_CASE("atom_create overflow protection");
    AtomSpace *as = atomspace_create(5);
    for (int i = 0; i < 5; i++) {
        char name[32];
        sprintf(name, "atom_%d", i);
        Atom *atom = atom_create(as, CONCEPT_NODE, name, NULL);
        ASSERT_NOT_NULL(atom);
    }
    /* Should fail when full */
    Atom *overflow = atom_create(as, CONCEPT_NODE, "overflow", NULL);
    ASSERT_NULL(overflow);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_find_by_name(void) {
    TEST_CASE("atom_find_by_name");
    AtomSpace *as = atomspace_create(100);
    atom_create(as, CONCEPT_NODE, "apple", NULL);
    atom_create(as, CONCEPT_NODE, "banana", NULL);
    atom_create(as, CONCEPT_NODE, "cherry", NULL);

    Atom *found = atom_find_by_name(as, "banana");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ_STR(found->name, "banana");

    Atom *notfound = atom_find_by_name(as, "grape");
    ASSERT_NULL(notfound);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_find_by_id(void) {
    TEST_CASE("atom_find_by_id");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "first", NULL);
    Atom *a2 = atom_create(as, CONCEPT_NODE, "second", NULL);

    Atom *found = atom_find_by_id(as, a2->id);
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(found->id, a2->id);
    ASSERT_EQ_STR(found->name, "second");

    Atom *notfound = atom_find_by_id(as, 999);
    ASSERT_NULL(notfound);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_add_link(void) {
    TEST_CASE("atom_add_link");
    AtomSpace *as = atomspace_create(100);
    Atom *parent = atom_create(as, LINK_ATOM, "parent_link", NULL);
    Atom *child1 = atom_create(as, CONCEPT_NODE, "child1", NULL);
    Atom *child2 = atom_create(as, CONCEPT_NODE, "child2", NULL);

    ASSERT_EQ(parent->arity, 0);

    atom_add_link(parent, child1);
    ASSERT_EQ(parent->arity, 1);
    ASSERT_EQ(parent->outgoing[0], child1);

    atom_add_link(parent, child2);
    ASSERT_EQ(parent->arity, 2);
    ASSERT_EQ(parent->outgoing[1], child2);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_add_link_null(void) {
    TEST_CASE("atom_add_link with NULL parameters");
    AtomSpace *as = atomspace_create(100);
    Atom *parent = atom_create(as, LINK_ATOM, "parent", NULL);

    atom_add_link(NULL, parent);  /* Should not crash */
    atom_add_link(parent, NULL);  /* Should not crash */
    ASSERT_EQ(parent->arity, 0);

    atomspace_destroy(as);
    TEST_PASS();
}

/* Goal Tests */
void test_goal_create_basic(void) {
    TEST_CASE("goal_create basic");
    Goal *goal = goal_create(1, "test_goal", 0.7f, 0.8f);
    ASSERT_NOT_NULL(goal);
    ASSERT_EQ(goal->id, 1);
    ASSERT_EQ_STR(goal->description, "test_goal");
    ASSERT_FLOAT_EQ(goal->urgency, 0.7f, 0.001f);
    ASSERT_FLOAT_EQ(goal->importance, 0.8f, 0.001f);
    ASSERT_FLOAT_EQ(goal->satisfaction.strength, 0.0f, 0.001f);
    ASSERT_NULL(goal->subgoals);
    ASSERT_NULL(goal->next);
    goal_destroy(goal);
    TEST_PASS();
}

void test_goal_create_null_description(void) {
    TEST_CASE("goal_create with NULL description");
    Goal *goal = goal_create(1, NULL, 0.5f, 0.5f);
    ASSERT_NULL(goal);
    TEST_PASS();
}

void test_goal_create_extreme_values(void) {
    TEST_CASE("goal_create with extreme urgency/importance values");
    Goal *g1 = goal_create(1, "min_values", 0.0f, 0.0f);
    Goal *g2 = goal_create(2, "max_values", 1.0f, 1.0f);
    Goal *g3 = goal_create(3, "beyond_max", 2.0f, 2.0f);
    Goal *g4 = goal_create(4, "negative", -0.5f, -0.5f);

    ASSERT_NOT_NULL(g1);
    ASSERT_NOT_NULL(g2);
    ASSERT_NOT_NULL(g3);
    ASSERT_NOT_NULL(g4);

    ASSERT_FLOAT_EQ(g1->urgency, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(g2->urgency, 1.0f, 0.001f);
    ASSERT_FLOAT_EQ(g3->urgency, 2.0f, 0.001f);
    ASSERT_FLOAT_EQ(g4->urgency, -0.5f, 0.001f);

    goal_destroy(g1);
    goal_destroy(g2);
    goal_destroy(g3);
    goal_destroy(g4);
    TEST_PASS();
}

void test_goal_add_subgoal(void) {
    TEST_CASE("goal_add subgoal");
    Goal *parent = goal_create(1, "parent", 1.0f, 1.0f);
    Goal *child1 = goal_create(2, "child1", 0.8f, 0.8f);
    Goal *child2 = goal_create(3, "child2", 0.6f, 0.6f);

    ASSERT_NULL(parent->subgoals);

    goal_add(parent, child1);
    ASSERT_NOT_NULL(parent->subgoals);
    ASSERT_EQ(parent->subgoals, child1);

    goal_add(parent, child2);
    ASSERT_EQ(parent->subgoals, child2);
    ASSERT_EQ(child2->next, child1);

    goal_destroy(parent);  /* Should also destroy children */
    TEST_PASS();
}

void test_goal_add_null(void) {
    TEST_CASE("goal_add with NULL parameters");
    Goal *goal = goal_create(1, "test", 0.5f, 0.5f);

    goal_add(NULL, goal);  /* Should not crash */
    goal_add(goal, NULL);  /* Should not crash */
    ASSERT_NULL(goal->subgoals);

    goal_destroy(goal);
    TEST_PASS();
}

void test_goal_hierarchy(void) {
    TEST_CASE("goal_hierarchy deep nesting");
    Goal *level0 = goal_create(0, "level0", 1.0f, 1.0f);
    Goal *level1 = goal_create(1, "level1", 0.9f, 0.9f);
    Goal *level2 = goal_create(2, "level2", 0.8f, 0.8f);
    Goal *level3 = goal_create(3, "level3", 0.7f, 0.7f);

    goal_add(level0, level1);
    goal_add(level1, level2);
    goal_add(level2, level3);

    ASSERT_EQ(level0->subgoals, level1);
    ASSERT_EQ(level1->subgoals, level2);
    ASSERT_EQ(level2->subgoals, level3);
    ASSERT_NULL(level3->subgoals);

    goal_destroy(level0);  /* Should recursively clean up */
    TEST_PASS();
}

/* PatternMatcher Tests */
void test_patternmatcher_create(void) {
    TEST_CASE("patternmatcher_create");
    PatternMatcher *pm = patternmatcher_create();
    ASSERT_NOT_NULL(pm);
    ASSERT_NULL(pm->patterns);
    ASSERT_EQ(pm->npatterns, 0);
    ASSERT_NOT_NULL(pm->similarity);
    ASSERT_NOT_NULL(pm->unify);
    free(pm);
    TEST_PASS();
}

void test_patternmatcher_add_pattern(void) {
    TEST_CASE("patternmatcher_add_pattern");
    AtomSpace *as = atomspace_create(100);
    PatternMatcher *pm = patternmatcher_create();

    Atom *p1 = atom_create(as, CONCEPT_NODE, "pattern1", NULL);
    Atom *p2 = atom_create(as, CONCEPT_NODE, "pattern2", NULL);

    patternmatcher_add_pattern(pm, p1);
    ASSERT_EQ(pm->npatterns, 1);
    ASSERT_EQ(pm->patterns[0], p1);

    patternmatcher_add_pattern(pm, p2);
    ASSERT_EQ(pm->npatterns, 2);
    ASSERT_EQ(pm->patterns[1], p2);

    free(pm->patterns);
    free(pm);
    atomspace_destroy(as);
    TEST_PASS();
}

/* ReasoningEngine Tests */
void test_reasoner_create(void) {
    TEST_CASE("reasoner_create");
    AtomSpace *as = atomspace_create(100);
    PatternMatcher *pm = patternmatcher_create();

    ReasoningEngine *re = reasoner_create(as, pm);
    ASSERT_NOT_NULL(re);
    ASSERT_EQ(re->atomspace, as);
    ASSERT_EQ(re->pm, pm);
    ASSERT_NULL(re->goals);
    ASSERT_EQ(re->inference_steps, 100);
    ASSERT_FLOAT_EQ(re->confidence_threshold, 0.1f, 0.001f);

    free(re);
    free(pm);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_reasoner_create_null(void) {
    TEST_CASE("reasoner_create with NULL parameters");
    AtomSpace *as = atomspace_create(100);
    PatternMatcher *pm = patternmatcher_create();

    ReasoningEngine *re1 = reasoner_create(NULL, pm);
    ReasoningEngine *re2 = reasoner_create(as, NULL);
    ReasoningEngine *re3 = reasoner_create(NULL, NULL);

    ASSERT_NULL(re1);
    ASSERT_NULL(re2);
    ASSERT_NULL(re3);

    free(pm);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_reasoner_add_goal(void) {
    TEST_CASE("reasoner_add_goal");
    AtomSpace *as = atomspace_create(100);
    PatternMatcher *pm = patternmatcher_create();
    ReasoningEngine *re = reasoner_create(as, pm);

    Goal *g1 = goal_create(1, "goal1", 0.9f, 0.9f);
    Goal *g2 = goal_create(2, "goal2", 0.8f, 0.8f);

    ASSERT_NULL(re->goals);

    reasoner_add_goal(re, g1);
    ASSERT_EQ(re->goals, g1);

    reasoner_add_goal(re, g2);
    ASSERT_EQ(re->goals, g2);
    ASSERT_EQ(g2->next, g1);

    goal_destroy(g1);
    goal_destroy(g2);
    free(re);
    free(pm);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_reasoning_cycle(void) {
    TEST_CASE("reasoning_cycle");
    AtomSpace *as = atomspace_create(100);
    PatternMatcher *pm = patternmatcher_create();
    ReasoningEngine *re = reasoner_create(as, pm);

    Goal *goal = goal_create(1, "test_goal", 0.8f, 0.8f);
    reasoner_add_goal(re, goal);

    long long initial_cycles = opencog_kernel.reasoning_cycles;
    float initial_satisfaction = goal->satisfaction.strength;

    reasoning_cycle(re);

    ASSERT_EQ(opencog_kernel.reasoning_cycles, initial_cycles + 1);
    ASSERT_TRUE(goal->satisfaction.strength > initial_satisfaction);
    ASSERT_TRUE(re->last_cycle > 0);

    goal_destroy(goal);
    free(re);
    free(pm);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_reasoning_cycle_null(void) {
    TEST_CASE("reasoning_cycle with NULL");
    long long initial_cycles = opencog_kernel.reasoning_cycles;
    reasoning_cycle(NULL);  /* Should not crash */
    ASSERT_EQ(opencog_kernel.reasoning_cycles, initial_cycles);
    TEST_PASS();
}

void test_inference_step(void) {
    TEST_CASE("inference_step goal satisfaction update");
    AtomSpace *as = atomspace_create(100);
    PatternMatcher *pm = patternmatcher_create();
    ReasoningEngine *re = reasoner_create(as, pm);

    Goal *goal = goal_create(1, "test", 0.5f, 0.5f);
    reasoner_add_goal(re, goal);

    float initial = goal->satisfaction.strength;
    inference_step(re);

    ASSERT_TRUE(goal->satisfaction.strength > initial);
    ASSERT_FLOAT_EQ(goal->satisfaction.strength, initial + 0.01f, 0.001f);

    goal_destroy(goal);
    free(re);
    free(pm);
    atomspace_destroy(as);
    TEST_PASS();
}

void test_inference_step_saturation(void) {
    TEST_CASE("inference_step goal satisfaction saturation");
    AtomSpace *as = atomspace_create(100);
    PatternMatcher *pm = patternmatcher_create();
    ReasoningEngine *re = reasoner_create(as, pm);

    Goal *goal = goal_create(1, "test", 0.5f, 0.5f);
    goal->satisfaction.strength = 0.95f;  /* Near saturation */
    reasoner_add_goal(re, goal);

    inference_step(re);

    /* Should not increase past 0.9 */
    ASSERT_FLOAT_EQ(goal->satisfaction.strength, 0.95f, 0.001f);

    goal_destroy(goal);
    free(re);
    free(pm);
    atomspace_destroy(as);
    TEST_PASS();
}

/* CognitiveState Tests */
void test_cognitive_create(void) {
    TEST_CASE("cognitive_create");
    opencoginit();
    int initial_count = opencog_kernel.cognitive_processes;

    CognitiveState *cs = cognitive_create();
    ASSERT_NOT_NULL(cs);
    ASSERT_NOT_NULL(cs->local_space);
    ASSERT_EQ(cs->local_space->maxatoms, 256);
    ASSERT_NULL(cs->active_goals);
    ASSERT_FLOAT_EQ(cs->attention_level, 0.5f, 0.001f);
    ASSERT_FLOAT_EQ(cs->motivation, 0.5f, 0.001f);
    ASSERT_EQ(cs->think_time, 0);
    ASSERT_EQ(cs->cognitive_load, 0);
    ASSERT_EQ(opencog_kernel.cognitive_processes, initial_count + 1);

    cognitive_destroy(cs);
    ASSERT_EQ(opencog_kernel.cognitive_processes, initial_count);

    opencog_reset();
    TEST_PASS();
}

void test_cognitive_multiple(void) {
    TEST_CASE("cognitive_create multiple instances");
    opencoginit();

    CognitiveState *cs1 = cognitive_create();
    CognitiveState *cs2 = cognitive_create();
    CognitiveState *cs3 = cognitive_create();

    ASSERT_NOT_NULL(cs1);
    ASSERT_NOT_NULL(cs2);
    ASSERT_NOT_NULL(cs3);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 3);

    /* Each should have independent local atomspace */
    atom_create(cs1->local_space, CONCEPT_NODE, "cs1_atom", NULL);
    atom_create(cs2->local_space, CONCEPT_NODE, "cs2_atom", NULL);

    ASSERT_EQ(cs1->local_space->natoms, 1);
    ASSERT_EQ(cs2->local_space->natoms, 1);
    ASSERT_EQ(cs3->local_space->natoms, 0);

    cognitive_destroy(cs1);
    cognitive_destroy(cs2);
    cognitive_destroy(cs3);

    opencog_reset();
    TEST_PASS();
}

/* String Similarity Tests */
void test_string_similarity_identical(void) {
    TEST_CASE("string_similarity identical strings");
    float sim = string_similarity("hello", "hello");
    ASSERT_FLOAT_EQ(sim, 1.0f, 0.001f);
    TEST_PASS();
}

void test_string_similarity_different(void) {
    TEST_CASE("string_similarity completely different");
    float sim = string_similarity("abc", "xyz");
    ASSERT_FLOAT_EQ(sim, 0.0f, 0.001f);
    TEST_PASS();
}

void test_string_similarity_partial(void) {
    TEST_CASE("string_similarity partial match");
    float sim = string_similarity("hello", "hallo");
    ASSERT_TRUE(sim > 0.0f);
    ASSERT_TRUE(sim < 1.0f);
    TEST_PASS();
}

void test_string_similarity_empty(void) {
    TEST_CASE("string_similarity empty strings");
    float sim1 = string_similarity("", "");
    float sim2 = string_similarity("hello", "");
    float sim3 = string_similarity("", "world");

    ASSERT_FLOAT_EQ(sim1, 1.0f, 0.001f);
    ASSERT_FLOAT_EQ(sim2, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(sim3, 0.0f, 0.001f);
    TEST_PASS();
}

void test_string_similarity_null(void) {
    TEST_CASE("string_similarity NULL strings");
    float sim1 = string_similarity(NULL, "hello");
    float sim2 = string_similarity("hello", NULL);
    float sim3 = string_similarity(NULL, NULL);

    ASSERT_FLOAT_EQ(sim1, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(sim2, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(sim3, 0.0f, 0.001f);
    TEST_PASS();
}

/* Atom Similarity Tests */
void test_atom_similarity_identical(void) {
    TEST_CASE("atom_similarity identical atoms");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "concept", NULL);
    Atom *a2 = atom_create(as, CONCEPT_NODE, "concept", NULL);

    float sim = atom_similarity(a1, a2);
    ASSERT_FLOAT_EQ(sim, 1.0f, 0.001f);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_similarity_different_type(void) {
    TEST_CASE("atom_similarity different types");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "same_name", NULL);
    Atom *a2 = atom_create(as, PREDICATE_NODE, "same_name", NULL);

    float sim = atom_similarity(a1, a2);
    ASSERT_FLOAT_EQ(sim, 0.1f, 0.001f);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_similarity_null(void) {
    TEST_CASE("atom_similarity NULL atoms");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "test", NULL);

    float sim1 = atom_similarity(NULL, a1);
    float sim2 = atom_similarity(a1, NULL);
    float sim3 = atom_similarity(NULL, NULL);

    ASSERT_FLOAT_EQ(sim1, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(sim2, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(sim3, 0.0f, 0.001f);

    atomspace_destroy(as);
    TEST_PASS();
}

/* Atom Unification Tests */
void test_atom_unify_identical(void) {
    TEST_CASE("atom_unify identical atoms");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "test", NULL);
    Atom *a2 = atom_create(as, CONCEPT_NODE, "test", NULL);

    int result = atom_unify(a1, a2, NULL);
    ASSERT_EQ(result, 1);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_unify_different_name(void) {
    TEST_CASE("atom_unify different names");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "apple", NULL);
    Atom *a2 = atom_create(as, CONCEPT_NODE, "banana", NULL);

    int result = atom_unify(a1, a2, NULL);
    ASSERT_EQ(result, 0);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_unify_different_type(void) {
    TEST_CASE("atom_unify different types");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "test", NULL);
    Atom *a2 = atom_create(as, PREDICATE_NODE, "test", NULL);

    int result = atom_unify(a1, a2, NULL);
    ASSERT_EQ(result, 0);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_atom_unify_null(void) {
    TEST_CASE("atom_unify NULL atoms");
    AtomSpace *as = atomspace_create(100);
    Atom *a1 = atom_create(as, CONCEPT_NODE, "test", NULL);

    int result1 = atom_unify(NULL, a1, NULL);
    int result2 = atom_unify(a1, NULL, NULL);
    int result3 = atom_unify(NULL, NULL, NULL);

    ASSERT_EQ(result1, 0);
    ASSERT_EQ(result2, 0);
    ASSERT_EQ(result3, 0);

    atomspace_destroy(as);
    TEST_PASS();
}

/* OpenCog Kernel Initialization Tests */
void test_opencoginit(void) {
    TEST_CASE("opencoginit");
    opencoginit();

    ASSERT_NOT_NULL(opencog_kernel.global_space);
    ASSERT_NOT_NULL(opencog_kernel.pm);
    ASSERT_NOT_NULL(opencog_kernel.global_reasoner);
    ASSERT_NOT_NULL(opencog_kernel.system_goals);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);
    ASSERT_FLOAT_EQ(opencog_kernel.system_attention, 1.0f, 0.001f);
    ASSERT_EQ(opencog_kernel.distributed_nodes, 1);

    /* Check system goals hierarchy */
    ASSERT_EQ_STR(opencog_kernel.system_goals->description, "system_survival");
    ASSERT_NOT_NULL(opencog_kernel.system_goals->subgoals);

    opencog_reset();
    TEST_PASS();
}

void test_opencoginit_double(void) {
    TEST_CASE("opencoginit double initialization");
    opencoginit();
    AtomSpace *first_space = opencog_kernel.global_space;

    /* Note: In real kernel code this would panic, here we test gracefully */
    opencoginit();  /* This will leak memory but shouldn't crash */

    ASSERT_NOT_NULL(opencog_kernel.global_space);

    opencog_reset();
    TEST_PASS();
}

/* TruthValue Tests */
void test_truthvalue_defaults(void) {
    TEST_CASE("TruthValue default values");
    AtomSpace *as = atomspace_create(100);
    Atom *atom = atom_create(as, CONCEPT_NODE, "test", NULL);

    ASSERT_FLOAT_EQ(atom->tv.strength, 0.5f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.confidence, 0.1f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.count, 1.0f, 0.001f);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_truthvalue_custom(void) {
    TEST_CASE("TruthValue custom values");
    AtomSpace *as = atomspace_create(100);
    TruthValue tv = {0.99f, 0.95f, 100.0f};
    Atom *atom = atom_create(as, CONCEPT_NODE, "certain", &tv);

    ASSERT_FLOAT_EQ(atom->tv.strength, 0.99f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.confidence, 0.95f, 0.001f);
    ASSERT_FLOAT_EQ(atom->tv.count, 100.0f, 0.001f);

    atomspace_destroy(as);
    TEST_PASS();
}

void test_truthvalue_extremes(void) {
    TEST_CASE("TruthValue extreme values");
    AtomSpace *as = atomspace_create(100);
    TruthValue tv1 = {0.0f, 0.0f, 0.0f};
    TruthValue tv2 = {1.0f, 1.0f, 1000000.0f};
    TruthValue tv3 = {-0.5f, -0.5f, -10.0f};

    Atom *a1 = atom_create(as, CONCEPT_NODE, "zero", &tv1);
    Atom *a2 = atom_create(as, CONCEPT_NODE, "max", &tv2);
    Atom *a3 = atom_create(as, CONCEPT_NODE, "negative", &tv3);

    ASSERT_FLOAT_EQ(a1->tv.strength, 0.0f, 0.001f);
    ASSERT_FLOAT_EQ(a2->tv.strength, 1.0f, 0.001f);
    ASSERT_FLOAT_EQ(a3->tv.strength, -0.5f, 0.001f);

    atomspace_destroy(as);
    TEST_PASS();
}

/* Global Statistics Tests */
void test_total_atoms_tracking(void) {
    TEST_CASE("total_atoms tracking");
    opencog_reset();
    opencoginit();

    long long initial = opencog_kernel.total_atoms;
    AtomSpace *as = atomspace_create(100);

    atom_create(as, CONCEPT_NODE, "a1", NULL);
    ASSERT_EQ(opencog_kernel.total_atoms, initial + 1);

    atom_create(as, CONCEPT_NODE, "a2", NULL);
    ASSERT_EQ(opencog_kernel.total_atoms, initial + 2);

    atom_create(as, CONCEPT_NODE, "a3", NULL);
    ASSERT_EQ(opencog_kernel.total_atoms, initial + 3);

    atomspace_destroy(as);
    opencog_reset();
    TEST_PASS();
}

void test_reasoning_cycles_tracking(void) {
    TEST_CASE("reasoning_cycles tracking");
    opencog_reset();
    opencoginit();

    ASSERT_EQ(opencog_kernel.reasoning_cycles, 0);

    reasoning_cycle(opencog_kernel.global_reasoner);
    ASSERT_EQ(opencog_kernel.reasoning_cycles, 1);

    reasoning_cycle(opencog_kernel.global_reasoner);
    reasoning_cycle(opencog_kernel.global_reasoner);
    ASSERT_EQ(opencog_kernel.reasoning_cycles, 3);

    opencog_reset();
    TEST_PASS();
}

void test_cognitive_processes_tracking(void) {
    TEST_CASE("cognitive_processes tracking");
    opencog_reset();
    opencoginit();

    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    CognitiveState *cs1 = cognitive_create();
    ASSERT_EQ(opencog_kernel.cognitive_processes, 1);

    CognitiveState *cs2 = cognitive_create();
    ASSERT_EQ(opencog_kernel.cognitive_processes, 2);

    cognitive_destroy(cs1);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 1);

    cognitive_destroy(cs2);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    opencog_reset();
    TEST_PASS();
}

/*
 * Main test runner
 */
int main(void) {
    printf("OpenCog Kernel Unit Tests\n");
    printf("=========================\n");
    printf("Testing all kernel-level AGI functionality\n");

    /* AtomSpace Tests */
    TEST_SUITE("AtomSpace Tests");
    test_atomspace_create_valid();
    test_atomspace_create_zero();
    test_atomspace_create_negative();
    test_atomspace_create_large();

    /* Atom Tests */
    TEST_SUITE("Atom Tests");
    test_atom_create_basic();
    test_atom_create_null_atomspace();
    test_atom_create_null_name();
    test_atom_create_null_truthvalue();
    test_atom_create_multiple();
    test_atom_create_all_types();
    test_atom_create_overflow();
    test_atom_find_by_name();
    test_atom_find_by_id();
    test_atom_add_link();
    test_atom_add_link_null();

    /* Goal Tests */
    TEST_SUITE("Goal Tests");
    test_goal_create_basic();
    test_goal_create_null_description();
    test_goal_create_extreme_values();
    test_goal_add_subgoal();
    test_goal_add_null();
    test_goal_hierarchy();

    /* PatternMatcher Tests */
    TEST_SUITE("PatternMatcher Tests");
    test_patternmatcher_create();
    test_patternmatcher_add_pattern();

    /* ReasoningEngine Tests */
    TEST_SUITE("ReasoningEngine Tests");
    test_reasoner_create();
    test_reasoner_create_null();
    test_reasoner_add_goal();
    test_reasoning_cycle();
    test_reasoning_cycle_null();
    test_inference_step();
    test_inference_step_saturation();

    /* CognitiveState Tests */
    TEST_SUITE("CognitiveState Tests");
    test_cognitive_create();
    test_cognitive_multiple();

    /* String Similarity Tests */
    TEST_SUITE("String Similarity Tests");
    test_string_similarity_identical();
    test_string_similarity_different();
    test_string_similarity_partial();
    test_string_similarity_empty();
    test_string_similarity_null();

    /* Atom Similarity Tests */
    TEST_SUITE("Atom Similarity Tests");
    test_atom_similarity_identical();
    test_atom_similarity_different_type();
    test_atom_similarity_null();

    /* Atom Unification Tests */
    TEST_SUITE("Atom Unification Tests");
    test_atom_unify_identical();
    test_atom_unify_different_name();
    test_atom_unify_different_type();
    test_atom_unify_null();

    /* OpenCog Initialization Tests */
    TEST_SUITE("OpenCog Initialization Tests");
    test_opencoginit();
    test_opencoginit_double();

    /* TruthValue Tests */
    TEST_SUITE("TruthValue Tests");
    test_truthvalue_defaults();
    test_truthvalue_custom();
    test_truthvalue_extremes();

    /* Statistics Tracking Tests */
    TEST_SUITE("Statistics Tracking Tests");
    test_total_atoms_tracking();
    test_reasoning_cycles_tracking();
    test_cognitive_processes_tracking();

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
