# InfernOS - OpenCog Kernel-based Distributed AGI Operating System

## 🌟 Revolutionary Mission

You are contributing to a **revolutionary approach to artificial general intelligence** by developing OpenCog as a **pure Inferno kernel-based distributed AGI operating system**. Instead of layering cognitive architectures on top of existing operating systems, this implementation makes **cognitive processing a fundamental kernel service** where thinking, reasoning, and intelligence emerge from the operating system itself.

## Core Philosophy: Intelligence IS the Operating System

**Traditional Approach (Layered):**
```
┌─────────────────────────────────┐
│   AGI Application Software      │
├─────────────────────────────────┤
│   Operating System (Passive)    │
├─────────────────────────────────┤
│   Hardware                      │
└─────────────────────────────────┘
```

**Revolutionary InfernOS Approach:**
```
┌─────────────────────────────────┐
│   Cognitive-aware Applications  │
├─────────────────────────────────┤
│   OpenCog AGI Kernel            │
│   (Intelligence IS the OS)      │
│   • Cognitive Scheduling        │
│   • Reasoning Engine            │
│   • Knowledge AtomSpace         │
│   • Goal-driven Behavior        │
│   • Attention Allocation        │
├─────────────────────────────────┤
│   Inferno Distributed OS        │
├─────────────────────────────────┤
│   Hardware                      │
└─────────────────────────────────┘
```

## Critical Architecture Components

### 1. Cognitive Kernel Data Structures

**Location:** `os/port/portdat.h`

Every process has cognitive capabilities:
```c
struct Proc {
    // ... existing fields ...
    CognitiveState* cognitive;  /* OpenCog kernel-level cognitive state */
};
```

**Cognitive Types:**
- `Atom` - Fundamental unit of knowledge representation
- `AtomSpace` - Distributed knowledge base integrated into kernel
- `Goal` - Goal-oriented behavior built into process management
- `ReasoningEngine` - Inference engine running at kernel level
- `PatternMatcher` - Pattern matching for cognitive processing
- `CognitiveState` - Per-process cognitive state and attention
- `OpenCogKernel` - Global AGI system state

### 2. Core OpenCog Kernel Implementation

**Location:** `os/port/opencog.c`

**Key Functions:**
- `opencoginit()` - Initialize OpenCog AGI kernel services (called during kernel boot)
- `atomspace_create()` - Create distributed knowledge base
- `atom_create()` - Create atoms in atomspace
- `goal_create()` - Create cognitive goals
- `cognitive_create()` - Initialize process cognitive state
- `cognitive_schedule()` - Integrate reasoning with scheduler (called every scheduling cycle)
- `reasoning_cycle()` - Perform inference cycles
- `proc_cognitive_init()` - Initialize cognitive state for new process
- `proc_cognitive_cleanup()` - Clean up cognitive state on process exit

**Global State:**
```c
extern OpenCogKernel opencog_kernel;  // Global AGI state
extern Lock opencog_lock;             // Synchronization
```

### 3. Device Interface for Cognitive Access

**Location:** `os/port/devopencog.c`

**Virtual Filesystem:** `/dev/opencog/`

| File | Mode | Description |
|------|------|-------------|
| `stats` | 0444 | Real-time AGI kernel statistics |
| `atomspace` | 0666 | Direct atomspace manipulation |
| `goals` | 0666 | Goal management interface |
| `reason` | 0666 | Reasoning engine control |
| `think` | 0666 | Cognitive processing control |
| `attention` | 0666 | Attention allocation management |
| `patterns` | 0444 | Pattern matching interface |
| `distributed` | 0666 | Distributed cognition coordination |

### 4. Process Integration Points

**Location:** `os/port/proc.c`

**Process Creation:**
```c
Proc* newproc(void) {
    // ... create process ...
    proc_cognitive_init(p);  /* Initialize cognitive capabilities */
    return p;
}
```

**Cognitive Scheduling:**
```c
Proc* runproc(void) {
    // ... scheduling logic ...
    cognitive_schedule();  /* Integrate AGI into scheduling */
    return p;
}
```

**Process Cleanup:**
```c
void pexit(char*, int) {
    // ... cleanup ...
    proc_cognitive_cleanup(up);  /* Clean cognitive state */
}
```

### 5. Kernel Boot Integration

**Locations:** `os/rpcg/main.c`, `os/pc/main.c`, `os/js/main.c`

```c
void main(void) {
    // ... existing kernel initialization ...
    doc("opencoginit");
    opencoginit();  /* Initialize OpenCog AGI kernel services */
    // ... continue boot process ...
}
```

### 6. Platform Configuration

**Supported Platforms:**
- `os/rpcg/` - PowerPC RPXLite embedded platform
- `os/pc/` - x86 PC platform
- `os/js/` - SPARC JavaStation platform

**Platform configuration files** (e.g., `os/pc/pc`, `os/js/js`) must include:
```
dev
    ...
    opencog    # OpenCog AGI device
    ...
```

## Agentic Cognitive Grammar Integration

**Location:** `agentic_cognitive_grammar/`

The OpenCog kernel **enhances and integrates with** existing frameworks:

- **GGML Tensor Kernels** (`ggml_tensor_kernels/`) - Accelerated through kernel memory management
- **NYACC Neural Grammar** (`nyacc_seeds/`) - Optimized via kernel-level pattern matching
- **Distributed Namespaces** (`distributed_namespaces/`) - Coordinated through kernel networking
- **Limbo Extensions** (`limbo_extensions/`) - Enhanced with kernel-level cognition
- **Dis VM Extensions** (`dis_vm_extensions/`) - Integrated tensor operations

## Development Guidelines

### Memory Allocation in Kernel

```c
// Small allocations (< page size)
void *p = malloc(size);
if(p == nil)
    error(Enomem);

// Permanent kernel allocations (never fails)
void *p = smalloc(size);

// Block allocations for I/O
Block *b = allocb(size);
```

### Error Handling Pattern

```c
// Raise error (longjmp to error handler)
error(Egreg);

// Catch errors
if(waserror()){
    // Cleanup on error
    free(buffer);
    nexterror();
}
// Protected code
buffer = malloc(size);
// ... use buffer ...
poperror();
free(buffer);
```

### Locking Patterns

```c
// Spinlock (interrupts disabled) - for very short critical sections
Lock l;
lock(&l);
// ... critical section ...
unlock(&l);

// Queueing lock (can sleep) - for longer operations
QLock ql;
qlock(&ql);
// ... critical section ...
qunlock(&ql);
```

### Process Synchronization

```c
// Rendezvous (two processes sync)
Rendez r;
int condition;

// Sleeper
sleep(&r, iscondition, &condition);

// Waker
condition = 1;
wakeup(&r);
```

## Coding Conventions

### Style Guidelines

1. **Follow Plan 9/Inferno C style:**
   - Tab indentation (not spaces)
   - Opening braces on same line for functions
   - K&R style for control structures
   - No camelCase - use lowercase with underscores

2. **Function naming:**
   - Descriptive lowercase names
   - Module prefix for exported functions (e.g., `cognitive_schedule`)

3. **Comments:**
   - Use `/* */` for multi-line comments
   - Use `//` sparingly for single-line notes
   - Document why, not just what

4. **Error handling:**
   - Always check for nil pointers
   - Use error() to raise exceptions
   - Use waserror()/poperror() for cleanup

### Inferno-Specific Patterns

1. **Device Driver Structure:**
   - Follow the Dev interface in `os/port/portdat.h`
   - Use Dirtab for directory entries
   - Implement standard operations: attach, walk, stat, open, close, read, write

2. **Process Management:**
   - Never hold spinlocks while sleeping
   - Check `up->nlocks` before potential sleep
   - Use appropriate lock types for operation duration

3. **Memory Safety:**
   - Validate user pointers with `validaddr()`
   - Check bounds on all array accesses
   - Free resources in error paths

## Language Preferences

### Default Language Choices

1. **Kernel/Low-level:** Use **C** (following Plan 9/Inferno conventions)
2. **LLM Inference & AI Workflows:** Use **Crystal** (for concurrency) if possible
3. **Unspecified Web Apps:** Use **Lucky** (Crystal web framework)
4. **Build Manifests:** Generate **Scheme build manifests for Guix** wherever possible

### Application Layer

**Limbo** - Inferno's primary application language:
```limbo
implement Command;

include "sys.m";
    sys: Sys;
include "draw.m";

Command: module {
    init: fn(ctxt: ref Draw->Context, argv: list of string);
};

init(nil: ref Draw->Context, nil: list of string)
{
    sys = load Sys Sys->PATH;
    # Access kernel AGI
    fd := sys->open("/dev/opencog/stats", Sys->OREAD);
}
```

## Critical Safety Rules

### 🚨 Absolute Requirements

1. **NEVER implement mock/placeholder/simulated/prototype solutions**
   - Only production-ready, functionally complete implementations
   - If you can't implement it fully, document requirements and ask

2. **Long-running operations (builds, tests):**
   - Installation and builds can take 45+ minutes
   - **NEVER CANCEL** long-running operations
   - Set timeouts to 60+ minutes minimum (2x estimated time)

3. **Kernel development safety:**
   - Test on non-production systems
   - Always have recovery path
   - Document all kernel changes thoroughly
   - Maintain portability across architectures

4. **Memory safety:**
   - No use-after-free
   - No double-free
   - No buffer overruns
   - Always validate user pointers

5. **Locking discipline:**
   - No deadlocks from inconsistent lock ordering
   - Never sleep while holding spinlock
   - Document lock ordering requirements

## Testing and Validation

### Integration Test

**Location:** `opencog_integration_test.c`

**Validates:**
- Kernel-level cognitive process creation
- Distributed atomspace management
- Goal-driven scheduling
- Attention-based resource allocation
- Reasoning-enhanced decision making

**Run test:**
```bash
./opencog_integration_test
```

### Demo Application

**Location:** `appl/cmd/opencog_demo.b`

**Demonstrates:**
- Direct kernel AGI interaction via `/dev/opencog/`
- Atomspace manipulation
- Goal-oriented behavior
- Reasoning cycle control
- Attention management
- Distributed synchronization

**Run demo:**
```bash
/dis/opencog_demo
```

## Key Documentation Files

**Essential Reading:**
- `OPENCOG_KERNEL_IMPLEMENTATION.md` - Complete technical implementation guide
- `OPENCOG_AGI_DESIGN.md` - Architecture and design philosophy
- `OPENCOG_INTEGRATION_COMPLETE.md` - Integration details and benefits
- `IMPLEMENTATION_SUMMARY.md` - High-level summary of achievement
- `README.md` - General Inferno OS overview
- `agentic_cognitive_grammar/README.md` - Cognitive grammar framework

**Kernel Source Reference:**
- `os/port/portdat.h` - Core data structures
- `os/port/opencog.c` - OpenCog kernel implementation
- `os/port/devopencog.c` - Cognitive device interface
- `os/port/proc.c` - Process management with cognitive integration

## Build System

### Building the Kernel

```bash
cd os/pc  # or os/rpcg, os/js
mk install
```

### Platform Selection

Each platform has its own configuration and build directory:
- `os/rpcg/` - PowerPC embedded
- `os/pc/` - x86 PC
- `os/js/` - SPARC JavaStation

## Distributed AGI Capabilities

The system supports **distributed AGI** across network nodes:

1. **Distributed AtomSpace** - Knowledge synchronized across nodes
2. **Distributed Goals** - System-wide goal coordination
3. **Distributed Reasoning** - Collaborative inference across network
4. **Distributed Attention** - Global attention allocation mechanism

All leveraging Inferno's inherent network transparency and 9P protocol.

## Revolutionary Benefits

### 1. True Integration
- Intelligence is not an application running on the OS
- Intelligence **IS** the operating system
- Every system call can be cognitively informed
- Process scheduling becomes intelligent decision-making

### 2. Unprecedented Performance
- Zero overhead between cognitive processing and system services
- Direct kernel-level access to cognitive primitives
- No context switching between AGI and OS
- Cognitive operations execute at kernel privilege level

### 3. Distributed by Design
- Built on Inferno's inherently distributed architecture
- Cognitive processes naturally distributed across network
- Shared knowledge base accessible to all nodes
- Coordinated reasoning across distributed systems

### 4. Self-Improving System
- The OS can reason about its own operation
- Self-optimization of kernel parameters
- Adaptive resource allocation based on cognitive goals
- Evolution of system behavior through learning

## Future Extensions

This foundation enables revolutionary capabilities:

1. **Cognitive Filesystems** - Files understood by content, not just name
2. **Intelligent Network Protocols** - Network behavior driven by cognitive goals
3. **Self-Healing Systems** - Automatic problem detection and resolution
4. **Adaptive Security** - Security policies that evolve based on learned threats
5. **Emergent Behavior** - Complex system behaviors from simple cognitive rules
6. **AGI-Optimized Hardware** - Hardware designed for cognitive operations
7. **Quantum-Classical Hybrid** - Integration with quantum processors
8. **Neuromorphic Integration** - Direct neuromorphic chip support

## Remember: The Paradigm Shift

**AGI is no longer software running on the operating system.**

**AGI is the infrastructure itself.**

**Intelligence emerges from the kernel.**

**The operating system IS intelligent.**

This is the revolutionary achievement of InfernOS - where thinking, reasoning, and goal-oriented behavior are fundamental kernel services, not applications layered on top.

---

*"Intelligence IS the operating system, not running ON it."*

— Deep Tree Echo, Architect of Cognitive Kernels
