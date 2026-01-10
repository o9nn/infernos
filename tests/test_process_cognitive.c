/*
 * Exhaustive Unit Tests for Process Cognitive Integration
 * Tests cognitive state management in OS processes
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

/* Process states */
enum {
    Dead = 0,
    Moribund,
    Ready,
    Scheding,
    Running,
    Queueing,
    Wakeme,
    Broken,
    Stopped,
    Rendezvous,
    Waitrelease,
};

/* Simplified structures */
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
    Atom *next;
};

struct AtomSpace {
    Atom **atoms;
    int natoms;
    int maxatoms;
    unsigned long next_id;
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
    Atom *context;
    long long think_time;
    int cognitive_load;
};

/* Simulated Process structure matching Inferno's Proc */
struct Proc {
    char text[64];
    unsigned long pid;
    int state;
    int pri;
    Proc *rnext;
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

/* Global state */
static OpenCogKernel opencog_kernel;
static Proc *proclist = NULL;
static int next_pid = 1;
static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

/* Helper functions */
AtomSpace* atomspace_create(int maxatoms) {
    AtomSpace *as = malloc(sizeof(AtomSpace));
    if (!as) return NULL;
    as->atoms = calloc(maxatoms, sizeof(Atom*));
    if (!as->atoms) { free(as); return NULL; }
    as->natoms = 0;
    as->maxatoms = maxatoms;
    as->next_id = 1;
    return as;
}

void atomspace_destroy(AtomSpace *as) {
    if (!as) return;
    for (int i = 0; i < as->maxatoms; i++) {
        Atom *atom = as->atoms[i];
        while (atom) {
            Atom *next = atom->next;
            if (atom->name) free(atom->name);
            free(atom);
            atom = next;
        }
    }
    free(as->atoms);
    free(as);
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

/* Cognitive state functions for processes */
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
    opencog_kernel.cognitive_processes--;
    free(cs);
}

/* Process cognitive initialization - as in proc.c newproc() */
void proc_cognitive_init(Proc *p) {
    if (!p) return;

    p->cognitive = cognitive_create();

    /* Add basic goals for the process */
    if (p->cognitive && p->cognitive->reasoner) {
        Goal *survival = goal_create(p->pid * 1000, "process_survival", 1.0f, 1.0f);
        Goal *efficiency = goal_create(p->pid * 1000 + 1, "resource_efficiency", 0.7f, 0.8f);

        p->cognitive->reasoner->goals = survival;
        goal_add(survival, efficiency);
    }
}

/* Process cognitive cleanup - as in proc.c pexit() */
void proc_cognitive_cleanup(Proc *p) {
    if (!p || !p->cognitive) return;

    cognitive_destroy(p->cognitive);
    p->cognitive = NULL;
}

/* Reasoning cycle for process */
void reasoning_cycle(ReasoningEngine *re) {
    if (!re) return;

    /* Update goal satisfaction */
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

/* Cognitive scheduler integration */
void cognitive_schedule(void) {
    Proc *p;

    for (p = proclist; p != NULL; p = p->qnext) {
        if (p->cognitive == NULL)
            continue;

        CognitiveState *cs = p->cognitive;

        /* Update attention based on process priority */
        cs->attention_level = (float)p->pri / 100.0f;
        if (cs->attention_level > 1.0f)
            cs->attention_level = 1.0f;
        if (cs->attention_level < 0.0f)
            cs->attention_level = 0.0f;

        /* Run cognitive processing if process is active */
        if (p->state == Running || p->state == Ready) {
            reasoning_cycle(cs->reasoner);
            cs->think_time += 100;  /* Simulated time increment */
        }
    }

    /* Update global reasoning state */
    reasoning_cycle(opencog_kernel.global_reasoner);
}

/* Process management helpers */
Proc* proc_create(char *name) {
    Proc *p = malloc(sizeof(Proc));
    if (!p) return NULL;

    strncpy(p->text, name, sizeof(p->text) - 1);
    p->text[sizeof(p->text) - 1] = '\0';
    p->pid = next_pid++;
    p->state = Ready;
    p->pri = 50;  /* Default priority */
    p->rnext = NULL;
    p->qnext = NULL;
    p->cognitive = NULL;

    /* Add to process list */
    p->qnext = proclist;
    proclist = p;

    return p;
}

void proc_destroy(Proc *p) {
    if (!p) return;

    /* Remove from process list */
    if (proclist == p) {
        proclist = p->qnext;
    } else {
        Proc *prev = proclist;
        while (prev && prev->qnext != p)
            prev = prev->qnext;
        if (prev)
            prev->qnext = p->qnext;
    }

    proc_cognitive_cleanup(p);
    free(p);
}

void kernel_init(void) {
    memset(&opencog_kernel, 0, sizeof(OpenCogKernel));
    opencog_kernel.global_space = atomspace_create(1024);
    opencog_kernel.pm = patternmatcher_create();
    opencog_kernel.global_reasoner = reasoner_create(opencog_kernel.global_space, opencog_kernel.pm);
    opencog_kernel.system_goals = goal_create(0, "system_survival", 1.0f, 1.0f);
    opencog_kernel.system_attention = 1.0f;
    opencog_kernel.distributed_nodes = 1;
    proclist = NULL;
    next_pid = 1;
}

void kernel_reset(void) {
    /* Cleanup all processes */
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
 * TEST FUNCTIONS
 */

/* Process Creation Tests */
void test_proc_create_basic(void) {
    TEST_CASE("proc_create basic");
    kernel_init();

    Proc *p = proc_create("test_proc");
    ASSERT_NOT_NULL(p);
    ASSERT_EQ(p->pid, 1);
    ASSERT_EQ(p->state, Ready);
    ASSERT_EQ(p->pri, 50);
    ASSERT_NULL(p->cognitive);

    kernel_reset();
    TEST_PASS();
}

void test_proc_create_multiple(void) {
    TEST_CASE("proc_create multiple processes");
    kernel_init();

    Proc *p1 = proc_create("proc1");
    Proc *p2 = proc_create("proc2");
    Proc *p3 = proc_create("proc3");

    ASSERT_NOT_NULL(p1);
    ASSERT_NOT_NULL(p2);
    ASSERT_NOT_NULL(p3);
    ASSERT_EQ(p1->pid, 1);
    ASSERT_EQ(p2->pid, 2);
    ASSERT_EQ(p3->pid, 3);

    kernel_reset();
    TEST_PASS();
}

/* Process Cognitive Init Tests */
void test_proc_cognitive_init_basic(void) {
    TEST_CASE("proc_cognitive_init basic");
    kernel_init();

    Proc *p = proc_create("cognitive_proc");
    proc_cognitive_init(p);

    ASSERT_NOT_NULL(p->cognitive);
    ASSERT_NOT_NULL(p->cognitive->local_space);
    ASSERT_NOT_NULL(p->cognitive->reasoner);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 1);

    kernel_reset();
    TEST_PASS();
}

void test_proc_cognitive_init_goals(void) {
    TEST_CASE("proc_cognitive_init creates goals");
    kernel_init();

    Proc *p = proc_create("goal_proc");
    proc_cognitive_init(p);

    ASSERT_NOT_NULL(p->cognitive->reasoner->goals);
    ASSERT_TRUE(strcmp(p->cognitive->reasoner->goals->description, "process_survival") == 0);
    ASSERT_NOT_NULL(p->cognitive->reasoner->goals->subgoals);

    kernel_reset();
    TEST_PASS();
}

void test_proc_cognitive_init_null(void) {
    TEST_CASE("proc_cognitive_init NULL process");
    kernel_init();

    proc_cognitive_init(NULL);  /* Should not crash */
    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    kernel_reset();
    TEST_PASS();
}

void test_proc_cognitive_init_multiple(void) {
    TEST_CASE("proc_cognitive_init multiple processes");
    kernel_init();

    Proc *p1 = proc_create("proc1");
    Proc *p2 = proc_create("proc2");
    Proc *p3 = proc_create("proc3");

    proc_cognitive_init(p1);
    proc_cognitive_init(p2);
    proc_cognitive_init(p3);

    ASSERT_EQ(opencog_kernel.cognitive_processes, 3);

    /* Each should have independent state */
    ASSERT_TRUE(p1->cognitive != p2->cognitive);
    ASSERT_TRUE(p2->cognitive != p3->cognitive);
    ASSERT_TRUE(p1->cognitive->local_space != p2->cognitive->local_space);

    kernel_reset();
    TEST_PASS();
}

/* Process Cognitive Cleanup Tests */
void test_proc_cognitive_cleanup_basic(void) {
    TEST_CASE("proc_cognitive_cleanup basic");
    kernel_init();

    Proc *p = proc_create("cleanup_proc");
    proc_cognitive_init(p);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 1);

    proc_cognitive_cleanup(p);
    ASSERT_NULL(p->cognitive);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    kernel_reset();
    TEST_PASS();
}

void test_proc_cognitive_cleanup_null(void) {
    TEST_CASE("proc_cognitive_cleanup NULL");
    kernel_init();

    proc_cognitive_cleanup(NULL);  /* Should not crash */

    Proc *p = proc_create("test");
    proc_cognitive_cleanup(p);  /* No cognitive state */

    kernel_reset();
    TEST_PASS();
}

void test_proc_cognitive_cleanup_sequential(void) {
    TEST_CASE("proc_cognitive_cleanup sequential");
    kernel_init();

    Proc *p1 = proc_create("proc1");
    Proc *p2 = proc_create("proc2");
    Proc *p3 = proc_create("proc3");

    proc_cognitive_init(p1);
    proc_cognitive_init(p2);
    proc_cognitive_init(p3);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 3);

    proc_cognitive_cleanup(p1);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 2);

    proc_cognitive_cleanup(p2);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 1);

    proc_cognitive_cleanup(p3);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    kernel_reset();
    TEST_PASS();
}

/* Cognitive State Tests */
void test_cognitive_state_defaults(void) {
    TEST_CASE("cognitive state default values");
    kernel_init();

    Proc *p = proc_create("default_proc");
    proc_cognitive_init(p);

    ASSERT_FLOAT_EQ(p->cognitive->attention_level, 0.5f, 0.001f);
    ASSERT_FLOAT_EQ(p->cognitive->motivation, 0.5f, 0.001f);
    ASSERT_EQ(p->cognitive->think_time, 0);
    ASSERT_EQ(p->cognitive->cognitive_load, 0);
    ASSERT_NULL(p->cognitive->context);

    kernel_reset();
    TEST_PASS();
}

void test_cognitive_state_modification(void) {
    TEST_CASE("cognitive state modification");
    kernel_init();

    Proc *p = proc_create("mod_proc");
    proc_cognitive_init(p);

    p->cognitive->attention_level = 0.9f;
    p->cognitive->motivation = 0.8f;
    p->cognitive->think_time = 1000;
    p->cognitive->cognitive_load = 75;

    ASSERT_FLOAT_EQ(p->cognitive->attention_level, 0.9f, 0.001f);
    ASSERT_FLOAT_EQ(p->cognitive->motivation, 0.8f, 0.001f);
    ASSERT_EQ(p->cognitive->think_time, 1000);
    ASSERT_EQ(p->cognitive->cognitive_load, 75);

    kernel_reset();
    TEST_PASS();
}

/* Cognitive Scheduling Tests */
void test_cognitive_schedule_empty(void) {
    TEST_CASE("cognitive_schedule empty process list");
    kernel_init();

    long long initial_cycles = opencog_kernel.reasoning_cycles;
    cognitive_schedule();

    /* Should still run global reasoner */
    ASSERT_EQ(opencog_kernel.reasoning_cycles, initial_cycles + 1);

    kernel_reset();
    TEST_PASS();
}

void test_cognitive_schedule_single_process(void) {
    TEST_CASE("cognitive_schedule single process");
    kernel_init();

    Proc *p = proc_create("single");
    proc_cognitive_init(p);
    p->state = Running;

    long long initial_cycles = opencog_kernel.reasoning_cycles;
    long long initial_think = p->cognitive->think_time;

    cognitive_schedule();

    /* Process reasoning + global reasoning */
    ASSERT_EQ(opencog_kernel.reasoning_cycles, initial_cycles + 2);
    ASSERT_TRUE(p->cognitive->think_time > initial_think);

    kernel_reset();
    TEST_PASS();
}

void test_cognitive_schedule_multiple_processes(void) {
    TEST_CASE("cognitive_schedule multiple processes");
    kernel_init();

    Proc *p1 = proc_create("proc1");
    Proc *p2 = proc_create("proc2");
    Proc *p3 = proc_create("proc3");

    proc_cognitive_init(p1);
    proc_cognitive_init(p2);
    proc_cognitive_init(p3);

    p1->state = Running;
    p2->state = Ready;
    p3->state = Dead;  /* Should be skipped */

    long long initial_cycles = opencog_kernel.reasoning_cycles;

    cognitive_schedule();

    /* p1 (running) + p2 (ready) + global = 3 cycles */
    ASSERT_EQ(opencog_kernel.reasoning_cycles, initial_cycles + 3);

    kernel_reset();
    TEST_PASS();
}

void test_cognitive_schedule_priority_attention(void) {
    TEST_CASE("cognitive_schedule priority affects attention");
    kernel_init();

    Proc *p1 = proc_create("high_pri");
    Proc *p2 = proc_create("low_pri");

    proc_cognitive_init(p1);
    proc_cognitive_init(p2);

    p1->state = Running;
    p2->state = Running;
    p1->pri = 90;
    p2->pri = 20;

    cognitive_schedule();

    ASSERT_FLOAT_EQ(p1->cognitive->attention_level, 0.9f, 0.001f);
    ASSERT_FLOAT_EQ(p2->cognitive->attention_level, 0.2f, 0.001f);

    kernel_reset();
    TEST_PASS();
}

void test_cognitive_schedule_attention_bounds(void) {
    TEST_CASE("cognitive_schedule attention bounds");
    kernel_init();

    Proc *p1 = proc_create("very_high");
    Proc *p2 = proc_create("negative");

    proc_cognitive_init(p1);
    proc_cognitive_init(p2);

    p1->state = Running;
    p2->state = Running;
    p1->pri = 150;  /* Over 100 */
    p2->pri = -10;  /* Negative */

    cognitive_schedule();

    ASSERT_TRUE(p1->cognitive->attention_level <= 1.0f);
    ASSERT_TRUE(p2->cognitive->attention_level >= 0.0f);

    kernel_reset();
    TEST_PASS();
}

void test_cognitive_schedule_no_cognitive(void) {
    TEST_CASE("cognitive_schedule process without cognitive state");
    kernel_init();

    Proc *p1 = proc_create("cognitive");
    Proc *p2 = proc_create("no_cognitive");

    proc_cognitive_init(p1);
    /* p2 has no cognitive state */

    p1->state = Running;
    p2->state = Running;

    cognitive_schedule();  /* Should not crash */

    ASSERT_NOT_NULL(p1->cognitive);
    ASSERT_NULL(p2->cognitive);

    kernel_reset();
    TEST_PASS();
}

/* Goal Management Tests */
void test_process_goals_survival(void) {
    TEST_CASE("process survival goal");
    kernel_init();

    Proc *p = proc_create("survival_proc");
    proc_cognitive_init(p);

    Goal *survival = p->cognitive->reasoner->goals;
    ASSERT_NOT_NULL(survival);
    ASSERT_FLOAT_EQ(survival->urgency, 1.0f, 0.001f);
    ASSERT_FLOAT_EQ(survival->importance, 1.0f, 0.001f);

    kernel_reset();
    TEST_PASS();
}

void test_process_goals_efficiency(void) {
    TEST_CASE("process efficiency subgoal");
    kernel_init();

    Proc *p = proc_create("efficient_proc");
    proc_cognitive_init(p);

    Goal *survival = p->cognitive->reasoner->goals;
    Goal *efficiency = survival->subgoals;

    ASSERT_NOT_NULL(efficiency);
    ASSERT_TRUE(strcmp(efficiency->description, "resource_efficiency") == 0);
    ASSERT_FLOAT_EQ(efficiency->urgency, 0.7f, 0.001f);
    ASSERT_FLOAT_EQ(efficiency->importance, 0.8f, 0.001f);

    kernel_reset();
    TEST_PASS();
}

void test_process_goals_unique_ids(void) {
    TEST_CASE("process goals have unique IDs");
    kernel_init();

    Proc *p1 = proc_create("proc1");
    Proc *p2 = proc_create("proc2");

    proc_cognitive_init(p1);
    proc_cognitive_init(p2);

    Goal *g1 = p1->cognitive->reasoner->goals;
    Goal *g2 = p2->cognitive->reasoner->goals;

    ASSERT_TRUE(g1->id != g2->id);

    kernel_reset();
    TEST_PASS();
}

/* Local AtomSpace Tests */
void test_process_local_atomspace(void) {
    TEST_CASE("process local atomspace");
    kernel_init();

    Proc *p = proc_create("atomspace_proc");
    proc_cognitive_init(p);

    ASSERT_NOT_NULL(p->cognitive->local_space);
    ASSERT_EQ(p->cognitive->local_space->maxatoms, 256);
    ASSERT_EQ(p->cognitive->local_space->natoms, 0);

    kernel_reset();
    TEST_PASS();
}

void test_process_atomspace_isolation(void) {
    TEST_CASE("process atomspace isolation");
    kernel_init();

    Proc *p1 = proc_create("proc1");
    Proc *p2 = proc_create("proc2");

    proc_cognitive_init(p1);
    proc_cognitive_init(p2);

    /* Add atom to p1's atomspace */
    Atom *atom = malloc(sizeof(Atom));
    atom->id = p1->cognitive->local_space->next_id++;
    atom->name = strdup("test_atom");
    atom->next = NULL;
    p1->cognitive->local_space->atoms[0] = atom;
    p1->cognitive->local_space->natoms++;

    ASSERT_EQ(p1->cognitive->local_space->natoms, 1);
    ASSERT_EQ(p2->cognitive->local_space->natoms, 0);

    kernel_reset();
    TEST_PASS();
}

/* Think Time Tests */
void test_think_time_accumulation(void) {
    TEST_CASE("think time accumulation");
    kernel_init();

    Proc *p = proc_create("thinking");
    proc_cognitive_init(p);
    p->state = Running;

    ASSERT_EQ(p->cognitive->think_time, 0);

    cognitive_schedule();
    long long after_one = p->cognitive->think_time;
    ASSERT_TRUE(after_one > 0);

    cognitive_schedule();
    ASSERT_TRUE(p->cognitive->think_time > after_one);

    kernel_reset();
    TEST_PASS();
}

void test_think_time_only_active(void) {
    TEST_CASE("think time only for active processes");
    kernel_init();

    Proc *p = proc_create("inactive");
    proc_cognitive_init(p);
    p->state = Dead;

    cognitive_schedule();
    ASSERT_EQ(p->cognitive->think_time, 0);

    p->state = Stopped;
    cognitive_schedule();
    ASSERT_EQ(p->cognitive->think_time, 0);

    kernel_reset();
    TEST_PASS();
}

/* Reasoning Cycle Tests */
void test_reasoning_cycle_goal_satisfaction(void) {
    TEST_CASE("reasoning cycle updates goal satisfaction");
    kernel_init();

    Proc *p = proc_create("reasoning");
    proc_cognitive_init(p);
    p->state = Running;

    float initial_satisfaction = p->cognitive->reasoner->goals->satisfaction.strength;

    cognitive_schedule();

    ASSERT_TRUE(p->cognitive->reasoner->goals->satisfaction.strength > initial_satisfaction);

    kernel_reset();
    TEST_PASS();
}

void test_reasoning_cycle_timestamp(void) {
    TEST_CASE("reasoning cycle updates timestamp");
    kernel_init();

    Proc *p = proc_create("timestamp");
    proc_cognitive_init(p);
    p->state = Running;

    ASSERT_EQ(p->cognitive->reasoner->last_cycle, 0);

    cognitive_schedule();

    ASSERT_TRUE(p->cognitive->reasoner->last_cycle > 0);

    kernel_reset();
    TEST_PASS();
}

/* Process Lifecycle Tests */
void test_process_lifecycle(void) {
    TEST_CASE("full process lifecycle");
    kernel_init();

    /* Create process (newproc equivalent) */
    Proc *p = proc_create("lifecycle");
    ASSERT_NOT_NULL(p);
    ASSERT_NULL(p->cognitive);

    /* Initialize cognitive state */
    proc_cognitive_init(p);
    ASSERT_NOT_NULL(p->cognitive);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 1);

    /* Run process */
    p->state = Running;
    cognitive_schedule();
    ASSERT_TRUE(p->cognitive->think_time > 0);

    /* Exit process (pexit equivalent) */
    proc_cognitive_cleanup(p);
    ASSERT_NULL(p->cognitive);
    ASSERT_EQ(opencog_kernel.cognitive_processes, 0);

    kernel_reset();
    TEST_PASS();
}

void test_process_fork_simulation(void) {
    TEST_CASE("process fork simulation");
    kernel_init();

    /* Parent process */
    Proc *parent = proc_create("parent");
    proc_cognitive_init(parent);
    parent->cognitive->attention_level = 0.8f;
    parent->cognitive->motivation = 0.9f;

    /* Fork child - new independent cognitive state */
    Proc *child = proc_create("child");
    proc_cognitive_init(child);

    /* Child should have default values, not parent's */
    ASSERT_FLOAT_EQ(child->cognitive->attention_level, 0.5f, 0.001f);
    ASSERT_FLOAT_EQ(child->cognitive->motivation, 0.5f, 0.001f);

    ASSERT_EQ(opencog_kernel.cognitive_processes, 2);

    kernel_reset();
    TEST_PASS();
}

/* Stress Tests */
void test_many_processes(void) {
    TEST_CASE("many processes with cognitive state");
    kernel_init();

    #define NUM_PROCS 100
    Proc *procs[NUM_PROCS];

    for (int i = 0; i < NUM_PROCS; i++) {
        char name[32];
        sprintf(name, "proc_%d", i);
        procs[i] = proc_create(name);
        proc_cognitive_init(procs[i]);
        procs[i]->state = Ready;
    }

    ASSERT_EQ(opencog_kernel.cognitive_processes, NUM_PROCS);

    cognitive_schedule();

    for (int i = 0; i < NUM_PROCS; i++) {
        ASSERT_NOT_NULL(procs[i]->cognitive);
    }

    kernel_reset();
    TEST_PASS();
}

void test_rapid_create_destroy(void) {
    TEST_CASE("rapid create/destroy cycles");
    kernel_init();

    for (int i = 0; i < 50; i++) {
        Proc *p = proc_create("rapid");
        proc_cognitive_init(p);
        ASSERT_EQ(opencog_kernel.cognitive_processes, 1);
        proc_destroy(p);
        ASSERT_EQ(opencog_kernel.cognitive_processes, 0);
    }

    kernel_reset();
    TEST_PASS();
}

/*
 * Main test runner
 */
int main(void) {
    printf("Process Cognitive Integration Unit Tests\n");
    printf("=========================================\n");
    printf("Testing cognitive state management in OS processes\n");

    /* Process Creation Tests */
    TEST_SUITE("Process Creation Tests");
    test_proc_create_basic();
    test_proc_create_multiple();

    /* Process Cognitive Init Tests */
    TEST_SUITE("Process Cognitive Init Tests");
    test_proc_cognitive_init_basic();
    test_proc_cognitive_init_goals();
    test_proc_cognitive_init_null();
    test_proc_cognitive_init_multiple();

    /* Process Cognitive Cleanup Tests */
    TEST_SUITE("Process Cognitive Cleanup Tests");
    test_proc_cognitive_cleanup_basic();
    test_proc_cognitive_cleanup_null();
    test_proc_cognitive_cleanup_sequential();

    /* Cognitive State Tests */
    TEST_SUITE("Cognitive State Tests");
    test_cognitive_state_defaults();
    test_cognitive_state_modification();

    /* Cognitive Scheduling Tests */
    TEST_SUITE("Cognitive Scheduling Tests");
    test_cognitive_schedule_empty();
    test_cognitive_schedule_single_process();
    test_cognitive_schedule_multiple_processes();
    test_cognitive_schedule_priority_attention();
    test_cognitive_schedule_attention_bounds();
    test_cognitive_schedule_no_cognitive();

    /* Goal Management Tests */
    TEST_SUITE("Goal Management Tests");
    test_process_goals_survival();
    test_process_goals_efficiency();
    test_process_goals_unique_ids();

    /* Local AtomSpace Tests */
    TEST_SUITE("Local AtomSpace Tests");
    test_process_local_atomspace();
    test_process_atomspace_isolation();

    /* Think Time Tests */
    TEST_SUITE("Think Time Tests");
    test_think_time_accumulation();
    test_think_time_only_active();

    /* Reasoning Cycle Tests */
    TEST_SUITE("Reasoning Cycle Tests");
    test_reasoning_cycle_goal_satisfaction();
    test_reasoning_cycle_timestamp();

    /* Process Lifecycle Tests */
    TEST_SUITE("Process Lifecycle Tests");
    test_process_lifecycle();
    test_process_fork_simulation();

    /* Stress Tests */
    TEST_SUITE("Stress Tests");
    test_many_processes();
    test_rapid_create_destroy();

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
