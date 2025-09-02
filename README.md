# CSE231_Assignment - Monsoon 2025

 Welcome to the Github Repository
## Table of Content
- [Overview](#overview)
- [Course-Information](#Course-Information)
- [Assignments](#Assignments)
- [Contributor](#Contributor)


## Overview
This repository contains all of the Assignments provided for Operating Systems Course during the Monsoon semester of 2025.

## Course-Information
- **Course-Name** : CSE231 - Operating System
- **Semester** : Monsoon - 2025
- **instructor** : Prof. Vivek Kumar

## Assignments
Here's a sneak peek at some of the assignments you'll find in this repository:
1. [Assignment 1: "SimpleLoader: An ELF Loader in C from Scratch"](#Assignment-1-SimpleLoader-An-ELF-Loader-in-C-from-Scratch")
   - Implement a SimpleLoader for loading an ELF 32-bit executable in plain-C.
   - Without using any library APIs available.

## Contributors
- [Aditya Kumar](https://github.com/Adityaa0124)
   - implemented the mem cleanup and releasing memory
   - implemented the load entire binary content into the memory from the ELF file
   - implemented the loading of the program header
   - tracking the program header and iterate through the phdr table
- [Dipanshu](https://github.com/dipanshu2313uwu)
   - iterate through the program header table and find the section of PT_LOAD
   - implemented the allocation of memory of size "p_memsz" using mmap function and copy the segment
   - implemented the Navigate to the entrypoint address into the segment loaded in the memory
   - Typecast the address to that of function pointer matching "_start" method in fib.c.



