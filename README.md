# CSE4100: System Programming Projects

This repository includes four projects from the **CSE4100 System Programming** course at Sogang University. Each project explores core system programming concepts, from implementing data structures to creating a custom memory allocator. The projects are as follows:

1. [Project 1: MyLib](#project-1-mylib)
2. [Project 2: MyShell](#project-2-myshell)
3. [Project 3: Concurrent Stock Server](#project-3-concurrent-stock-server)
4. [Project 4: Mallocator](#project-4-mallocator)

---

## Project 1: MyLib

### Overview
The **MyLib** project introduces students to kernel-level data structures. This project requires implementing lists, hash tables, and bitmaps in C, creating an interactive program to manage these structures, and evaluating their functionalities.

### Implementation Details
- **Data Structures**: Implement three key data structures:
  - **List**: Supports basic operations like `push`, `pop`, `swap`, and `shuffle`.
  - **Hash Table**: Uses a custom hash function to manage and store integer keys.
  - **Bitmap**: Allows setting and clearing bits in a defined-size bitmap.

- **Commands**:
  - `create list <LIST>`, `create hashtable <HASH TABLE>`, `create bitmap <BITMAP> <BIT CNT>`
  - `delete <LIST | HASH TABLE | BITMAP>`, `dumpdata <LIST | HASH TABLE | BITMAP>`
  - Other commands for specific operations (e.g., `list_push_back`, `bitmap_mark`).

- **Execution**: After implementing the required functions, the program should be able to read commands from input files (`*.in`) and execute them correctly.

### Submission
- Submit the source code, a `Makefile`, and documentation in a single `.tar.gz` file. The executable name must be `testlib`, and the documentation must be a `.docx` file.

---

## Project 2: MyShell

### Overview
The **MyShell** project involves creating a custom Linux shell program in C. This shell supports process control, job management, and inter-process communication through pipes. The project is divided into three phases:

1. **Phase I**: Basic shell implementation with core commands (e.g., `cd`, `ls`, `mkdir`, `rmdir`, `touch`, `cat`, `echo`, and `exit`).
2. **Phase II**: Extend the shell to support piping (`|`) between commands.
3. **Phase III**: Implement job control to run processes in the background, with commands like `jobs`, `bg`, `fg`, and `kill`.

### Key Features
- **Core Commands**: The shell executes built-in commands by forking a child process.
- **Piping**: Implements piping functionality, allowing the output of one process to become the input for the next.
- **Background Execution**: Supports background process execution and job control commands.

### Submission
- Organize the project into folders for each phase (`phase1`, `phase2`, `phase3`), each containing a `Makefile`, `README.md`, and source code. The submission must be archived in a `.tar` file named as `prj2_<student_id>.tar`.

---

## Project 3: Concurrent Stock Server

### Overview
This project implements a **Concurrent Stock Server** that handles simultaneous client requests. The server supports listing, buying, and selling stocks, using two different concurrency models: an event-driven approach and a thread-based approach.

### Project Tasks
1. **Task 1**: Implement the event-driven approach using `select()` to manage multiple clients.
2. **Task 2**: Use a thread-based approach with `pthread` to manage client connections via a pool of worker threads.
3. **Task 3**: Evaluate the performance of both implementations by measuring elapsed time with different numbers of clients.

### Features
- **Stock Management**: Load stock data from a `stock.txt` file and update it in memory. Operations include `show`, `buy`, and `sell`.
- **Binary Tree Structure**: Store stock data in a binary tree structure for efficient access and updates, implementing fine-grained locking to handle the readers-writers problem.
- **Performance Analysis**: Run experiments with various client configurations and record throughput to analyze scalability.

### Submission
- Submit the source code, a `Makefile`, and a detailed report as a PDF (`document.pdf`). The report should include performance graphs and analysis.

---

## Project 4: Mallocator

### Overview
The **Mallocator** project involves building a custom memory allocator in C, replacing standard functions like `malloc`, `free`, and `realloc`. The goal is to implement an efficient, 8-byte aligned allocator that minimizes fragmentation.

### Components
- **Functions**:
  - `mm_init`: Initializes the allocator.
  - `mm_malloc`: Allocates a block of at least a specified size.
  - `mm_free`: Frees a previously allocated block.
  - `mm_realloc`: Changes the size of a memory block, preserving its contents.

- **Consistency Checker**: Implement a heap consistency checker, `mm_check`, to verify the integrity of the heap by checking for issues like overlapping blocks or unlinked free blocks.

- **Performance Evaluation**: Use the provided `mdriver.c` program to test for correctness, space utilization, and throughput. Optimize for both metrics to achieve a high performance index.

### Programming Rules
- No standard memory-management functions (e.g., `malloc`, `calloc`) are allowed.
- The allocator must return 8-byte aligned pointers.
- No global or static compound data structures (e.g., arrays, structs) are allowed.

### Submission
- Submit only `mm.c` and `document.pdf`. Archive the files in a `.tar.gz` format named as `prj4_<student_id>.tar.gz`.

---

This README covers each project’s goals, features, and submission requirements. Follow the instructions precisely to ensure successful project completion.
