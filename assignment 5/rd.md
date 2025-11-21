# Assignment 05 - Group 85 - SimpleMultithreader

**Topic** : Using Multithreading with Ease
**Instructor** : Vivek Kumar

## Team Members
- **Aditya Kumar** (Roll No: 2024035)
- **Dipanshu** (Roll No: 2024196)

## Table of Content
- [Summary](#Summary)
- [Functionality](#Functionality)
- [Technical Implementation Details](#Technical-Implementation-Details)
- [How to Run](#How-to-Run)
- [Contribution](#Contribution)

## Summary
The **SimpleMultithreader** is a header-only C++ implementation designed to simplify the usage of Pthread-based parallelism. It provides a main method, `parallel_for`, to facilitate parallel execution of loop bodies using C++11 lambda expressions. It abstracts away the complexity of thread creation, joining, and memory management from the user.

## Functionality
The SimpleMultithreader performs the following functions:

**parallel_for Method for the vector file (1D Loops)**
- Accepts a lambda function representing the loop body.
- Runs the loop body in parallel using a specified number of Pthreads (`numThreads`).
- Dynamically creates Pthreads without utilizing task/thread pools.
- Ensures the exact number of threads specified by the programmer runs concurrently, utilizing the main thread as a worker to minimize overhead.

**parallel_for Method for the matrix file (2D Loops)**
- Designed for parallelizing two-dimensional for-loops (Matrices).
- Utilizes C++11 lambda expressions for the loop body.
- Creates a new set of Pthreads for each call, terminating them upon the scope's completion.

## Technical Implementation Details
This section details the specific logic used to satisfy the assignment constraints.

**1. Data Structures ("Suitcases")**
Since `pthread_create` only accepts a single `void*` argument, we defined structs to package the necessary data for the workers:
- `thread_args_vector`: Packages the 1D lambda and low/high indices.
- `thread_args_matrix`: Packages the 2D lambda and row/column boundaries.

**2. Rule B Compliance (Main Thread Optimization)**
To strictly follow the requirement that "runtime execution must have the exact number of threads specified," our implementation uses the Main Thread as a worker.
- If `numThreads` is 4, we spawn 3 new pthreads.
- The Main Thread executes the final chunk of work itself.
- This avoids the issue of having N+1 threads (N workers + 1 Main) and reduces context-switching overhead.

**3. Time Measurement**
Execution time is measured using `clock_gettime(CLOCK_MONOTONIC)` to capture high-precision start and end timestamps, which are printed to stdout after threads join.

## How to Run

**Compile the project:**
In the root directory, run:
```bash
make all
```
**Run the Vector Test:**
In the root directory, run the following command (replace `{Number of Threads}` with an integer, e.g., 4):
```bash
./vector {Number of Threads}
```
Example: `./vector 4`

**Run the Matrix Test:**
In the root directory, run the following command:
```bash
./matrix {Number of Threads}
```
Example: `./matrix 4`

**Clean up: To remove all executables and object files, run:**
```bash
make clean
```
## Contribution
All the work was collectively and equally done by both participants, Dipanshu and Aditya.
The code programming, logic design (structs, lambda handling), and compilation for the Multithreader were collectively discussed and executed via pair programming
