# Bonus Assignment - Group 85 - EGOS -2000

**Topic**: Custom Shell Commands & MLFQ Scheduler Implementation
**Instructor**: Vivek Kumar

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
[cite_start]This project extends the **EGOS-2000** operating system by implementing user-space shell utilities and a kernel-space scheduler[cite: 25]. The assignment is divided into two parts:
1.  [cite_start]**Custom Shell Commands**: Implementation of `grep` and `wcl` using low-level EGOS system calls, bypassing standard C file libraries[cite: 28].
2.  [cite_start]**MLFQ Scheduler**: A preemptive Multi-Level Feedback Queue scheduler to manage process execution based on priority levels and runtime behavior[cite: 29].

## Functionality

**1. Shell Utilities (User Space)**
* [cite_start]**wcl (Word Count - Lines)**: Accepts multiple filenames as arguments, reads the full content of each file, and prints the total number of newlines across all files[cite: 67, 98].
* **grep (Global Regular Expression Print)**: Accepts a search pattern and a filename. [cite_start]It searches the file line-by-line and prints any line containing the specified pattern[cite: 66, 86].

**2. MLFQ Scheduler (Kernel Space)**
* [cite_start]**Prioritization**: Maintains 5 priority levels (0-4), where Level 0 is the highest priority[cite: 128].
* [cite_start]**Dynamic Adjustment**: Processes move down levels based on CPU usage time slices ($(level+1) \times 100$ ms)[cite: 129].
* [cite_start]**Aging/Reset**: To prevent starvation, all processes are reset to Level 0 every 10 seconds or upon keyboard interrupts (for the Shell process)

## Technical Implementation Details

**1. Shell Commands (`wcl` & `grep`)**
[cite_start]To comply with the strict requirement of not using standard C file APIs (like `fopen`, `fgets`)[cite: 80]:
* [cite_start]**System Calls**: We utilized `dir_lookup` to retrieve inode numbers and `file_read` to access file content directly from the disk[cite: 75, 76].
* [cite_start]**Block-Based Reading**: Since `file_read` retrieves data in fixed chunks (`BLOCK_SIZE`), we implemented a loop to read files block-by-block until the end of the file[cite: 76, 79].
* **Manual Buffering**:
    * For `wcl`: We iterate through the buffer counting `\n` characters.
    * For `grep`: We manually reconstruct lines from the block buffer. If a line spans across two blocks, the logic preserves the partial line from the first block and appends the start of the second block before checking for the pattern using `strstr`.

**2. MLFQ Scheduler**
* [cite_start]**Process Struct**: Updated `struct process` in `process.h` to track `current_level`, `time_slice_remaining`, and lifecycle statistics (turnaround time, response time)[cite: 111, 116].
* [cite_start]**Scheduling Logic**: Modified `proc_yield()` in `kernel.c` to always select the runnable process with the lowest level number (highest priority)[cite: 133].
* [cite_start]**Interrupt Handling**: Updated `intr_entry()` to handle timer interrupts, decrementing the running process's remaining time and triggering level demotion if the time slice expires[cite: 141].

## How to Run

**1. Compile the Project**
[cite_start]Navigate to the root `egos-2000` directory and run the following commands to build the kernel and user applications[cite: 32]:

```bash
# Clean previous builds
make clean

# Compile the OS and applications
make
