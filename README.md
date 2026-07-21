# ST5004CEM - Operating Systems and Security
## Coursework Assignment (March Intake 2026)

**Student Name:** [Yaswant Poucdel]
**Student ID:** [15941222]

---


## Task 1: Process Management and Threading

**Folder:** `Task-1/`

Demonstrates multi-threading, mutex/semaphore synchronization, a race condition and its fix, deadlock prevention via consistent lock ordering, and a round-robin CPU scheduler simulation.

**Compile:**
```bash
cd Task-1
gcc task1.c -o task1 -lpthread
```

**Run:**
```bash
./task1
```

**Expected output:** Five demo sections print in sequence — race condition (unsafe vs safe counter), semaphore-limited resource access, deadlock-free lock acquisition, and round-robin scheduling with average waiting time.

---

## Task 2: Memory Management Simulation

**Folder:** `Task-2/`

Simulates a paging system with configurable page size, and compares FIFO and LRU page replacement algorithms, tracking page faults and hit/miss ratios.

**Compile:**
```bash
cd Task-2
gcc task2.c -o task2
```

**Run:**
```bash
./task2
```

**Expected output:** Address translation demo (page number/offset), followed by a step-by-step trace of FIFO and LRU replacement on the same reference string, with fault/hit ratios for comparison.

---

## Task 3: File System Operations and Security

**Folder:** `Task-3/`

Implements a secure file management system with user authentication, an owner/group/other permission model, file create/read/write/delete operations, XOR-based encryption/decryption, and audit logging to `audit.log`.

**Compile:**
```bash
cd Task-3
gcc task3.c -o task3
```

**Run:**
```bash
./task3
```

**Expected output:** Authentication result, permission listing, successful file operations by the authenticated owner, a denied access attempt by a lower-privilege user, encryption/decryption of the file, and a final deletion — all recorded in `audit.log`.

**Note:** After running, check the audit trail:
```bash
cat audit.log
```

---

## Task 4: Network Programming and IPC

**Folder:** `Task-4/`

A multi-client TCP server and client implementing a simple text-based protocol (`AUTH`, `MSG`, `QUIT`), with thread-per-client concurrency, credential authentication, input validation, and error handling.

**Compile (server):**
```bash
cd Task-4
gcc server.c -o server -lpthread
```

**Compile (client):**
```bash
gcc client.c -o client
```

**Run — requires two separate terminals:**

Terminal 1 (start the server first, leave it running):
```bash
./server
```

Terminal 2 (and additional terminals for more clients):
```bash
./client
```

**Example client session:**
```
> AUTH alice alice123
Server: OK Authenticated
> MSG Hello server
Server: OK Server received: Hello server
> QUIT
Server: OK Goodbye
```

**Valid test users:** `alice/alice123`, `bob/bob123`, `carol/carol123`

**To test concurrency:** open 2–3 more terminals, run `./client` in each, and authenticate as different users at the same time. Watch the server terminal for interleaved log lines confirming multiple simultaneous connections.

---

## Submission Checklist

- [ ] All source code files (`.c`) included and compiling without errors
- [ ] README with compilation/execution instructions (this file + per-task READMEs)
- [ ] Screenshots or output logs for each task's execution
- [ ] Design documentation for Task 1 (500–750 words)
- [ ] Analysis report comparing FIFO vs LRU for Task 2 (500–750 words)
- [ ] User guide + security analysis for Task 3 (500–750 words)
- [ ] Protocol documentation + testing documentation for Task 4 (500 words)
- [ ] All documentation converted to PDF format
- [ ] Files named according to `NAME_studentID` convention
- [ ] References included for any external resources used

---

## References

[List any references you used — lecture slides, textbook chapters, man pages such as `pthread_create(3)`, `socket(2)`, `sem_wait(3)`, etc.]

---