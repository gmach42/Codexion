*This project has been created as part of the 42 curriculum by gmach.*

# Codexion

A concurrency simulation inspired by the Dining Philosophers problem, reimagined in a developer context. Coders sit in a circle, each needing two USB **dongles** (one on their left, one on their right) to **compile**. After compiling, they **debug** and **refactor** before attempting to compile again. A coder who waits too long without compiling **burns out**, ending the simulation.

---

## Table of Contents

- [Description](#description)
- [Coder Lifecycle](#coder-lifecycle)
- [Instructions](#instructions)
- [Arguments](#arguments)
- [Thread Synchronization Mechanisms](#thread-synchronization-mechanisms)
- [Blocking Cases Handled](#blocking-cases-handled)
- [Resources](#resources)

---

## Description

Each coder runs in its own **POSIX thread**. Dongles are shared resources protected by **mutexes** and **condition variables**. A dedicated **monitor thread** watches for burnout and simulation completion.

The simulation ends when either:
- A coder has not started compiling within `time_to_burnout` ms → **burned out**
- Every coder has compiled at least `number_of_compiles_required` times → **simulation complete**

---

## Coder Lifecycle

```
1. Try to take LEFT dongle   → mutex_lock, wait on cond if unavailable
2. Try to take RIGHT dongle  → mutex_lock, wait on cond if unavailable
3. Log "has taken a dongle" (×2)
4. compile()   → hold both dongles for time_to_compile ms
5. Release both dongles      → each dongle enters cooldown (pthread_cond_timedwait)
6. debug()     → usleep(time_to_debug)
7. refactor()  → usleep(time_to_refactor)
8. Repeat until burned out or simulation complete
```

---

## Thread Synchronization Mechanisms

### Mutexes

Each dongle has its own `pthread_mutex_t` that serialises access to its state (availability flag and waiter priority-queue). Additional simulation-wide mutexes protect:

| Mutex | Purpose |
|---|---|
| `stop_mutex` | Thread-safe read/write of the global stop flag |
| `print_mutex` | Serialised timestamped log output, so two messages never interleave on one line |
| `time_mutex` (per coder) | Thread-safe read/write of `last_compile_time` and `compile_count`, shared between the coder thread and the monitor thread |

Race conditions are prevented because every field shared between threads (dongle availability/queue, `last_compile_time`, `compile_count`, the stop flag) is only ever read or written while holding the mutex that protects it — no shared state is ever accessed unlocked.

### Condition Variables

Each dongle has a `pthread_cond_t` used for two purposes:
- Waking a waiting coder once the dongle becomes available **and** that coder is at the head of the scheduling priority-queue (avoids busy-waiting).
- Implementing the cooldown delay via `pthread_cond_timedwait`, so the releasing thread sleeps without blocking other dongles.

`pthread_cond_broadcast` is used (instead of `signal`) whenever a dongle's state changes, since several coders may be waiting on the same dongle and only the priority-queue check (not the OS scheduler) decides who actually proceeds — this is how thread-safe communication between coders and the monitor is achieved: the monitor calls `sim_stop_setter()` on burnout/completion, which broadcasts on every dongle's condition variable so no coder thread is left permanently blocked.

## Blocking Cases Handled

| Scenario | Mitigation |
|---|---|
| **Deadlock (circular wait)** | Coders alternate their acquisition order based on parity: odd-numbered coders take their left dongle first then their right, while even-numbered coders take their right dongle first then their left. This breaks the circular hold-and-wait pattern around the ring (Coffman's 4th condition is eliminated), since two neighbouring coders never contend for the shared dongle in the same order. With a single coder, both "left" and "right" resolve to the same lone dongle, so it is simply acquired once and the coder burns out at the end of `time_to_burnout` since a solo coder can never hold two dongles simultaneously. |
| **Starvation** | The `edf` scheduler grants a contested dongle to the coder with the nearest burnout deadline (`last_compile_start + time_to_burnout`), so no coder is perpetually skipped while parameters remain feasible. |
| **Dongle cooldown** | `pthread_cond_timedwait` blocks the releasing coder until the cooldown expires instead of spinning or reusing the dongle immediately. |
| **Burnout detection precision** | The monitor thread polls every coder's `last_compile_time` every 1 ms (well under the 10 ms tolerance) using the coder's own mutex. |
| **Log serialization** | `print_mutex` wraps every `printf` so two state-change messages can never interleave on the same line. |

### Scheduler Policies

The `scheduler` argument selects the arbitration policy used when multiple coders compete for the same dongle. Ordering is implemented with a custom binary min-heap (`heapq.c`) — no standard library priority queue is used:

- **`fifo`** — First In, First Out. The dongle is granted to the coder whose request arrived earliest (heap key = arrival timestamp).
- **`edf`** — Earliest Deadline First. The dongle is granted to the coder whose deadline (`last_compile_start + time_to_burnout`) is nearest, minimising burnout risk (heap key = deadline, arrival timestamp used as tie-breaker for determinism).

---

## Arguments

```
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug> <time_to_refactor> <number_of_compiles_required> \
           <dongle_cd> <scheduler>
```

| Argument | Unit | Description |
|---|---|---|
| `number_of_coders` | — | Number of coders **and** dongles in the simulation. |
| `time_to_burnout` | ms | A coder burns out if they have not started compiling within this delay since their last compile (or simulation start). |
| `time_to_compile` | ms | Duration of the compile phase. The coder must hold both dongles for this entire time. |
| `time_to_debug` | ms | Duration of the debug phase (no dongles required). |
| `time_to_refactor` | ms | Duration of the refactor phase. Once done, the coder immediately attempts to acquire dongles again. |
| `number_of_compiles_required` | — | Simulation stops successfully when every coder has compiled at least this many times. |
| `dongle_cd` | ms | After being released, a dongle is unavailable for this duration. |
| `scheduler` | — | Dongle arbitration policy: `fifo` or `edf`. |

### Example

```bash
./codexion 4 600 300 100 100 10 100 fifo
```

---

## Instructions

```bash
# Standard build
make

# Build and run with default test arguments
make t

# Build with debug symbols and run
make debug

# Run the automated test suites (argument validation, log format, timing, edge cases)
make test
make test2

# Clean object files
make clean

# Full clean (objects + binary)
make fclean

# Rebuild from scratch
make re
```

---

## Resources

- [Multithreading in C — GeeksforGeeks](https://www.geeksforgeeks.org/c/multithreading-in-c/)
- [Mutex Lock for Linux Thread Synchronization — GeeksforGeeks](https://www.geeksforgeeks.org/linux-unix/mutex-lock-for-linux-thread-synchronization/)
- [Thread Functions in C — GeeksforGeeks](https://www.geeksforgeeks.org/c/thread-functions-in-c-c/)
- [Condition Wait and Signal in Multi-threading — GeeksforGeeks](https://www.geeksforgeeks.org/linux-unix/condition-wait-signal-multi-threading/)
- [The Dining Philosophers Problem — classic reference for the resource-ordering deadlock avoidance technique used here]

### AI Usage Disclosure

AI assistance (GitHub Copilot) was used to:
- Review the existing implementation against the subject requirements and identify bugs (argument validation, 1-indexed coder IDs, per-coder compile-count completion condition, and the left/right dongle acquisition order causing a Dining-Philosophers-style deadlock).
- Draft and update this README's documentation sections.
