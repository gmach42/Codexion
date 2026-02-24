*This project has been created as part of the 42 curriculum by gmach.*
# Description
## Blocking cases handled
// TODO Describe concurrency issues (deadlocksm Coffman's conditions, starvation, cooldown, burnout detection and logs)
![Mutex logic](https://media.geeksforgeeks.org/wp-content/uploads/Mutex_lock_for_linux.jpg)
## Thread synchronization mechanisms
// TODO explain pthread_muted, pthread_cond... how they coordinate access to dongles, logging, monito. Demonstrate thread-safe communication
# Instructions
# Resources
[Geeks for Geeks - Multithreading in C](https://www.geeksforgeeks.org/c/multithreading-in-c/)
[Geeks for Geeks - Mutex](https://www.geeksforgeeks.org/linux-unix/mutex-lock-for-linux-thread-synchronization/)
[Geeks for Geeks - pthread Functions](https://www.geeksforgeeks.org/c/thread-functions-in-c-c/)
[Geeks for Geeks - Conditional Wait and Signal](https://www.geeksforgeeks.org/linux-unix/condition-wait-signal-multi-threading/)


Logic
```
1. Try to take LEFT dongle  → mutex_lock + wait if not available
2. Try to take RIGHT dongle → mutex_lock + wait if not available
3. Log "has taken a dongle" (x2)
4. compile()  → usleep(time_to_compile)
5. Release both dongles → each starts its cooldown (cond_timedwait)
6. debug()    → usleep(time_to_debug)
7. refactor() → usleep(time_to_refactor)
8. Repeat until burned out or compiled enough
```

 valgrind --tool=helgrind ./codexion 4 6000 3000 1000 1000 10 100 fifo
 valgrind --tool=drd ./codexion 4 6000 3000 1000 1000 10 100 fifo

ARGS:
	◦ `number_of_coders`: The number of coders and also the number of dongles.
	◦ `time_to_burnout` (in milliseconds): If a coder did not start compiling within
		`time_to_burnout` milliseconds since the beginning of their last compile or the
		beginning of the simulation, they burn out.
	◦ `time_to_compile` (in milliseconds): The time it takes for a coder to compile.
		During that time, they must hold two dongles.
	◦ `time_to_debug` (in milliseconds): The time a coder will spend debugging.
	◦ `time_to_refactor` (in milliseconds): The time a coder will spend refactoring.
		After completing the refactoring phase, the coder will immediately attempt to
		acquire dongles and start compiling again.
	◦ `number_of_compiles_required`: If all coders have compiled at least this
		many times, the simulation stops. Otherwise, it stops when a coder burns
		out.
	◦ `dongle_cooldown` (in milliseconds): After being released, a dongle is unavail-
		able until its cooldown has passed.
	◦ `scheduler`: The arbitration policy used by dongles to decide who gets them
		when multiple coders request them.
		The value must be exactly one of: `fifo` or `edf`.
		`fifo` means First In, First Out: the dongle is granted to the coder whose
		request arrived first.
		`edf` means Earliest Deadline First with deadline = last_compile_start +
		time_to_burnout


gdb
(gdb) break monitor_routine
(gdb) run
(gdb) info threads
(gdb) thread 3
(gdb) backtrace
