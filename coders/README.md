*This project has been created as part of the 42 curriculum by ldzuba.*

# Codexion

## Description

Codexion is a concurrency simulation inspired by the classic **Dining Philosophers** problem, reframed around coders sharing USB dongles to compile quantum code.

One or more coders sit in a circular co-working hub around a shared Quantum Compiler. Each coder alternates between three states: **compiling** (requires two dongles simultaneously), **debugging**, and **refactoring**. There are as many dongles as coders, one between each adjacent pair. If a coder goes too long without compiling, they burn out — and the simulation ends.

The goal is to implement a correct, fair, and deadlock-free simulation using POSIX threads and mutexes, with two scheduling policies (FIFO and EDF) for dongle arbitration.

Key challenges:
- Preventing deadlock in a circular resource topology
- Preventing starvation under both FIFO and EDF scheduling
- Detecting burnout precisely within a 10 ms window
- Serializing logs so no two messages ever interleave

---

## Instructions

### Compilation

All source files are in the `coders/` directory. From the root of the repository:

```bash
make
```

This produces the `codexion` binary in `coders/`.

Available Makefile rules: `all`, `clean`, `fclean`, `re`.

The program compiles with `-Wall -Wextra -Werror -pthread`.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All arguments are mandatory. Times are in milliseconds. `scheduler` must be exactly `fifo` or `edf`.

### Examples

```bash
# 4 coders, each must compile 3 times, FIFO scheduling
./codexion 4 800 200 200 200 3 50 fifo

# 5 coders, EDF scheduling
./codexion 5 800 200 200 200 3 50 edf

# Tight timing — expect a burnout
./codexion 4 410 200 200 200 5 50 fifo

# Single coder — always burns out (cannot compile alone)
./codexion 1 500 200 200 200 3 50 fifo
```

### Expected log format

```
timestamp_ms X has taken a dongle
timestamp_ms X is compiling
timestamp_ms X is debugging
timestamp_ms X is refactoring
timestamp_ms X burned out
```

The simulation stops when all coders have compiled at least `number_of_compiles_required` times, or when any coder burns out.

---

## Blocking Cases Handled

### Deadlock Prevention

A circular resource topology creates the classic **circular wait** condition from Coffman's four necessary conditions for deadlock:

1. **Mutual exclusion** — each dongle can only be held by one coder at a time
2. **Hold and wait** — a coder could hold one dongle and wait for the other
3. **No preemption** — dongles cannot be forcibly taken
4. **Circular wait** — coder N waits for dongle held by coder N+1

The solution attacks condition 4 with two complementary strategies:

- **Startup staggering**: even-numbered coders delay their first compile attempt by `time_to_compile` milliseconds, so odd coders (1, 3, 5...) always grab their dongle pairs first. This eliminates the initial simultaneous rush that causes circular wait.
- **Try-and-release on second dongle**: a coder registers in both dongle queues atomically before blocking, but if it cannot acquire the second dongle within its deadline, it releases the first entirely and retries. No coder ever holds a dongle indefinitely while waiting for another.

### Starvation Prevention

Under FIFO, queue position is determined by arrival timestamp — earlier requests are served first, guaranteeing bounded wait times.

Under EDF, queue position is determined by burnout deadline — the coder closest to burning out is served first. This provides liveness guarantees when parameters are feasible: the coder most at risk always gets priority access to the dongles it needs.

Both policies use a **min-heap priority queue** per dongle. Critically, both dongles for a compile attempt are enqueued using the **same priority snapshot**, taken atomically before any blocking occurs. This prevents a coder from losing its queue position between the two enqueue calls.

### Cooldown Handling

After a dongle is released, it becomes unavailable for `dongle_cooldown` milliseconds. The `available_at` timestamp is set on release and checked in `is_my_turn()` before granting access. Coders waiting for a dongle sleep via `pthread_cond_timedwait` until either the cooldown expires or their deadline is reached, whichever comes first.

### Precise Burnout Detection

A dedicated monitor thread runs independently of all coder threads. It sleeps via `pthread_cond_timedwait` until the earliest known burnout deadline across all coders, then checks whether any coder has genuinely expired. This approach uses zero CPU while waiting and wakes up precisely at the right moment.

When a coder compiles successfully, it signals the monitor via `pthread_cond_signal` so the monitor recalculates the new earliest deadline immediately. This keeps the monitor's sleep window accurate as deadlines shift.

Burnout logs are printed within the 10 ms window required by the subject.

### Log Serialization

All log output goes through `log_action()`, which holds `sim->print_mutex` for the duration of the `printf` call. This guarantees that no two log lines ever interleave, even when multiple coder threads and the monitor thread attempt to log simultaneously.

---

## Thread Synchronization Mechanisms

### `pthread_mutex_t` — Mutual Exclusion

Four categories of mutexes are used:

| Mutex | Protects |
|---|---|
| `sim->sim_mutex` | `simulation_over` flag, `coder->deadline`, `coder->last_compile_start`, `coder->compile_count` |
| `sim->print_mutex` | All stdout output via `log_action()` |
| `sim->monitor_mutex` | The monitor's condition variable and sleep state |
| `dongle->mutex` | Each dongle's `in_use` flag, `available_at` timestamp, and priority queue |

**Race condition prevention example** — updating a coder's deadline:
```c
// Writer (coder thread) — holds sim_mutex
pthread_mutex_lock(&sim->sim_mutex);
coder->last_compile_start = get_time_ms();
coder->deadline = coder->last_compile_start + sim->time_to_burnout;
pthread_mutex_unlock(&sim->sim_mutex);

// Reader (monitor thread) — also holds sim_mutex
pthread_mutex_lock(&sim->sim_mutex);
earliest = sim->coders[0].deadline;
// ... iterate coders
pthread_mutex_unlock(&sim->sim_mutex);
```

Without `sim_mutex`, the monitor could read a partially updated deadline (e.g. `last_compile_start` written but `deadline` not yet updated), leading to a spurious burnout detection.

### `pthread_cond_t` — Condition Variables

Three condition variables coordinate blocking and waking:

**Per-dongle `dongle->cond`**: coder threads block here while waiting for a dongle to become available. On every `release_dongle()`, `pthread_cond_broadcast` wakes all waiters so they can re-check `is_my_turn()`. Broadcast is used instead of signal because the highest-priority waiter (front of the heap) may not be the thread the OS would otherwise wake.

```c
// Release side
dongle->in_use = 0;
dongle->available_at = get_time_ms() + sim->dongle_cooldown;
pthread_cond_broadcast(&dongle->cond);  // wake all waiters

// Wait side (inside take_dongle_queued)
pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
// ts = min(deadline, cooldown_expiry)
```

**`sim->monitor_cond`**: the monitor thread blocks here between deadline checks. Coder threads signal it after each successful compile to update the sleep window. The simulation-over path also signals it to unblock the monitor for a clean exit.

```c
// After updating deadline in do_compile:
pthread_mutex_lock(&sim->monitor_mutex);
pthread_cond_signal(&sim->monitor_cond);
pthread_mutex_unlock(&sim->monitor_mutex);
```

### Priority Queue (Min-Heap)

A custom min-heap is implemented per dongle since C has no standard priority queue. Each node stores a coder ID and a priority value (arrival timestamp for FIFO, burnout deadline for EDF). The heap guarantees O(log n) insertion and O(log n) removal, with O(1) peek at the highest-priority waiter.

`pq_remove()` supports arbitrary removal (for bailout on timeout) by replacing the removed node with the last element and calling `pq_reheapify()`, which runs either `bubble_up` or `bubble_down` depending on whether the replacement is smaller or larger than its new neighbors.

### Thread-Safe Communication Between Coders and Monitor

Coders and the monitor never communicate directly. All coordination happens through shared state protected by `sim_mutex`:

- Coders write `deadline` and `compile_count` under `sim_mutex`
- Monitor reads `deadline` under `sim_mutex` to find the next expiry
- Either a coder or the monitor may set `simulation_over = 1`, but only the **first** to do so acts on it (checked atomically inside `sim_mutex`)
- After setting `simulation_over`, the setter broadcasts on all dongle condition variables to unblock any coder stuck in `take_dongle_queued`

---

## Resources

### Classic References

- **Dijkstra, E.W.** — *Hierarchical Ordering of Sequential Processes* (1971) — original formulation of the Dining Philosophers problem
- **Coffman, E.G. et al.** — *System Deadlocks* (1971) — the four necessary conditions for deadlock
- **POSIX Threads Programming** — Lawrence Livermore National Laboratory: https://hpc-tutorials.llnl.gov/posix/
- **The Linux Programming Interface** — Michael Kerrisk, chapters 30–33 (threads, mutexes, condition variables)
- **`man pthread_cond_timedwait`**, **`man pthread_mutex_init`**, **`man gettimeofday`**
- **Earliest Deadline First scheduling** — Liu & Layland, *Scheduling Algorithms for Multiprogramming in a Hard Real-Time Environment*, JACM 1973

### AI Usage

Claude (Anthropic) was used throughout this project as a pair-programming assistant. Specifically:

- **Architecture design**: defining the structure layout (`t_sim`, `t_coder`, `t_dongle`, `t_pqueue`) and the overall threading model before writing any code

- **Bug diagnosis**: identifying and fixing race conditions flagged by ThreadSanitizer, including unprotected reads of `coder->deadline` and `coder->compile_count`

- **README writing**: drafting this document
