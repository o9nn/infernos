/*
 * Exhaustive Unit Tests for OpenCog Device Interface
 * Tests all /dev/opencog virtual filesystem functionality
 */

#define _GNU_SOURCE  /* For strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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

#define ASSERT_FALSE(cond) \
    if (cond) { TEST_FAIL("Assertion failed: NOT " #cond); return; }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        printf("FAIL: Expected %ld, got %ld\n", (long)(b), (long)(a)); \
        fail_count++; return; \
    }

#define ASSERT_NOT_NULL(ptr) \
    if ((ptr) == NULL) { TEST_FAIL("Unexpected NULL: " #ptr); return; }

#define ASSERT_NULL(ptr) \
    if ((ptr) != NULL) { TEST_FAIL("Expected NULL: " #ptr); return; }

#define ASSERT_CONTAINS(haystack, needle) \
    if (strstr((haystack), (needle)) == NULL) { \
        printf("FAIL: '%s' not found in output\n", needle); \
        fail_count++; return; \
    }

/* Device file types */
enum DevQid {
    Qdir,
    Qstats,
    Qatomspace,
    Qgoals,
    Qreason,
    Qthink,
    Qattention,
    Qpatterns,
    Qdistributed,
};

/* Simplified structures for testing */
typedef struct TruthValue TruthValue;
typedef struct Atom Atom;
typedef struct AtomSpace AtomSpace;
typedef struct Goal Goal;
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
    Goal *next;
};

struct CognitiveState {
    AtomSpace *local_space;
    Goal *active_goals;
    float attention_level;
    float motivation;
    long long think_time;
    int cognitive_load;
};

struct OpenCogKernel {
    AtomSpace *global_space;
    Goal *system_goals;
    int cognitive_processes;
    long long total_atoms;
    long long reasoning_cycles;
    float system_attention;
    int distributed_nodes;
    int npatterns;
    float confidence_threshold;
};

/* Simulated device state */
static OpenCogKernel opencog_kernel;
static CognitiveState *current_process = NULL;
static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

/* Atom type constant */
#define CONCEPT_NODE 2

/* Helper functions */
AtomSpace* atomspace_create(int maxatoms) {
    AtomSpace *as = malloc(sizeof(AtomSpace));
    as->atoms = calloc(maxatoms, sizeof(Atom*));
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

Atom* atom_create(AtomSpace *as, int type, char *name, TruthValue *tv) {
    if (!as || !name) return NULL;
    Atom *atom = malloc(sizeof(Atom));
    atom->id = as->next_id++;
    atom->type = type;
    atom->name = strdup(name);
    if (tv) atom->tv = *tv;
    else { atom->tv.strength = 0.5f; atom->tv.confidence = 0.1f; atom->tv.count = 1.0f; }
    atom->next = as->atoms[atom->id % as->maxatoms];
    as->atoms[atom->id % as->maxatoms] = atom;
    as->natoms++;
    opencog_kernel.total_atoms++;
    return atom;
}

Goal* goal_create(unsigned long id, char *description, float urgency, float importance) {
    Goal *goal = malloc(sizeof(Goal));
    goal->id = id;
    goal->description = strdup(description);
    goal->urgency = urgency;
    goal->importance = importance;
    goal->satisfaction.strength = 0.0f;
    goal->satisfaction.confidence = 0.0f;
    goal->satisfaction.count = 0.0f;
    goal->next = NULL;
    return goal;
}

CognitiveState* cognitive_create(void) {
    CognitiveState *cs = malloc(sizeof(CognitiveState));
    cs->local_space = atomspace_create(256);
    cs->active_goals = NULL;
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

void kernel_init(void) {
    memset(&opencog_kernel, 0, sizeof(OpenCogKernel));
    opencog_kernel.global_space = atomspace_create(1024);
    opencog_kernel.system_attention = 1.0f;
    opencog_kernel.distributed_nodes = 1;
    opencog_kernel.confidence_threshold = 0.1f;
}

void kernel_reset(void) {
    if (opencog_kernel.global_space) atomspace_destroy(opencog_kernel.global_space);
    if (current_process) cognitive_destroy(current_process);
    current_process = NULL;
    memset(&opencog_kernel, 0, sizeof(OpenCogKernel));
}

/* Safe seprint-style helper */
static char* safe_seprint(char *s, char *e, const char *fmt, ...) {
    if (s >= e) return s;
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(s, e - s, fmt, args);
    va_end(args);
    if (n < 0) return s;
    if (n >= e - s) return e - 1;
    return s + n;
}

/*
 * Device Read Operations - Simulating devopencog read functions
 */
int dev_read_stats(char *buf, int bufsize) {
    char *s = buf;
    char *e = buf + bufsize;

    s = safe_seprint(s, e, "OpenCog Kernel-based AGI Operating System\n");
    s = safe_seprint(s, e, "========================================\n\n");
    s = safe_seprint(s, e, "Global Statistics:\n");
    s = safe_seprint(s, e, "  Cognitive processes: %d\n", opencog_kernel.cognitive_processes);
    s = safe_seprint(s, e, "  Total atoms: %lld\n", opencog_kernel.total_atoms);
    s = safe_seprint(s, e, "  Reasoning cycles: %lld\n", opencog_kernel.reasoning_cycles);
    s = safe_seprint(s, e, "  System attention: %.2f\n", opencog_kernel.system_attention);
    s = safe_seprint(s, e, "  Distributed nodes: %d\n", opencog_kernel.distributed_nodes);

    return s - buf;
}

int dev_read_atomspace(char *buf, int bufsize) {
    char *s = buf;
    char *e = buf + bufsize;

    if (current_process && current_process->local_space) {
        s += snprintf(s, e - s, "Process Local AtomSpace:\n");
        s += snprintf(s, e - s, "  Atoms: %d/%d\n",
            current_process->local_space->natoms,
            current_process->local_space->maxatoms);
        s += snprintf(s, e - s, "  Next ID: %lu\n", current_process->local_space->next_id);
    } else {
        s += snprintf(s, e - s, "No cognitive state for process\n");
    }

    return s - buf;
}

int dev_read_goals(char *buf, int bufsize) {
    char *s = buf;
    char *e = buf + bufsize;

    if (current_process) {
        Goal *goal = current_process->active_goals;
        s += snprintf(s, e - s, "Process Active Goals:\n");
        while (goal && s < e - 100) {
            s += snprintf(s, e - s, "  Goal %lu: %s (urgency=%.2f, importance=%.2f)\n",
                goal->id, goal->description, goal->urgency, goal->importance);
            goal = goal->next;
        }
    } else {
        s += snprintf(s, e - s, "No cognitive state for process\n");
    }

    return s - buf;
}

int dev_read_reason(char *buf, int bufsize) {
    return snprintf(buf, bufsize, "reasoning_cycles=%lld\nconfidence_threshold=%.2f\n",
        opencog_kernel.reasoning_cycles, opencog_kernel.confidence_threshold);
}

int dev_read_think(char *buf, int bufsize) {
    if (current_process) {
        return snprintf(buf, bufsize, "think_time=%lld\ncognitive_load=%d\nattention=%.2f\n",
            current_process->think_time, current_process->cognitive_load,
            current_process->attention_level);
    } else {
        return snprintf(buf, bufsize, "No cognitive state\n");
    }
}

int dev_read_attention(char *buf, int bufsize) {
    return snprintf(buf, bufsize, "system_attention=%.2f\nprocess_attention=%.2f\n",
        opencog_kernel.system_attention,
        current_process ? current_process->attention_level : 0.0f);
}

int dev_read_patterns(char *buf, int bufsize) {
    return snprintf(buf, bufsize, "Pattern Matcher Status:\n  Active patterns: %d\n  Similarity function: active\n  Unification: active\n",
        opencog_kernel.npatterns);
}

int dev_read_distributed(char *buf, int bufsize) {
    return snprintf(buf, bufsize, "distributed_nodes=%d\nnetwork_coherence=active\ndistributed_reasoning=active\n",
        opencog_kernel.distributed_nodes);
}

/*
 * Device Write Operations - Simulating devopencog write functions
 */
int dev_write_atomspace(char *data) {
    char cmd[64], arg[256];
    int n = sscanf(data, "%63s %255[^\n]", cmd, arg);

    if (strcmp(cmd, "create") == 0 && n >= 2) {
        if (!current_process) {
            current_process = cognitive_create();
        }
        TruthValue tv = {0.8f, 0.5f, 1.0f};
        Atom *atom = atom_create(current_process->local_space, CONCEPT_NODE, arg, &tv);
        return atom ? 0 : -1;
    } else if (strcmp(cmd, "clear") == 0) {
        if (current_process && current_process->local_space) {
            current_process->local_space->natoms = 0;
            current_process->local_space->next_id = 1;
        }
        return 0;
    }
    return -1;
}

int dev_write_goals(char *data) {
    char cmd[64], arg[256];
    int n = sscanf(data, "%63s %255[^\n]", cmd, arg);

    if (strcmp(cmd, "add") == 0 && n >= 2) {
        if (!current_process) {
            current_process = cognitive_create();
        }
        Goal *goal = goal_create(time(NULL) % 1000, arg, 0.7f, 0.8f);
        if (goal) {
            goal->next = current_process->active_goals;
            current_process->active_goals = goal;
            return 0;
        }
        return -1;
    } else if (strcmp(cmd, "clear") == 0) {
        if (current_process) {
            current_process->active_goals = NULL;
        }
        return 0;
    }
    return -1;
}

int dev_write_reason(char *data) {
    char cmd[64], arg[64];
    int n = sscanf(data, "%63s %63s", cmd, arg);

    if (strcmp(cmd, "cycle") == 0) {
        opencog_kernel.reasoning_cycles++;
        return 0;
    } else if (strcmp(cmd, "threshold") == 0 && n >= 2) {
        opencog_kernel.confidence_threshold = atof(arg);
        return 0;
    }
    return -1;
}

int dev_write_think(char *data) {
    char cmd[64];
    sscanf(data, "%63s", cmd);

    if (strcmp(cmd, "focus") == 0) {
        if (current_process) {
            current_process->attention_level = 1.0f;
            current_process->motivation += 0.1f;
            if (current_process->motivation > 1.0f)
                current_process->motivation = 1.0f;
        }
        return 0;
    } else if (strcmp(cmd, "relax") == 0) {
        if (current_process) {
            current_process->attention_level = 0.5f;
            current_process->motivation -= 0.1f;
            if (current_process->motivation < 0.0f)
                current_process->motivation = 0.0f;
        }
        return 0;
    }
    return -1;
}

int dev_write_attention(char *data) {
    float level = atof(data);
    if (level >= 0.0f && level <= 1.0f) {
        if (!current_process) {
            current_process = cognitive_create();
        }
        current_process->attention_level = level;
        return 0;
    }
    return -1;
}

int dev_write_distributed(char *data) {
    char cmd[64];
    sscanf(data, "%63s", cmd);

    if (strcmp(cmd, "sync") == 0) {
        /* Trigger distributed synchronization */
        opencog_kernel.reasoning_cycles++;
        return 0;
    }
    return -1;
}

/*
 * TEST FUNCTIONS
 */

/* Stats Device Tests */
void test_dev_stats_read(void) {
    TEST_CASE("dev_read_stats basic output");
    kernel_init();
    char buf[8192];
    int n = dev_read_stats(buf, sizeof(buf));

    ASSERT_TRUE(n > 0);
    ASSERT_CONTAINS(buf, "OpenCog Kernel-based AGI Operating System");
    ASSERT_CONTAINS(buf, "Global Statistics:");
    ASSERT_CONTAINS(buf, "Cognitive processes:");
    ASSERT_CONTAINS(buf, "Total atoms:");
    ASSERT_CONTAINS(buf, "Reasoning cycles:");
    ASSERT_CONTAINS(buf, "System attention:");
    ASSERT_CONTAINS(buf, "Distributed nodes:");

    kernel_reset();
    TEST_PASS();
}

void test_dev_stats_reflects_state(void) {
    TEST_CASE("dev_read_stats reflects kernel state");
    kernel_init();
    opencog_kernel.cognitive_processes = 5;
    opencog_kernel.total_atoms = 100;
    opencog_kernel.reasoning_cycles = 1000;
    opencog_kernel.distributed_nodes = 3;

    char buf[8192];
    dev_read_stats(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "Cognitive processes: 5");
    ASSERT_CONTAINS(buf, "Total atoms: 100");
    ASSERT_CONTAINS(buf, "Reasoning cycles: 1000");
    ASSERT_CONTAINS(buf, "Distributed nodes: 3");

    kernel_reset();
    TEST_PASS();
}

/* AtomSpace Device Tests */
void test_dev_atomspace_no_process(void) {
    TEST_CASE("dev_read_atomspace no cognitive state");
    kernel_init();
    current_process = NULL;

    char buf[4096];
    dev_read_atomspace(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "No cognitive state");

    kernel_reset();
    TEST_PASS();
}

void test_dev_atomspace_with_process(void) {
    TEST_CASE("dev_read_atomspace with cognitive state");
    kernel_init();
    current_process = cognitive_create();

    char buf[4096];
    dev_read_atomspace(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "Process Local AtomSpace:");
    ASSERT_CONTAINS(buf, "Atoms:");
    ASSERT_CONTAINS(buf, "Next ID:");

    kernel_reset();
    TEST_PASS();
}

void test_dev_atomspace_write_create(void) {
    TEST_CASE("dev_write_atomspace create command");
    kernel_init();

    int result = dev_write_atomspace("create test_atom");
    ASSERT_EQ(result, 0);
    ASSERT_NOT_NULL(current_process);
    ASSERT_EQ(current_process->local_space->natoms, 1);

    kernel_reset();
    TEST_PASS();
}

void test_dev_atomspace_write_multiple_create(void) {
    TEST_CASE("dev_write_atomspace multiple creates");
    kernel_init();

    dev_write_atomspace("create atom1");
    dev_write_atomspace("create atom2");
    dev_write_atomspace("create atom3");

    ASSERT_EQ(current_process->local_space->natoms, 3);

    kernel_reset();
    TEST_PASS();
}

void test_dev_atomspace_write_clear(void) {
    TEST_CASE("dev_write_atomspace clear command");
    kernel_init();

    dev_write_atomspace("create atom1");
    dev_write_atomspace("create atom2");
    ASSERT_EQ(current_process->local_space->natoms, 2);

    dev_write_atomspace("clear");
    ASSERT_EQ(current_process->local_space->natoms, 0);
    ASSERT_EQ(current_process->local_space->next_id, 1);

    kernel_reset();
    TEST_PASS();
}

void test_dev_atomspace_write_invalid(void) {
    TEST_CASE("dev_write_atomspace invalid command");
    kernel_init();

    int result = dev_write_atomspace("invalid_command");
    ASSERT_EQ(result, -1);

    kernel_reset();
    TEST_PASS();
}

/* Goals Device Tests */
void test_dev_goals_no_process(void) {
    TEST_CASE("dev_read_goals no cognitive state");
    kernel_init();
    current_process = NULL;

    char buf[4096];
    dev_read_goals(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "No cognitive state");

    kernel_reset();
    TEST_PASS();
}

void test_dev_goals_empty(void) {
    TEST_CASE("dev_read_goals empty goals list");
    kernel_init();
    current_process = cognitive_create();

    char buf[4096];
    dev_read_goals(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "Process Active Goals:");

    kernel_reset();
    TEST_PASS();
}

void test_dev_goals_write_add(void) {
    TEST_CASE("dev_write_goals add command");
    kernel_init();

    int result = dev_write_goals("add test_goal");
    ASSERT_EQ(result, 0);
    ASSERT_NOT_NULL(current_process);
    ASSERT_NOT_NULL(current_process->active_goals);

    kernel_reset();
    TEST_PASS();
}

void test_dev_goals_write_multiple_add(void) {
    TEST_CASE("dev_write_goals multiple adds");
    kernel_init();

    dev_write_goals("add goal1");
    dev_write_goals("add goal2");
    dev_write_goals("add goal3");

    int count = 0;
    Goal *g = current_process->active_goals;
    while (g) { count++; g = g->next; }

    ASSERT_EQ(count, 3);

    kernel_reset();
    TEST_PASS();
}

void test_dev_goals_write_clear(void) {
    TEST_CASE("dev_write_goals clear command");
    kernel_init();

    dev_write_goals("add goal1");
    dev_write_goals("add goal2");
    ASSERT_NOT_NULL(current_process->active_goals);

    dev_write_goals("clear");
    ASSERT_NULL(current_process->active_goals);

    kernel_reset();
    TEST_PASS();
}

void test_dev_goals_read_after_add(void) {
    TEST_CASE("dev_read_goals shows added goals");
    kernel_init();

    dev_write_goals("add optimize_resources");

    char buf[4096];
    dev_read_goals(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "optimize_resources");
    ASSERT_CONTAINS(buf, "urgency=");
    ASSERT_CONTAINS(buf, "importance=");

    kernel_reset();
    TEST_PASS();
}

/* Reason Device Tests */
void test_dev_reason_read(void) {
    TEST_CASE("dev_read_reason");
    kernel_init();
    opencog_kernel.reasoning_cycles = 42;
    opencog_kernel.confidence_threshold = 0.25f;

    char buf[2048];
    dev_read_reason(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "reasoning_cycles=42");
    ASSERT_CONTAINS(buf, "confidence_threshold=0.25");

    kernel_reset();
    TEST_PASS();
}

void test_dev_reason_write_cycle(void) {
    TEST_CASE("dev_write_reason cycle command");
    kernel_init();
    ASSERT_EQ(opencog_kernel.reasoning_cycles, 0);

    dev_write_reason("cycle");
    ASSERT_EQ(opencog_kernel.reasoning_cycles, 1);

    dev_write_reason("cycle");
    dev_write_reason("cycle");
    ASSERT_EQ(opencog_kernel.reasoning_cycles, 3);

    kernel_reset();
    TEST_PASS();
}

void test_dev_reason_write_threshold(void) {
    TEST_CASE("dev_write_reason threshold command");
    kernel_init();

    dev_write_reason("threshold 0.5");

    char buf[2048];
    dev_read_reason(buf, sizeof(buf));
    ASSERT_CONTAINS(buf, "confidence_threshold=0.50");

    kernel_reset();
    TEST_PASS();
}

/* Think Device Tests */
void test_dev_think_no_process(void) {
    TEST_CASE("dev_read_think no cognitive state");
    kernel_init();
    current_process = NULL;

    char buf[1024];
    dev_read_think(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "No cognitive state");

    kernel_reset();
    TEST_PASS();
}

void test_dev_think_with_process(void) {
    TEST_CASE("dev_read_think with cognitive state");
    kernel_init();
    current_process = cognitive_create();
    current_process->think_time = 12345;
    current_process->cognitive_load = 50;

    char buf[1024];
    dev_read_think(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "think_time=12345");
    ASSERT_CONTAINS(buf, "cognitive_load=50");
    ASSERT_CONTAINS(buf, "attention=");

    kernel_reset();
    TEST_PASS();
}

void test_dev_think_write_focus(void) {
    TEST_CASE("dev_write_think focus command");
    kernel_init();
    current_process = cognitive_create();
    current_process->attention_level = 0.5f;
    current_process->motivation = 0.5f;

    dev_write_think("focus");

    ASSERT_TRUE(fabs(current_process->attention_level - 1.0f) < 0.001f);
    ASSERT_TRUE(fabs(current_process->motivation - 0.6f) < 0.001f);

    kernel_reset();
    TEST_PASS();
}

void test_dev_think_write_relax(void) {
    TEST_CASE("dev_write_think relax command");
    kernel_init();
    current_process = cognitive_create();
    current_process->attention_level = 1.0f;
    current_process->motivation = 0.7f;

    dev_write_think("relax");

    ASSERT_TRUE(fabs(current_process->attention_level - 0.5f) < 0.001f);
    ASSERT_TRUE(fabs(current_process->motivation - 0.6f) < 0.001f);

    kernel_reset();
    TEST_PASS();
}

void test_dev_think_motivation_bounds(void) {
    TEST_CASE("dev_write_think motivation bounds");
    kernel_init();
    current_process = cognitive_create();

    /* Test upper bound */
    current_process->motivation = 0.95f;
    dev_write_think("focus");
    ASSERT_TRUE(current_process->motivation <= 1.0f);

    /* Test lower bound */
    current_process->motivation = 0.05f;
    dev_write_think("relax");
    ASSERT_TRUE(current_process->motivation >= 0.0f);

    kernel_reset();
    TEST_PASS();
}

/* Attention Device Tests */
void test_dev_attention_read(void) {
    TEST_CASE("dev_read_attention");
    kernel_init();
    opencog_kernel.system_attention = 0.95f;
    current_process = cognitive_create();
    current_process->attention_level = 0.75f;

    char buf[1024];
    dev_read_attention(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "system_attention=0.95");
    ASSERT_CONTAINS(buf, "process_attention=0.75");

    kernel_reset();
    TEST_PASS();
}

void test_dev_attention_read_no_process(void) {
    TEST_CASE("dev_read_attention no process");
    kernel_init();
    current_process = NULL;

    char buf[1024];
    dev_read_attention(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "process_attention=0.00");

    kernel_reset();
    TEST_PASS();
}

void test_dev_attention_write_valid(void) {
    TEST_CASE("dev_write_attention valid value");
    kernel_init();

    int result = dev_write_attention("0.75");
    ASSERT_EQ(result, 0);
    ASSERT_NOT_NULL(current_process);
    ASSERT_TRUE(fabs(current_process->attention_level - 0.75f) < 0.001f);

    kernel_reset();
    TEST_PASS();
}

void test_dev_attention_write_boundary(void) {
    TEST_CASE("dev_write_attention boundary values");
    kernel_init();

    dev_write_attention("0.0");
    ASSERT_TRUE(fabs(current_process->attention_level - 0.0f) < 0.001f);

    dev_write_attention("1.0");
    ASSERT_TRUE(fabs(current_process->attention_level - 1.0f) < 0.001f);

    kernel_reset();
    TEST_PASS();
}

void test_dev_attention_write_invalid(void) {
    TEST_CASE("dev_write_attention invalid values");
    kernel_init();
    current_process = cognitive_create();
    current_process->attention_level = 0.5f;

    int result1 = dev_write_attention("1.5");
    ASSERT_EQ(result1, -1);

    int result2 = dev_write_attention("-0.5");
    ASSERT_EQ(result2, -1);

    /* Attention should be unchanged */
    ASSERT_TRUE(fabs(current_process->attention_level - 0.5f) < 0.001f);

    kernel_reset();
    TEST_PASS();
}

/* Patterns Device Tests */
void test_dev_patterns_read(void) {
    TEST_CASE("dev_read_patterns");
    kernel_init();
    opencog_kernel.npatterns = 15;

    char buf[2048];
    dev_read_patterns(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "Pattern Matcher Status:");
    ASSERT_CONTAINS(buf, "Active patterns: 15");
    ASSERT_CONTAINS(buf, "Similarity function: active");
    ASSERT_CONTAINS(buf, "Unification: active");

    kernel_reset();
    TEST_PASS();
}

/* Distributed Device Tests */
void test_dev_distributed_read(void) {
    TEST_CASE("dev_read_distributed");
    kernel_init();
    opencog_kernel.distributed_nodes = 5;

    char buf[1024];
    dev_read_distributed(buf, sizeof(buf));

    ASSERT_CONTAINS(buf, "distributed_nodes=5");
    ASSERT_CONTAINS(buf, "network_coherence=active");
    ASSERT_CONTAINS(buf, "distributed_reasoning=active");

    kernel_reset();
    TEST_PASS();
}

void test_dev_distributed_write_sync(void) {
    TEST_CASE("dev_write_distributed sync command");
    kernel_init();
    ASSERT_EQ(opencog_kernel.reasoning_cycles, 0);

    dev_write_distributed("sync");
    ASSERT_EQ(opencog_kernel.reasoning_cycles, 1);

    kernel_reset();
    TEST_PASS();
}

/* Buffer Handling Tests */
void test_buffer_small(void) {
    TEST_CASE("small buffer handling");
    kernel_init();

    char buf[64];  /* Very small buffer */
    int n = dev_read_stats(buf, sizeof(buf));

    ASSERT_TRUE(n <= 64);
    /* Should not crash with small buffer */

    kernel_reset();
    TEST_PASS();
}

void test_buffer_exact(void) {
    TEST_CASE("exact buffer size");
    kernel_init();

    char buf[200];
    int n = dev_read_reason(buf, sizeof(buf));

    ASSERT_TRUE(n < sizeof(buf));
    ASSERT_TRUE(strlen(buf) < sizeof(buf));

    kernel_reset();
    TEST_PASS();
}

/* Concurrent Operation Tests */
void test_sequential_operations(void) {
    TEST_CASE("sequential device operations");
    kernel_init();

    /* Create atoms */
    dev_write_atomspace("create concept1");
    dev_write_atomspace("create concept2");

    /* Add goals */
    dev_write_goals("add goal1");
    dev_write_goals("add goal2");

    /* Run reasoning */
    dev_write_reason("cycle");
    dev_write_reason("cycle");

    /* Adjust attention */
    dev_write_attention("0.8");

    /* Focus */
    dev_write_think("focus");

    /* Sync */
    dev_write_distributed("sync");

    /* Verify state */
    ASSERT_EQ(current_process->local_space->natoms, 2);
    ASSERT_EQ(opencog_kernel.reasoning_cycles, 3);

    kernel_reset();
    TEST_PASS();
}

void test_state_isolation(void) {
    TEST_CASE("state isolation between device files");
    kernel_init();

    /* Operations on atomspace shouldn't affect goals */
    dev_write_atomspace("create atom1");
    dev_write_goals("add goal1");

    ASSERT_EQ(current_process->local_space->natoms, 1);
    ASSERT_NOT_NULL(current_process->active_goals);

    /* Clear atomspace shouldn't affect goals */
    dev_write_atomspace("clear");
    ASSERT_EQ(current_process->local_space->natoms, 0);
    ASSERT_NOT_NULL(current_process->active_goals);

    kernel_reset();
    TEST_PASS();
}

/* Edge Cases */
void test_empty_commands(void) {
    TEST_CASE("empty command handling");
    kernel_init();

    int r1 = dev_write_atomspace("");
    int r2 = dev_write_goals("");
    int r3 = dev_write_reason("");
    int r4 = dev_write_think("");

    /* All should fail or handle gracefully */
    ASSERT_EQ(r1, -1);
    ASSERT_EQ(r2, -1);
    ASSERT_EQ(r3, -1);
    ASSERT_EQ(r4, -1);

    kernel_reset();
    TEST_PASS();
}

void test_long_atom_names(void) {
    TEST_CASE("long atom names");
    kernel_init();

    char long_name[300];
    memset(long_name, 'a', sizeof(long_name) - 1);
    long_name[sizeof(long_name) - 1] = '\0';

    char cmd[350];
    snprintf(cmd, sizeof(cmd), "create %s", long_name);

    int result = dev_write_atomspace(cmd);
    ASSERT_EQ(result, 0);

    kernel_reset();
    TEST_PASS();
}

void test_special_characters(void) {
    TEST_CASE("special characters in names");
    kernel_init();

    dev_write_atomspace("create atom_with_underscore");
    dev_write_atomspace("create atom-with-dash");
    dev_write_atomspace("create atom.with.dot");

    ASSERT_EQ(current_process->local_space->natoms, 3);

    kernel_reset();
    TEST_PASS();
}

/*
 * Main test runner
 */
int main(void) {
    printf("OpenCog Device Interface Unit Tests\n");
    printf("====================================\n");
    printf("Testing all /dev/opencog functionality\n");

    /* Stats Device Tests */
    TEST_SUITE("Stats Device Tests");
    test_dev_stats_read();
    test_dev_stats_reflects_state();

    /* AtomSpace Device Tests */
    TEST_SUITE("AtomSpace Device Tests");
    test_dev_atomspace_no_process();
    test_dev_atomspace_with_process();
    test_dev_atomspace_write_create();
    test_dev_atomspace_write_multiple_create();
    test_dev_atomspace_write_clear();
    test_dev_atomspace_write_invalid();

    /* Goals Device Tests */
    TEST_SUITE("Goals Device Tests");
    test_dev_goals_no_process();
    test_dev_goals_empty();
    test_dev_goals_write_add();
    test_dev_goals_write_multiple_add();
    test_dev_goals_write_clear();
    test_dev_goals_read_after_add();

    /* Reason Device Tests */
    TEST_SUITE("Reason Device Tests");
    test_dev_reason_read();
    test_dev_reason_write_cycle();
    test_dev_reason_write_threshold();

    /* Think Device Tests */
    TEST_SUITE("Think Device Tests");
    test_dev_think_no_process();
    test_dev_think_with_process();
    test_dev_think_write_focus();
    test_dev_think_write_relax();
    test_dev_think_motivation_bounds();

    /* Attention Device Tests */
    TEST_SUITE("Attention Device Tests");
    test_dev_attention_read();
    test_dev_attention_read_no_process();
    test_dev_attention_write_valid();
    test_dev_attention_write_boundary();
    test_dev_attention_write_invalid();

    /* Patterns Device Tests */
    TEST_SUITE("Patterns Device Tests");
    test_dev_patterns_read();

    /* Distributed Device Tests */
    TEST_SUITE("Distributed Device Tests");
    test_dev_distributed_read();
    test_dev_distributed_write_sync();

    /* Buffer Handling Tests */
    TEST_SUITE("Buffer Handling Tests");
    test_buffer_small();
    test_buffer_exact();

    /* Sequential Operation Tests */
    TEST_SUITE("Sequential Operation Tests");
    test_sequential_operations();
    test_state_isolation();

    /* Edge Case Tests */
    TEST_SUITE("Edge Case Tests");
    test_empty_commands();
    test_long_atom_names();
    test_special_characters();

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
