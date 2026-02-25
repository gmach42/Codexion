# Codexion

*Created as part of the 42 curriculum by gmach.*

A concurrency simulation inspired by the Dining Philosophers problem, reimagined in a developer context. Coders sit in a circle, each needing two USB **dongles** (one on their left, one on their right) to **compile**. After compiling, they **debug** and **refactor** before attempting to compile again. A coder who waits too long without compiling **burns out**, ending the simulation.

---

## Table of Contents

- [Description](#description)
- [Coder Lifecycle](#coder-lifecycle)
- [Concurrency & Synchronization](#concurrency--synchronization)
- [Arguments](#arguments)
- [Build & Usage](#build--usage)
- [Debugging](#debugging)
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

## Concurrency & Synchronization

### Mutexes

Each dongle has its own `pthread_mutex_t` that serialises access to its state. Additional simulation-wide mutexes protect:

| Mutex | Purpose |
|---|---|
| `stop_mutex` | Thread-safe read/write of the global stop flag |
| `print_mutex` | Serialised timestamped log output |
| `compile_mutex` | Atomic increment of the global compile counter |
| `time_mutex` (per coder) | Thread-safe update of `last_compile_time` |

### Condition Variables

Each dongle has a `pthread_cond_t` used to signal waiting coders when the dongle becomes available again after its cooldown. This avoids busy-waiting.

### Blocking Cases Handled

| Scenario | Mitigation |
|---|---|
| **Deadlock** | Coders with an even ID pick up their right dongle first, breaking the circular wait (Coffman condition). |
| **Starvation** | The `edf` scheduler prioritises the coder closest to burnout, ensuring fairness. |
| **Dongle cooldown** | `pthread_cond_timedwait` blocks the caller until the cooldown expires instead of spinning. |
| **Burnout detection** | The monitor thread polls `last_compile_time` every 1 ms with a dedicated mutex. |

### Scheduler Policies

The `scheduler` argument selects the arbitration policy used when multiple coders compete for the same dongle:

- **`fifo`** — First In, First Out. The dongle is granted to the coder whose request arrived earliest.
- **`edf`** — Earliest Deadline First. The dongle is granted to the coder whose deadline (`last_compile_start + time_to_burnout`) is nearest, minimising burnout risk.

---

## Arguments

```
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug> <time_to_refactor> <number_of_compiles_required> \
           <dongle_cooldown> <scheduler>
```

| Argument | Unit | Description |
|---|---|---|
| `number_of_coders` | — | Number of coders **and** dongles in the simulation. |
| `time_to_burnout` | ms | A coder burns out if they have not started compiling within this delay since their last compile (or simulation start). |
| `time_to_compile` | ms | Duration of the compile phase. The coder must hold both dongles for this entire time. |
| `time_to_debug` | ms | Duration of the debug phase (no dongles required). |
| `time_to_refactor` | ms | Duration of the refactor phase. Once done, the coder immediately attempts to acquire dongles again. |
| `number_of_compiles_required` | — | Simulation stops successfully when every coder has compiled at least this many times. |
| `dongle_cooldown` | ms | After being released, a dongle is unavailable for this duration. |
| `scheduler` | — | Dongle arbitration policy: `fifo` or `edf`. |

### Example

```bash
./codexion 4 600 300 100 100 10 100 fifo
```

---

## Build & Usage

```bash
# Standard build
make

# Build and run with default test arguments
make t

# Build with debug symbols and run
make debug

# Memory check with Valgrind
make valgrind

# Clean object files
make clean

# Full clean (objects + binary)
make fclean

# Rebuild from scratch
make re
```

---

## Debugging

Exemple of gdb usage to debug the monitor thread:

```gdb
(gdb) --args ./codexion 4 600 300 100 100 10 100 fifo
(gdb) break monitor_routine
(gdb) run
(gdb) info threads
(gdb) thread 3
(gdb) backtrace
```

---

## Resources

- [Multithreading in C — GeeksforGeeks](https://www.geeksforgeeks.org/c/multithreading-in-c/)
- [Mutex Lock for Linux Thread Synchronization — GeeksforGeeks](https://www.geeksforgeeks.org/linux-unix/mutex-lock-for-linux-thread-synchronization/)
- [Thread Functions in C — GeeksforGeeks](https://www.geeksforgeeks.org/c/thread-functions-in-c-c/)
- [Condition Wait and Signal in Multi-threading — GeeksforGeeks](https://www.geeksforgeeks.org/linux-unix/condition-wait-signal-multi-threading/)
