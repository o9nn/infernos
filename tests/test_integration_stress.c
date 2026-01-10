/*
 * Integration and Stress Tests for OpenCog Kernel AGI
 * Tests system-wide integration and performance under load
 */

#define _GNU_SOURCE  /* For strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

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

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        printf("FAIL: Expected %ld, got %ld\n", (long)(b), (long)(a)); \
        fail_count++; return; \
    }

#define ASSERT_NOT_NULL(ptr) \
    if ((ptr) == NULL) { TEST_FAIL("Unexpected NULL: " #ptr); return; }

#define ASSERT_NULL(ptr) \
    if ((ptr) != NULL) { TEST_FAIL("Expected NULL: " #ptr); return; }

#define ASSERT_FLOAT_EQ(a, b, epsilon) \
    if (fabs((a) - (b)) > (epsilon)) { \
        printf("FAIL: Expected %f, got %f\n", (b), (a)); \
        fail_count++; return; \
    }

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

/* Structures */
typedef struct TruthValue TruthValue;
typedef struct Atom Atom;
typedef struct AtomSpace AtomSpace;
typedef struct Goal Goal;
typedef struct PatternMatcher PatternMatcher;
typedef struct ReasoningEngine ReasoningEngine;
typedef struct CognitiveState CognitiveState;
typedef struct OpenCogKernel OpenCogKernel;
typedef struct Proc Proc;

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
};

struct Goal {
    unsigned long id;
    char *description;
    float urgency;
    float importance;
    TruthValue satisfaction;
    Goal *subgoals;
    Goal *next;
};

struct PatternMatcher {
    Atom **patterns;
    int npatterns;
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
    long long think_time;
    int cognitive_load;
};

struct Proc {
    char text[64];
    unsigned long pid;
    int state;
    int pri;
    Proc *qnext;
    CognitiveState *cognitive;
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

/* Process states */
enum { Dead = 0, Moribund, Ready, Running };

/* Global state */
static OpenCogKernel opencog_kernel;
static Proc *proclist = NULL;
static int next_pid = 1;
static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

/* Timer utility */
double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

/* Forward declarations */
AtomSpace* atomspace_create(int maxatoms);
void atomspace_destroy(AtomSpace *as);
Atom* atom_create(AtomSpace *as, int type, char *name, TruthValue *tv);
void atom_add_link(Atom *atom, Atom *target);
Goal* goal_create(unsigned long id, char *description, float urgency, float importance);
void goal_add(Goal *parent, Goal *subgoal);
PatternMatcher* patternmatcher_create(void);
ReasoningEngine* reasoner_create(AtomSpace *as, PatternMatcher *pm);
CognitiveState* cognitive_create(void);
void cognitive_destroy(CognitiveState *cs);
void reasoning_cycle(ReasoningEngine *re);
float atom_similarity(Atom *a1, Atom *a2);
int atom_unify(Atom *pattern, Atom *target);
float string_similarity(char *s1, char *s2);
Proc* proc_create(char *name);
void proc_cognitive_init(Proc *p);
void proc_cognitive_cleanup(Proc *p);
void cognitive_schedule(void);
void kernel_init(void);
void kernel_reset(void);

/* Implementations */
AtomSpace* atomspace_create(int maxatoms) {
    AtomSpace *as = malloc(sizeof(AtomSpace));
    if (!as) return NULL;
    as->atoms = calloc(maxatoms, sizeof(Atom*));
    if (!as->atoms) { free(as); return NULL; }
    as->natoms = 0;
    as->maxatoms = maxatoms;
    as->next_id = 1;
    as->parent = NULL;
    return as;
}

void atomspace_destroy(AtomSpace *as) {
    if (!as) return;
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
    free(as);
}

Atom* atom_create(AtomSpace *as, int type, char *name, TruthValue *tv) {
    if (!as || !name || as->natoms >= as->maxatoms) return NULL;

    Atom *atom = malloc(sizeof(Atom));
    if (!atom) return NULL;

    atom->id = as->next_id++;
    atom->type = type;
    atom->name = strdup(name);
    if (!atom->name) { free(atom); return NULL; }

    if (tv) atom->tv = *tv;
    else { atom->tv.strength = 0.5f; atom->tv.confidence = 0.1f; atom->tv.count = 1.0f; }

    atom->outgoing = NULL;
    atom->arity = 0;

    unsigned long hash = atom->id % as->maxatoms;
    atom->next = as->atoms[hash];
    as->atoms[hash] = atom;
    as->natoms++;

    opencog_kernel.total_atoms++;

    return atom;
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
    goal->urgency = urgency;
    goal->importance = importance;
    goal->satisfaction.strength = 0.0f;
    goal->satisfaction.confidence = 0.0f;
    goal->satisfaction.count = 0.0f;
    goal->subgoals = NULL;
    goal->next = NULL;
    return goal;
}

void goal_add(Goal *parent, Goal *subgoal) {
    if (!parent || !subgoal) return;
    subgoal->next = parent->subgoals;
    parent->subgoals = subgoal;
}

PatternMatcher* patternmatcher_create(void) {
    PatternMatcher *pm = malloc(sizeof(PatternMatcher));
    if (!pm) return NULL;
    pm->patterns = NULL;
    pm->npatterns = 0;
    return pm;
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

CognitiveState* cognitive_create(void) {
    CognitiveState *cs = malloc(sizeof(CognitiveState));
    if (!cs) return NULL;
    cs->local_space = atomspace_create(256);
    if (!cs->local_space) { free(cs); return NULL; }
    cs->active_goals = NULL;
    cs->reasoner = reasoner_create(cs->local_space, opencog_kernel.pm);
    cs->attention_level = 0.5f;
    cs->motivation = 0.5f;
    cs->think_time = 0;
    cs->cognitive_load = 0;
    opencog_kernel.cognitive_processes++;
    return cs;
}

void cognitive_destroy(CognitiveState *cs) {
    if (!cs) return;
    if (cs->local_space) atomspace_destroy(cs->local_space);
    opencog_kernel.cognitive_processes--;
    free(cs);
}

void reasoning_cycle(ReasoningEngine *re) {
    if (!re) return;
    Goal *goal;
    for (goal = re->goals; goal != NULL; goal = goal->next) {
        if (goal->satisfaction.strength < 0.9f) {
            goal->satisfaction.strength += 0.01f;
            goal->satisfaction.confidence += 0.005f;
        }
    }
    re->last_cycle = time(NULL);
    opencog_kernel.reasoning_cycles++;
}

float string_similarity(char *s1, char *s2) {
    if (!s1 || !s2) return 0.0f;
    int len1 = strlen(s1), len2 = strlen(s2);
    if (len1 == 0 && len2 == 0) return 1.0f;
    if (len1 == 0 || len2 == 0) return 0.0f;
    int common = 0;
    for (int i = 0; i < len1; i++) {
        for (int j = 0; j < len2; j++) {
            if (s1[i] == s2[j]) { common++; break; }
        }
    }
    return (float)(2 * common) / (len1 + len2);
}

float atom_similarity(Atom *a1, Atom *a2) {
    if (!a1 || !a2) return 0.0f;
    if (a1->type != a2->type) return 0.1f;
    if (strcmp(a1->name, a2->name) == 0) return 1.0f;
    return string_similarity(a1->name, a2->name);
}

int atom_unify(Atom *pattern, Atom *target) {
    if (!pattern || !target) return 0;
    if (pattern->type == target->type && strcmp(pattern->name, target->name) == 0) return 1;
    return 0;
}

Proc* proc_create(char *name) {
    Proc *p = malloc(sizeof(Proc));
    if (!p) return NULL;
    strncpy(p->text, name, sizeof(p->text) - 1);
    p->text[sizeof(p->text) - 1] = '\0';
    p->pid = next_pid++;
    p->state = Ready;
    p->pri = 50;
    p->qnext = proclist;
    p->cognitive = NULL;
    proclist = p;
    return p;
}

void proc_cognitive_init(Proc *p) {
    if (!p) return;
    p->cognitive = cognitive_create();
    if (p->cognitive && p->cognitive->reasoner) {
        Goal *survival = goal_create(p->pid * 1000, "process_survival", 1.0f, 1.0f);
        Goal *efficiency = goal_create(p->pid * 1000 + 1, "resource_efficiency", 0.7f, 0.8f);
        p->cognitive->reasoner->goals = survival;
        goal_add(survival, efficiency);
    }
}

void proc_cognitive_cleanup(Proc *p) {
    if (!p || !p->cognitive) return;
    cognitive_destroy(p->cognitive);
    p->cognitive = NULL;
}

void cognitive_schedule(void) {
    Proc *p;
    for (p = proclist; p != NULL; p = p->qnext) {
        if (p->cognitive == NULL) continue;
        CognitiveState *cs = p->cognitive;
        cs->attention_level = (float)p->pri / 100.0f;
        if (cs->attention_level > 1.0f) cs->attention_level = 1.0f;
        if (p->state == Running || p->state == Ready) {
            reasoning_cycle(cs->reasoner);
            cs->think_time += 100;
        }
    }
    reasoning_cycle(opencog_kernel.global_reasoner);
}

void kernel_init(void) {
    memset(&opencog_kernel, 0, sizeof(OpenCogKernel));
    opencog_kernel.global_space = atomspace_create(10000);
    opencog_kernel.pm = patternmatcher_create();
    opencog_kernel.global_reasoner = reasoner_create(opencog_kernel.global_space, opencog_kernel.pm);
    opencog_kernel.system_goals = goal_create(0, "system_survival", 1.0f, 1.0f);
    opencog_kernel.system_attention = 1.0f;
    opencog_kernel.distributed_nodes = 1;
    proclist = NULL;
    next_pid = 1;
}

void kernel_reset(void) {
    while (proclist) {
        Proc *p = proclist;
        proclist = p->qnext;
        proc_cognitive_cleanup(p);
        free(p);
    }
    if (opencog_kernel.global_space) atomspace_destroy(opencog_kernel.global_space);
    memset(&opencog_kernel, 0, sizeof(OpenCogKernel));
}

/*
 * INTEGRATION TESTS
 */

void test_full_system_init(void) {
    TEST_CASE("full system initialization");
    kernel_init();

    ASSERT_NOT_NULL(opencog_kernel.global_space);
    ASSERT_NOT_NULL(opencog_kernel.pm);
    ASSERT_NOT_NULL(opencog_kernel.global_reasoner);
    ASSERT_NOT_NULL(opencog_kernel.system_goals);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);
    ASSERT_EQ(opencog_kernel.total_atoms, 0);

    kernel_reset();
    TEST_PASS();
}

void test_integrated_process_lifecycle(void) {
    TEST_CASE("integrated process lifecycle");
    kernel_init();

    /* Create and initialize process */
    Proc *p = proc_create("integrated_test");
    proc_cognitive_init(p);

    /* Verify full state */
    ASSERT_NOT_NULL(p->cognitive);
    ASSERT_NOT_NULL(p->cognitive->local_space);
    ASSERT_NOT_NULL(p->cognitive->reasoner);
    ASSERT_NOT_NULL(p->cognitive->reasoner->goals);

    /* Run reasoning */
    p->state = Running;
    cognitive_schedule();
    ASSERT_TRUE(p->cognitive->think_time > 0);
    ASSERT_TRUE(opencog_kernel.reasoning_cycles > 0);

    /* Cleanup */
    proc_cognitive_cleanup(p);
    ASSERT_NULL(p->cognitive);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    kernel_reset();
    TEST_PASS();
}

void test_atomspace_with_process(void) {
    TEST_CASE("atomspace integration with process");
    kernel_init();

    Proc *p = proc_create("atomspace_test");
    proc_cognitive_init(p);

    /* Add atoms to global and local atomspace */
    atom_create(opencog_kernel.global_space, CONCEPT_NODE, "global_knowledge", NULL);
    atom_create(p->cognitive->local_space, CONCEPT_NODE, "local_knowledge", NULL);

    ASSERT_EQ(opencog_kernel.global_space->natoms, 1);
    ASSERT_EQ(p->cognitive->local_space->natoms, 1);
    ASSERT_EQ(opencog_kernel.total_atoms, 2);

    kernel_reset();
    TEST_PASS();
}

void test_goal_hierarchy_integration(void) {
    TEST_CASE("goal hierarchy integration");
    kernel_init();

    /* Create process with goals */
    Proc *p = proc_create("goal_test");
    proc_cognitive_init(p);

    /* Verify goal hierarchy */
    Goal *survival = p->cognitive->reasoner->goals;
    ASSERT_NOT_NULL(survival);
    ASSERT_NOT_NULL(survival->subgoals);

    /* Add more goals */
    Goal *custom = goal_create(999, "custom_goal", 0.6f, 0.7f);
    goal_add(survival, custom);

    int count = 0;
    for (Goal *g = survival->subgoals; g; g = g->next) count++;
    ASSERT_EQ(count, 2);

    kernel_reset();
    TEST_PASS();
}

void test_multi_process_reasoning(void) {
    TEST_CASE("multi-process reasoning coordination");
    kernel_init();

    /* Create multiple processes */
    Proc *procs[5];
    for (int i = 0; i < 5; i++) {
        char name[32];
        sprintf(name, "proc_%d", i);
        procs[i] = proc_create(name);
        proc_cognitive_init(procs[i]);
        procs[i]->state = Running;
    }

    ASSERT_EQ(opencog_kernel.cognitive_processes, 5);

    /* Run scheduling */
    long long initial_cycles = opencog_kernel.reasoning_cycles;
    cognitive_schedule();

    /* Each process + global reasoner = 6 cycles */
    ASSERT_EQ(opencog_kernel.reasoning_cycles, initial_cycles + 6);

    kernel_reset();
    TEST_PASS();
}

void test_atomspace_link_creation(void) {
    TEST_CASE("atomspace link creation");
    kernel_init();

    Atom *a1 = atom_create(opencog_kernel.global_space, CONCEPT_NODE, "subject", NULL);
    Atom *a2 = atom_create(opencog_kernel.global_space, CONCEPT_NODE, "object", NULL);
    Atom *link = atom_create(opencog_kernel.global_space, INHERITANCE_LINK, "inherits", NULL);

    atom_add_link(link, a1);
    atom_add_link(link, a2);

    ASSERT_EQ(link->arity, 2);
    ASSERT_EQ(link->outgoing[0], a1);
    ASSERT_EQ(link->outgoing[1], a2);

    kernel_reset();
    TEST_PASS();
}

void test_similarity_computation(void) {
    TEST_CASE("similarity computation integration");
    kernel_init();

    Atom *a1 = atom_create(opencog_kernel.global_space, CONCEPT_NODE, "hello", NULL);
    Atom *a2 = atom_create(opencog_kernel.global_space, CONCEPT_NODE, "hello", NULL);
    Atom *a3 = atom_create(opencog_kernel.global_space, CONCEPT_NODE, "world", NULL);
    Atom *a4 = atom_create(opencog_kernel.global_space, PREDICATE_NODE, "hello", NULL);

    float sim_identical = atom_similarity(a1, a2);
    float sim_different = atom_similarity(a1, a3);
    float sim_type_diff = atom_similarity(a1, a4);

    ASSERT_FLOAT_EQ(sim_identical, 1.0f, 0.001f);
    ASSERT_TRUE(sim_different < 1.0f);
    ASSERT_FLOAT_EQ(sim_type_diff, 0.1f, 0.001f);

    kernel_reset();
    TEST_PASS();
}

/*
 * STRESS TESTS
 */

void test_stress_many_atoms(void) {
    TEST_CASE("stress test: many atoms");
    kernel_init();

    double start = get_time_ms();

    #define NUM_ATOMS 5000
    for (int i = 0; i < NUM_ATOMS; i++) {
        char name[32];
        sprintf(name, "atom_%d", i);
        Atom *a = atom_create(opencog_kernel.global_space, CONCEPT_NODE, name, NULL);
        if (i < 100) ASSERT_NOT_NULL(a);  /* Only check first 100 */
    }

    double elapsed = get_time_ms() - start;
    printf("(%d atoms in %.2f ms) ", NUM_ATOMS, elapsed);

    ASSERT_TRUE(opencog_kernel.total_atoms > 0);

    kernel_reset();
    TEST_PASS();
}

void test_stress_many_processes(void) {
    TEST_CASE("stress test: many processes");
    kernel_init();

    double start = get_time_ms();

    #define NUM_PROCS 100
    Proc *procs[NUM_PROCS];
    for (int i = 0; i < NUM_PROCS; i++) {
        char name[32];
        sprintf(name, "stress_proc_%d", i);
        procs[i] = proc_create(name);
        proc_cognitive_init(procs[i]);
        procs[i]->state = Running;
    }

    double create_time = get_time_ms() - start;
    printf("(created %d procs in %.2f ms, ", NUM_PROCS, create_time);

    ASSERT_EQ(opencog_kernel.cognitive_processes, NUM_PROCS);

    /* Run scheduling */
    start = get_time_ms();
    cognitive_schedule();
    double schedule_time = get_time_ms() - start;
    printf("scheduled in %.2f ms) ", schedule_time);

    kernel_reset();
    TEST_PASS();
}

void test_stress_reasoning_cycles(void) {
    TEST_CASE("stress test: many reasoning cycles");
    kernel_init();

    Proc *p = proc_create("reasoning_stress");
    proc_cognitive_init(p);
    p->state = Running;

    double start = get_time_ms();

    #define NUM_CYCLES 1000
    for (int i = 0; i < NUM_CYCLES; i++) {
        cognitive_schedule();
    }

    double elapsed = get_time_ms() - start;
    printf("(%d cycles in %.2f ms) ", NUM_CYCLES, elapsed);

    ASSERT_TRUE(opencog_kernel.reasoning_cycles >= NUM_CYCLES);

    kernel_reset();
    TEST_PASS();
}

void test_stress_rapid_creation_destruction(void) {
    TEST_CASE("stress test: rapid create/destroy");
    kernel_init();

    double start = get_time_ms();

    #define NUM_ITERATIONS 500
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        Proc *p = proc_create("rapid");
        proc_cognitive_init(p);

        /* Remove from list and destroy */
        proclist = p->qnext;
        proc_cognitive_cleanup(p);
        free(p);
    }

    double elapsed = get_time_ms() - start;
    printf("(%d iterations in %.2f ms) ", NUM_ITERATIONS, elapsed);

    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    kernel_reset();
    TEST_PASS();
}

void test_stress_deep_goal_hierarchy(void) {
    TEST_CASE("stress test: deep goal hierarchy");
    kernel_init();

    Goal *root = goal_create(0, "root", 1.0f, 1.0f);

    double start = get_time_ms();

    #define GOAL_DEPTH 100
    Goal *current = root;
    for (int i = 1; i <= GOAL_DEPTH; i++) {
        char desc[32];
        sprintf(desc, "goal_%d", i);
        Goal *child = goal_create(i, desc, 0.9f, 0.9f);
        goal_add(current, child);
        current = child;
    }

    double elapsed = get_time_ms() - start;
    printf("(%d deep in %.2f ms) ", GOAL_DEPTH, elapsed);

    /* Verify depth */
    int depth = 0;
    current = root;
    while (current->subgoals) {
        depth++;
        current = current->subgoals;
    }
    ASSERT_EQ(depth, GOAL_DEPTH);

    kernel_reset();
    TEST_PASS();
}

void test_stress_link_dense_graph(void) {
    TEST_CASE("stress test: dense link graph");
    kernel_init();

    #define GRAPH_SIZE 50
    Atom *nodes[GRAPH_SIZE];

    /* Create nodes */
    for (int i = 0; i < GRAPH_SIZE; i++) {
        char name[32];
        sprintf(name, "node_%d", i);
        nodes[i] = atom_create(opencog_kernel.global_space, CONCEPT_NODE, name, NULL);
    }

    double start = get_time_ms();

    /* Create links (partial dense graph) */
    int link_count = 0;
    for (int i = 0; i < GRAPH_SIZE; i++) {
        for (int j = i + 1; j < GRAPH_SIZE && j < i + 10; j++) {
            char name[32];
            sprintf(name, "link_%d_%d", i, j);
            Atom *link = atom_create(opencog_kernel.global_space, SIMILARITY_LINK, name, NULL);
            if (link) {
                atom_add_link(link, nodes[i]);
                atom_add_link(link, nodes[j]);
                link_count++;
            }
        }
    }

    double elapsed = get_time_ms() - start;
    printf("(%d links in %.2f ms) ", link_count, elapsed);

    ASSERT_TRUE(link_count > 0);

    kernel_reset();
    TEST_PASS();
}

void test_stress_concurrent_attention(void) {
    TEST_CASE("stress test: attention allocation");
    kernel_init();

    #define ATTENTION_PROCS 50
    Proc *procs[ATTENTION_PROCS];

    for (int i = 0; i < ATTENTION_PROCS; i++) {
        char name[32];
        sprintf(name, "attention_%d", i);
        procs[i] = proc_create(name);
        proc_cognitive_init(procs[i]);
        procs[i]->state = Running;
        procs[i]->pri = rand() % 100;
    }

    double start = get_time_ms();

    /* Multiple scheduling rounds */
    for (int round = 0; round < 100; round++) {
        cognitive_schedule();

        /* Verify attention bounds */
        for (int i = 0; i < ATTENTION_PROCS; i++) {
            if (procs[i]->cognitive) {
                float attention = procs[i]->cognitive->attention_level;
                if (attention < 0.0f || attention > 1.0f) {
                    TEST_FAIL("Attention out of bounds");
                    kernel_reset();
                    return;
                }
            }
        }
    }

    double elapsed = get_time_ms() - start;
    printf("(100 rounds in %.2f ms) ", elapsed);

    kernel_reset();
    TEST_PASS();
}

void test_stress_memory_patterns(void) {
    TEST_CASE("stress test: memory allocation patterns");
    kernel_init();

    double start = get_time_ms();

    /* Interleaved allocation and deallocation */
    #define PATTERN_SIZE 200
    CognitiveState *states[PATTERN_SIZE];

    /* Allocate all */
    for (int i = 0; i < PATTERN_SIZE; i++) {
        states[i] = cognitive_create();
        ASSERT_NOT_NULL(states[i]);
    }

    /* Free odd indices */
    for (int i = 1; i < PATTERN_SIZE; i += 2) {
        cognitive_destroy(states[i]);
        states[i] = NULL;
    }

    /* Reallocate odd indices */
    for (int i = 1; i < PATTERN_SIZE; i += 2) {
        states[i] = cognitive_create();
        ASSERT_NOT_NULL(states[i]);
    }

    /* Free all */
    for (int i = 0; i < PATTERN_SIZE; i++) {
        if (states[i]) {
            cognitive_destroy(states[i]);
        }
    }

    double elapsed = get_time_ms() - start;
    printf("(%.2f ms) ", elapsed);

    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    kernel_reset();
    TEST_PASS();
}

/*
 * PERFORMANCE BENCHMARKS
 */

void test_benchmark_atom_creation(void) {
    TEST_CASE("benchmark: atom creation rate");
    kernel_init();

    double start = get_time_ms();
    int count = 0;

    while (get_time_ms() - start < 1000 && count < 50000) {
        char name[32];
        sprintf(name, "bench_%d", count);
        Atom *a = atom_create(opencog_kernel.global_space, CONCEPT_NODE, name, NULL);
        if (!a) break;
        count++;
    }

    double elapsed = get_time_ms() - start;
    double rate = count / (elapsed / 1000.0);
    printf("(%.0f atoms/sec) ", rate);

    ASSERT_TRUE(rate > 100);  /* At least 100 atoms/sec */

    kernel_reset();
    TEST_PASS();
}

void test_benchmark_reasoning_rate(void) {
    TEST_CASE("benchmark: reasoning cycle rate");
    kernel_init();

    Proc *p = proc_create("bench");
    proc_cognitive_init(p);
    p->state = Running;

    double start = get_time_ms();
    int count = 0;

    while (get_time_ms() - start < 1000 && count < 100000) {
        cognitive_schedule();
        count++;
    }

    double elapsed = get_time_ms() - start;
    double rate = count / (elapsed / 1000.0);
    printf("(%.0f cycles/sec) ", rate);

    ASSERT_TRUE(rate > 100);  /* At least 100 cycles/sec */

    kernel_reset();
    TEST_PASS();
}

void test_benchmark_similarity_computation(void) {
    TEST_CASE("benchmark: similarity computation rate");
    kernel_init();

    Atom *a1 = atom_create(opencog_kernel.global_space, CONCEPT_NODE, "benchmark_atom_one", NULL);
    Atom *a2 = atom_create(opencog_kernel.global_space, CONCEPT_NODE, "benchmark_atom_two", NULL);

    double start = get_time_ms();
    int count = 0;
    float total = 0;

    while (get_time_ms() - start < 1000 && count < 1000000) {
        total += atom_similarity(a1, a2);
        count++;
    }

    double elapsed = get_time_ms() - start;
    double rate = count / (elapsed / 1000.0);
    printf("(%.0f comps/sec) ", rate);

    ASSERT_TRUE(rate > 1000);  /* At least 1000 comparisons/sec */

    kernel_reset();
    TEST_PASS();
}

/*
 * Main test runner
 */
int main(void) {
    printf("Integration and Stress Tests\n");
    printf("============================\n");
    printf("Testing system-wide integration and performance\n");

    srand(time(NULL));

    /* Integration Tests */
    TEST_SUITE("Integration Tests");
    test_full_system_init();
    test_integrated_process_lifecycle();
    test_atomspace_with_process();
    test_goal_hierarchy_integration();
    test_multi_process_reasoning();
    test_atomspace_link_creation();
    test_similarity_computation();

    /* Stress Tests */
    TEST_SUITE("Stress Tests");
    test_stress_many_atoms();
    test_stress_many_processes();
    test_stress_reasoning_cycles();
    test_stress_rapid_creation_destruction();
    test_stress_deep_goal_hierarchy();
    test_stress_link_dense_graph();
    test_stress_concurrent_attention();
    test_stress_memory_patterns();

    /* Performance Benchmarks */
    TEST_SUITE("Performance Benchmarks");
    test_benchmark_atom_creation();
    test_benchmark_reasoning_rate();
    test_benchmark_similarity_computation();

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
