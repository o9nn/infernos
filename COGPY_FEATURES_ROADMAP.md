# CogPy/CogFernos Features Implementation Roadmap for Infernos

**Date:** 2026-01-14
**Based on:** Analysis of `.github/agents/` specifications and existing codebase

## Executive Summary

This document identifies features from the CogPy ecosystem that can be implemented in Infernos to enhance its AGI capabilities. Features are prioritized by integration complexity and value.

---

## Feature Categories

| Category | CogPy Source | Priority | Complexity |
|----------|--------------|----------|------------|
| GGML Tensor Operations | cogpy/ggml-cog, coggml-kernel.md | HIGH | Medium |
| Echo State Networks | a9nn.md, nnecco-a9nn.md | HIGH | High |
| GGUF Model Support | cogpy/gguf, gguf.md | MEDIUM | Low |
| Consciousness Layers | nnecco-a9nn.md | MEDIUM | High |
| Emotion Processing | nnecco-a9nn.md, TOGA.md | LOW | Medium |
| Personality Tensors | TOGA.md, a9nn.md | LOW | Medium |
| Distributed AtomSpace | FlareCog.md | MEDIUM | High |
| PLN (Probabilistic Logic) | coggml-kernel.md | HIGH | High |

---

## HIGH PRIORITY FEATURES

### 1. GGML Tensor Kernel Completion

**Source:** `coggml-kernel.md`, `cogpy/ggml-cog`

**Current State:** Only `tensor_add()` and `tensor_matmul()` implemented (2/16 operations)

**Features to Implement:**

```c
// Priority tensor operations (agentic_cognitive_grammar/ggml_tensor_kernels/tensor_ops.c)

// 1. Activation Functions
int tensor_relu(TensorOp *op, void *ctx);      // ReLU activation
int tensor_sigmoid(TensorOp *op, void *ctx);   // Sigmoid activation
int tensor_softmax(TensorOp *op, void *ctx);   // Softmax for attention

// 2. Neural Network Layers
int tensor_layernorm(TensorOp *op, void *ctx); // Layer normalization
int tensor_attention(TensorOp *op, void *ctx); // Multi-head attention
int tensor_embedding(TensorOp *op, void *ctx); // Embedding lookup

// 3. Recurrent Cells
int tensor_gru(TensorOp *op, void *ctx);       // GRU cell
int tensor_lstm(TensorOp *op, void *ctx);      // LSTM cell
```

**Implementation Pattern (from coggml-kernel.md):**
```c
/**
 * hgfs_alloc - Allocate memory as a GGML tensor node
 * @size: Size in bytes
 * @depth: Membrane depth (OEIS A000081)
 */
void *hgfs_alloc(size_t size, uint32_t depth) {
    struct ggml_tensor *t = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, size / 4);
    t->depth = depth;
    register_hypergraph_node(t, depth);
    return (void *)t->data;
}
```

**Integration Points:**
- `os/port/opencog.c` - Use tensors for AtomSpace operations
- `agentic_cognitive_grammar/ggml_tensor_kernels/` - Core implementations

---

### 2. Echo State Reservoir Network

**Source:** `a9nn.md`, `nnecco-a9nn.md`

**Current State:** Not implemented

**Feature Specification:**

```c
// New file: os/port/reservoir.c

typedef struct EchoReservoir {
    int size;           // Default: 847 neurons
    float spectralRadius;  // Default: 0.9
    float leakRate;        // Default: 0.3
    float inputScaling;    // Default: 1.0

    float *W_reservoir;    // size x size weights
    float *W_input;        // size x input_dim
    float *W_output;       // output_dim x size
    float *state;          // Current reservoir state
} EchoReservoir;

// Functions
EchoReservoir* reservoir_create(int size, int input_dim, int output_dim);
float* reservoir_forward(EchoReservoir *r, float *input);
void reservoir_adapt(EchoReservoir *r, float arousal, int frame);
void reservoir_reset(EchoReservoir *r);
```

**Integration with OpenCog Kernel:**
```c
// In os/port/portdat.h - Add to CognitiveState
struct CognitiveState {
    // ... existing fields ...
    struct EchoReservoir* reservoir;  // Echo State Network for temporal processing
};

// In os/port/opencog.c - Initialize reservoir per process
void proc_cognitive_init(Proc *p) {
    // ... existing code ...
    p->cognitive->reservoir = reservoir_create(847, 768, 256);
}
```

---

### 3. Probabilistic Logic Networks (PLN)

**Source:** `coggml-kernel.md`, `FlareCog.md`

**Current State:** `inference_step()` is placeholder (os/port/opencog.c:271-292)

**Features to Implement:**

```c
// Replace stub inference_step() with real PLN

// Truth value operations
TruthValue tv_deduction(TruthValue premise1, TruthValue premise2);
TruthValue tv_induction(TruthValue premise, TruthValue example);
TruthValue tv_abduction(TruthValue premise, TruthValue conclusion);
TruthValue tv_revision(TruthValue tv1, TruthValue tv2);

// Inference rules
void pln_forward_chain(ReasoningEngine *re, int max_steps);
void pln_backward_chain(ReasoningEngine *re, Atom *goal, int max_depth);
Atom* pln_unify(Atom *pattern, Atom *target, Atom ***bindings);

// Attention allocation (ECAN)
void ecan_spread_importance(AtomSpace *as);
void ecan_update_sti(Atom *atom, float stimulation);  // Short-term importance
void ecan_update_lti(Atom *atom, float amount);       // Long-term importance
```

---

## MEDIUM PRIORITY FEATURES

### 4. GGUF Model File Support

**Source:** `gguf.md`, `cogpy/gguf`

**Current State:** Not implemented

**Use Case:** Load pre-trained neural network weights into AtomSpace

```c
// New file: os/port/gguf.c

typedef struct GGUFHeader {
    uint32_t magic;        // 0x46554747
    uint32_t version;
    uint64_t tensor_count;
    uint64_t metadata_count;
} GGUFHeader;

typedef struct GGUFModel {
    GGUFHeader header;
    char *name;
    char *architecture;
    int parameter_count;
    // Tensor data
} GGUFModel;

// Device interface: /dev/opencog/models
GGUFModel* gguf_load(char *path);
int gguf_get_tensor(GGUFModel *m, char *name, float **data, int *dims);
void gguf_close(GGUFModel *m);
```

---

### 5. Consciousness Layer Processing

**Source:** `nnecco-a9nn.md`

**Current State:** Not implemented

**Feature Specification:**

```c
// New structure in os/port/portdat.h

enum ConsciousnessLevel {
    L0_REFLEXIVE = 0,      // Direct stimulus-response
    L1_EXPERIENTIAL = 1,   // Frame-aware perception
    L2_REFLECTIVE = 2,     // Metacognitive analysis
    L3_META = 3            // Self-model reasoning
};

typedef struct ConsciousnessState {
    int level;
    int frame;  // Current cognitive frame (chaos, strategy, play, social)
    vlong last_transition;
    struct {
        int from;
        int to;
        vlong timestamp;
        char *reason;
    } history[32];
    int history_count;
} ConsciousnessState;

// Add to CognitiveState
struct CognitiveState {
    // ... existing ...
    ConsciousnessState *consciousness;
};
```

---

### 6. Distributed AtomSpace Coordination

**Source:** `FlareCog.md`

**Current State:** Basic distributed_nodes counter exists

**Features to Implement:**

```c
// Enhance os/port/devopencog.c /dev/opencog/distributed

// Storage node abstraction
typedef struct StorageNode {
    char *hostname;
    int port;
    AtomSpace *remote_space;
    int connected;
    float latency;
} StorageNode;

// Distributed query engine
typedef struct DistributedQuery {
    char *pattern;
    int ttl;
    StorageNode **nodes;
    int node_count;
} DistributedQuery;

// Functions
StorageNode* storage_node_connect(char *hostname, int port);
Atom** distributed_query(DistributedQuery *q);
void distributed_sync(AtomSpace *local, StorageNode *remote);
```

---

## LOW PRIORITY FEATURES

### 7. Emotion Processing Unit

**Source:** `nnecco-a9nn.md`, `TOGA.md`

```c
// New file: os/port/emotion.c

enum EmotionType {
    EMO_NEUTRAL = 0,
    EMO_HAPPY, EMO_EXCITED, EMO_ANNOYED,
    EMO_THOUGHTFUL, EMO_CONFUSED, EMO_CURIOUS,
    EMO_DETERMINED, EMO_PLAYFUL, EMO_SARCASTIC
};

typedef struct EmotionState {
    int current;
    float intensity;
    float valence;   // -1.0 to 1.0
    float arousal;   // 0.0 to 1.0
} EmotionState;

void emotion_set(CognitiveState *cs, int type, float intensity);
float emotion_modulate_reservoir(EmotionState *es);  // Returns input scaling modifier
```

---

### 8. Personality Tensor System

**Source:** `TOGA.md`, `a9nn.md`

```c
// New structure

typedef struct PersonalityTensor {
    float traits[16];  // Mutable personality traits
    float constraints[4];  // Immutable ethical constraints (always 1.0)
} PersonalityTensor;

// Traits: cheerfulness, curiosity, creativity, determination, etc.
// Constraints: no_actual_harm, respect_boundaries, constructive_expression, safety

PersonalityTensor* personality_create(float *initial_traits);
void personality_evolve(PersonalityTensor *p, float learning_rate);
float personality_compatibility(PersonalityTensor *p1, PersonalityTensor *p2);
```

---

## Implementation Phases

### Phase 1: Core Tensor Operations (Week 1-2)
1. Complete tensor activation functions (relu, sigmoid, softmax)
2. Implement layer normalization
3. Add embedding lookup

### Phase 2: Reasoning Engine (Week 3-4)
1. Implement PLN truth value operations
2. Add forward/backward chaining
3. Implement ECAN attention allocation

### Phase 3: Temporal Processing (Week 5-6)
1. Implement Echo State Reservoir
2. Add consciousness layer processing
3. Integrate with cognitive scheduler

### Phase 4: Distribution (Week 7-8)
1. Implement storage node abstraction
2. Add distributed query engine
3. Implement inter-node synchronization

### Phase 5: Higher Cognition (Week 9-10)
1. Add emotion processing
2. Implement personality tensors
3. GGUF model loading

---

## Integration Checklist

### Kernel Files to Modify:
- [ ] `os/port/portdat.h` - Add new structures
- [ ] `os/port/opencog.c` - Implement PLN, reservoir
- [ ] `os/port/devopencog.c` - Add new device files
- [ ] `os/port/proc.c` - Initialize new cognitive components

### New Files to Create:
- [ ] `os/port/reservoir.c` - Echo State Network
- [ ] `os/port/pln.c` - Probabilistic Logic Networks
- [ ] `os/port/gguf.c` - GGUF model support
- [ ] `os/port/emotion.c` - Emotion processing

### Agentic Cognitive Grammar Updates:
- [ ] `agentic_cognitive_grammar/ggml_tensor_kernels/tensor_ops.c` - Complete all 16 operations
- [ ] `agentic_cognitive_grammar/nyacc_seeds/neural_grammar.c` - Implement distributed sync

### Python Updates:
- [ ] `python/helpers/cognitive_grammar.py` - Add PLN message types
- [ ] `python/tools/cognitive_network.py` - Add reservoir status queries

---

## References

| Agent Spec | Repository | Key Features |
|------------|------------|--------------|
| coggml-kernel.md | cogpy/occ, cogpy/ggml-cog | GGML kernels, PLN, ECAN |
| a9nn.md | cogpy/a9nn | Echo State, Parallel LLaMA |
| nnecco-a9nn.md | cogpy/a9nn | Consciousness layers, Emotion |
| gguf.md | cogpy/gguf | Model file format |
| FlareCog.md | cogpy/cogflare-temp | Distributed AtomSpace |
| TOGA.md | agent-toga | Personality tensors |

---

**Total New Features Identified:** 24
**Lines of New Code Estimated:** ~3,000-5,000
**Integration Complexity:** Medium-High
