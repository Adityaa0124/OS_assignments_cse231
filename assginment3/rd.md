# 🐾 SimpleScheduler

**SimpleScheduler** is a lightweight process scheduler written entirely in **C** as part of the **CSE 231: Operating Systems** course (Assignment 3, IIIT-Delhi).

It extends the previous assignment, **SimpleShell**, by introducing a **daemon-like scheduler** that performs **round-robin scheduling** across multiple CPUs with fixed time slices.

This project demonstrates how an operating system efficiently allocates limited CPU resources among multiple processes using **signals** and **process control system calls**.

---

## ⚙️ How to Use SimpleScheduler

### 1. Compile the Programs

You can compile all the required files using `make`:

```bash
make
```

Or compile manually using `gcc`:

```bash
gcc simple_shell.c -o meowshell
gcc simple_scheduler.c -o simplescheduler
```
### 2. Run the Shell

Run the shell with the following command:
```bash
./meowshell <NCPU> <TSLICE_MS>
```
Parameters:
  NCPU → Number of CPUs (parallel processes that can run simultaneously)
  TSLICE_MS → Time slice per process in milliseconds

  ### 3. Submit Jobs
  
   Within the shell prompt, you can submit executable programs using:
  
    submit ./a.out
Each submitted job:
  - Is created as a new process using fork().
  - Waits for scheduling signals from the scheduler before execution.
  - Is scheduled in a round-robin fashion according to the defined time slice.
  - 
### 4. Program Termination

 Press Ctrl + C to exit the shell.
  - Upon termination, it will:
  - Stop accepting new jobs
  - Wait for all running jobs to finish
  - Gracefully terminate the scheduler
  - Display a final summary table of all executed jobs

## Implementation Overview
#### 1. Scheduler Daemon
The scheduler acts as a simple CPU scheduler that:
  ● Maintains a ready queue of process IDs (PIDs).
  ● Runs up to NCPU processes concurrently.
  ● Controls process execution using SIGSTOP and SIGCONT signals.
It implements Round-Robin Scheduling, ensuring each process receives an equal share of
CPU time defined by TSLICE_MS.

### 2. MeowShell
The shell is responsible for:
  ● Reading and executing user commands.
  ● Submitting new jobs to the scheduler.
  ● Maintaining a command history
  ● Communicating with the scheduler using a named FIFO pipe
  (/tmp/scheduler_fifo).

### 3. Communication via FIFO
The shell and scheduler communicate through a named FIFO pipe located at:
/tmp/scheduler_fifo
  ● Direction: Shell → Scheduler
  ● Data Sent: Process ID (pid_t)

## System Calls Used
  ● fork(): Create child processes
  ● exec(): Execute binaries
  ● waitpid(): Wait for process completion
  ● mkfifo(), open(), read(), write(): Interprocess communication
  ● kill(): Send control signals (SIGSTOP, SIGCONT, SIGTERM)
  ● signal(): Handle cleanup and interrupts
  ● gettimeofday(): Measure job duration

##CONTRIBUTOR
    ADITYA KUMAR
      ● Dummy_main.h
      ● simple_shell.c
    DIPANSHU
      ● Simple_sheduler.c
      ● Test files
##biblography
  communication via FIFO
  /tmp/scheduler_fifo 
  mkfifo() under ai policy and kill () signals from the class notes 
  
  
  

