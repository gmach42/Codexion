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
