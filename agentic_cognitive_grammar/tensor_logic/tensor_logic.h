/*
 * Tensor Logic - Neural-Symbolic Unification for AGI
 *
 * Based on Tensor Logic research: an AI programming language that
 * deeply unifies deep learning and symbolic AI, combining the
 * scalability of neural networks with the transparency of symbolic
 * knowledge representation and reasoning.
 *
 * This implementation provides:
 * - Tensor-based truth value representation
 * - Differentiable symbolic reasoning
 * - Neural-symbolic atom embeddings
 * - Gradient-based knowledge learning
 * - Distributed tensor operations for cognition
 */

#ifndef TENSOR_LOGIC_H
#define TENSOR_LOGIC_H

#include <stddef.h>

/* Forward declarations */
typedef struct TensorTruthValue TensorTruthValue;
typedef struct TensorAtom TensorAtom;
typedef struct TensorAtomSpace TensorAtomSpace;
typedef struct TensorRule TensorRule;
typedef struct TensorInference TensorInference;
typedef struct TensorLogicEngine TensorLogicEngine;
typedef struct GradientContext GradientContext;

/* Tensor dimensions for embeddings */
#define TENSOR_EMBED_DIM 64
#define TENSOR_HIDDEN_DIM 128
#define TENSOR_MAX_ATOMS 4096
#define TENSOR_MAX_RULES 512
#define TENSOR_MAX_BATCH 32

/*
 * Tensor Truth Value - Continuous representation of truth
 * Allows gradient-based learning of logical relationships
 */
struct TensorTruthValue {
    float strength;      /* Truth strength [0,1] */
    float confidence;    /* Confidence in strength [0,1] */
    float evidence;      /* Evidence count */
    float embedding[TENSOR_EMBED_DIM];  /* Neural embedding */
    float gradient[TENSOR_EMBED_DIM];   /* Gradient for learning */
};

/*
 * Tensor Atom - Neural-symbolic knowledge unit
 * Combines symbolic name with neural embedding
 */
struct TensorAtom {
    unsigned long id;
    int type;
    char *name;
    TensorTruthValue tv;
    float embedding[TENSOR_EMBED_DIM];  /* Learned representation */
    float attention_weight;              /* Attention score */
    TensorAtom **outgoing;               /* Outgoing links */
    int arity;
    TensorAtom *next;                    /* Hash chain */
};

/*
 * Tensor AtomSpace - Neural-symbolic knowledge base
 * Supports differentiable operations on atoms
 */
struct TensorAtomSpace {
    TensorAtom **atoms;
    int natoms;
    int maxatoms;
    unsigned long next_id;

    /* Neural components */
    float *atom_embeddings;     /* [maxatoms x TENSOR_EMBED_DIM] */
    float *relation_matrix;     /* [maxatoms x maxatoms] learned relations */
    float *attention_scores;    /* [maxatoms] current attention */

    /* Learning state */
    float learning_rate;
    float momentum;
    int training_steps;
};

/*
 * Tensor Rule - Differentiable inference rule
 * Allows learning of rule weights through backpropagation
 */
struct TensorRule {
    unsigned long id;
    char *name;
    float weight;                        /* Learned rule weight */
    float confidence;                    /* Rule confidence */

    /* Rule structure */
    TensorAtom **premises;               /* Premise atoms */
    int num_premises;
    TensorAtom *conclusion;              /* Conclusion atom */

    /* Neural components */
    float premise_weights[16];           /* Max 16 premises */
    float hidden_state[TENSOR_HIDDEN_DIM];
    float gradient[TENSOR_HIDDEN_DIM];

    TensorRule *next;
};

/*
 * Tensor Inference - Single inference step result
 */
struct TensorInference {
    TensorRule *rule;
    TensorAtom *conclusion;
    float confidence;
    float *attention_pattern;  /* Which atoms were attended to */
    TensorInference *next;
};

/*
 * Gradient Context - For backpropagation through reasoning
 */
struct GradientContext {
    float *gradients;          /* Accumulated gradients */
    int grad_size;
    float loss;
    int num_steps;

    /* Adam optimizer state */
    float *m;                  /* First moment */
    float *v;                  /* Second moment */
    float beta1;
    float beta2;
    float epsilon;
};

/*
 * Tensor Logic Engine - Main reasoning engine
 */
struct TensorLogicEngine {
    TensorAtomSpace *atomspace;
    TensorRule *rules;
    int num_rules;

    /* Inference state */
    TensorInference *inference_chain;
    int inference_depth;
    int max_depth;

    /* Neural network weights */
    float *query_weights;      /* [TENSOR_EMBED_DIM x TENSOR_HIDDEN_DIM] */
    float *key_weights;        /* [TENSOR_EMBED_DIM x TENSOR_HIDDEN_DIM] */
    float *value_weights;      /* [TENSOR_EMBED_DIM x TENSOR_HIDDEN_DIM] */
    float *output_weights;     /* [TENSOR_HIDDEN_DIM x TENSOR_EMBED_DIM] */

    /* Learning state */
    GradientContext *grad_ctx;
    float temperature;         /* Softmax temperature for attention */
    int training_mode;
};

/* Activation functions */
float tensor_sigmoid(float x);
float tensor_tanh(float x);
float tensor_relu(float x);
float tensor_softmax_single(float x, float *values, int n);

/* Truth value operations */
TensorTruthValue* tensor_tv_create(float strength, float confidence);
TensorTruthValue* tensor_tv_merge(TensorTruthValue *tv1, TensorTruthValue *tv2);
TensorTruthValue* tensor_tv_revision(TensorTruthValue *tv1, TensorTruthValue *tv2);
TensorTruthValue* tensor_tv_deduction(TensorTruthValue *tv1, TensorTruthValue *tv2);
TensorTruthValue* tensor_tv_induction(TensorTruthValue *tv1, TensorTruthValue *tv2);
TensorTruthValue* tensor_tv_abduction(TensorTruthValue *tv1, TensorTruthValue *tv2);
void tensor_tv_destroy(TensorTruthValue *tv);

/* Tensor atom operations */
TensorAtom* tensor_atom_create(TensorAtomSpace *as, int type, char *name, TensorTruthValue *tv);
TensorAtom* tensor_atom_find(TensorAtomSpace *as, char *name);
TensorAtom* tensor_atom_find_by_id(TensorAtomSpace *as, unsigned long id);
void tensor_atom_add_link(TensorAtom *atom, TensorAtom *target);
float tensor_atom_similarity(TensorAtom *a1, TensorAtom *a2);
int tensor_atom_unify(TensorAtom *pattern, TensorAtom *target, TensorAtom ***bindings);
void tensor_atom_update_embedding(TensorAtom *atom, float *new_embedding);
void tensor_atom_destroy(TensorAtom *atom);

/* Tensor atomspace operations */
TensorAtomSpace* tensor_atomspace_create(int maxatoms);
void tensor_atomspace_destroy(TensorAtomSpace *as);
void tensor_atomspace_update_embeddings(TensorAtomSpace *as);
void tensor_atomspace_compute_attention(TensorAtomSpace *as, float *query);
TensorAtom** tensor_atomspace_get_top_k(TensorAtomSpace *as, int k);

/* Tensor rule operations */
TensorRule* tensor_rule_create(char *name, TensorAtom **premises, int num_premises, TensorAtom *conclusion);
void tensor_rule_destroy(TensorRule *rule);
float tensor_rule_apply(TensorRule *rule, TensorAtomSpace *as);
void tensor_rule_update_weights(TensorRule *rule, GradientContext *ctx);

/* Tensor logic engine operations */
TensorLogicEngine* tensor_logic_create(TensorAtomSpace *as);
void tensor_logic_destroy(TensorLogicEngine *engine);
void tensor_logic_add_rule(TensorLogicEngine *engine, TensorRule *rule);
TensorInference* tensor_logic_infer(TensorLogicEngine *engine, TensorAtom *query, int max_steps);
void tensor_logic_train_step(TensorLogicEngine *engine, TensorAtom *query, TensorTruthValue *target);
void tensor_logic_backward(TensorLogicEngine *engine);

/* Gradient operations */
GradientContext* gradient_context_create(int size);
void gradient_context_destroy(GradientContext *ctx);
void gradient_context_zero(GradientContext *ctx);
void gradient_context_accumulate(GradientContext *ctx, float *grads, int size);
void gradient_context_apply_adam(GradientContext *ctx, float *weights, int size);

/* Attention mechanism */
void tensor_attention_forward(TensorLogicEngine *engine, TensorAtom **atoms, int n, float *output);
void tensor_attention_backward(TensorLogicEngine *engine, float *grad_output, TensorAtom **atoms, int n);

/* Distributed operations */
void tensor_logic_sync(TensorLogicEngine *engine, int node_id);
void tensor_logic_aggregate(TensorLogicEngine **engines, int num_engines, TensorLogicEngine *output);

/* Cognitive integration */
void tensor_cognitive_update_full(TensorLogicEngine *engine, float *cognitive_state, int state_size);
void tensor_cognitive_goal_gradient(TensorLogicEngine *engine, float *goal_embedding, float *gradient);

#endif /* TENSOR_LOGIC_H */
