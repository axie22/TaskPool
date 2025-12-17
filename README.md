# ThreadPool

A modern C++ thread pool implementation built from scratch to explore concurrency synchronization, and scheduling fundamentals.

## Features

- Fixed-size worker thread pool
- Generic `submit()` API supporting arbitrary callables and arguments
- `std::future`-based result retrieval with exception propagation
- Graceful shutdown with clean worker termination
- Accurate `wait_for_idle()` to block until all work is completed
- Instrumentation via an atomic active-task counter
- Benchmarking harness for scalability experiments

### Core Components

- **Task Queue**
  A `std::queue<std::function<void()>>` holding work items.

- **Worker Threads**
  A fixed number of threads that:

  1. Wait on a condition variable
  2. Pop tasks from the queue
  3. Execute tasks outside the lock
  4. Track active execution state

- **Active Task Tracking**
  An `std::atomic<size_t>` tracks how many tasks are currently executing.
  This allows the pool to distinguish between:

  - no queued work
  - work currently in progress

- **RAII Guard (`ActiveGuard`)**
  Ensures `active_tasks` is incremented before task execution and decremented afterward — even if the task throws.
  Results typically show:

- Near-linear scaling up to available CPU cores
- Diminishing returns past hardware concurrency
- Performance degradation with excessive thread counts

---

## Build Instructions

### Requirements

- C++17 compatible compiler (clang++ or g++)
- POSIX threads

### Build with Make

```bash
make
```

### Run

```bash
./main
```

### Clean

```bash
make clean
```

---

## File Structure

```bash
.
├── threadpool.h      # ThreadPool interface + template submit()
├── threadpool.cpp    # Worker loop, shutdown, wait_for_idle
├── benchmarks.h      # Benchmark declarations
├── benchmarks.cpp    # Benchmark implementations
├── main.cpp          # Entry point
├── Makefile
└── README.md
```
