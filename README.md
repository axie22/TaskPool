# TaskPool

## Overview

A solid “thread pool + task scheduler” is basically 3 layers:

1. **A queue of work** (tasks)
2. **A set of worker threads** that repeatedly pop work and execute it
3. **A scheduling API** that lets you submit tasks, get results back, and optionally express dependencies/priorities

### Task

A task is “some callable to run later” + metadata:

- priority
- affinity (which worker preferred)
- deadline / delay
- dependencies (run after X finishes)

In practice you store tasks as something like:

- `std::function<void()>` (simple, slower / allocates sometimes)
- a custom **type-erased** callable (faster, more complex)
- a “packaged task” that can fulfill a future

### Thread pool

- Create `N` threads.
- Each thread loops:

  - wait for work
  - pop a task
  - run it

- Shutdown cleanly (stop flag + wake all workers + join).

### Futures / results

When you submit a task that returns `T`, you want `std::future<T>` back:

- Wrap the callable in `std::packaged_task<T()>`
- Store it as `void()` in the queue by capturing the packaged_task in a lambda
- Return the future to the caller

---

## Minimal architecture (good first version)

### Data structures

- `std::deque<Task>` global queue
- `std::mutex` + `std::condition_variable`
- `std::atomic<bool> stop`

### API surface

- `submit(f)` → returns `future<R>`
- `submit_bulk(range, f)` (optional)
- `wait_for_idle()` (optional)
- destructor stops and joins

### Worker loop (logic)

1. lock
2. wait until `stop || !queue.empty()`
3. if `stop && queue.empty()`: exit
4. pop task
5. unlock
6. run task

---

## “Scheduler” layer

A thread pool alone runs tasks ASAP. A **scheduler** decides _which_ task runs _when_. Add features one by one:

### Feature A: priorities

Use a `std::priority_queue` instead of deque.

- Each task has `priority` (higher wins).
- Workers pop highest priority.

Tradeoff: priority_queue is slower than deque but fine.

### Feature B: delayed tasks (timers)

Support `schedule_after(50ms, f)`.

- Keep a min-heap ordered by `run_at` time.
- Workers check:

  - if ready tasks exist → run them
  - else wait until next timer fires

This teaches:

- `std::chrono`
- time-based waiting (`cv.wait_until`)

### Feature C: work stealing (big performance jump)

Instead of one global queue:

- each worker has a **local deque**
- workers push new tasks into their own deque
- workers pop from their own deque (fast, no contention)
- if empty, they **steal** from the back of another worker’s deque

This teaches:

- contention reduction
- deques + locking strategy
- scaling behavior

Implementation detail:

- Local deque protected by a small mutex
- Steal from other worker’s back (common strategy)

### Feature D: dependencies (DAG execution)

Let tasks say “run after these tasks finish”.
Model:

- each task has an atomic counter `remaining_deps`
- when a task finishes, it decrements counters of dependents
- any dependent that hits 0 becomes runnable (enqueue)

This teaches:

- atomic counters
- graph bookkeeping
- correctness under concurrency

---

## Build plan

### Milestone 1: Correct basic pool (1–2 days)

- `submit()` returning futures
- clean shutdown
- exceptions propagate through futures

Tests:

- run 1000 tasks, verify all futures correct
- submit tasks that throw, ensure future.get throws

### Milestone 2: wait_for_idle + instrumentation (1 day)

- `active_tasks` atomic counter
- `wait_for_idle()` waits until queue empty and active==0
- metrics: tasks executed / steals / average queue length

### Milestone 3: priorities OR timers (1–2 days)

Pick one:

- priorities: easiest scheduler feature
- timers: most “scheduler-like”

### Milestone 4: work stealing (2–4 days)

- per-worker deque
- stealing logic
- benchmark scaling with N threads

### Milestone 5: dependencies (optional, 3–6 days)

- DAG scheduling
- “when_all” helper that returns a future when a set completes

---

## Common pitfalls (the stuff you’ll _actually_ learn from)

- **Shutdown deadlocks**: forgetting to `notify_all()` when stopping
- **Lost wakeups**: wrong predicate in `cv.wait(lock, pred)`
- **Tasks capturing references** that dangle (lifetime bugs)
- **Exception handling**: if a task throws and you don’t contain it, you can crash a worker thread
- **False sharing**: counters on same cache line cause slowdowns
- **Priority inversion**: high priority tasks blocked by dependency chain

---

## What I’d implement first (simple but “real”)

If you want something portfolio-worthy without going insane:

- Thread pool with `submit() -> future`
- Per-thread local deques + work stealing
- Optional priorities (just to demonstrate scheduler control)
- A tiny benchmark + README explaining design tradeoffs

---

If you want, I can sketch a concrete class layout (headers, key methods, and the exact internal state you’ll store) in a way that’s idiomatic modern C++ (C++20-ish) and avoids the usual concurrency footguns.
