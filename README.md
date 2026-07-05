# ST5004CEM - Operating Systems and Security
## Task 1: Process Management and Threading

**Student Name:** [Yaswant poudel]
**Student ID:** [240744]
---

## What the Program Does

The program executes 5 demonstrations in sequence:

1. **Race Condition Demo**
   - Shows a shared counter incremented by 4 threads
   - Without a lock (may produce incorrect totals)
   - With a mutex lock (always produces the correct total)

2. **Semaphore Demo**
   - 5 worker threads compete for a resource
   - Only 2 threads may access it concurrently (enforced by semaphore)

3. **Deadlock Prevention Demo**
   - 2 threads each acquire two locks (`lock_a`, `lock_b`)
   - Both acquire them in the **same order**, preventing circular wait / deadlock

4. **Round-Robin Scheduler Simulation**
   - Simulates CPU scheduling for 3 processes with burst times 10, 5, 8
   - Uses a time quantum of 3
   - Prints execution order and average waiting time

---
