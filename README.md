# Minimal C++ Shell

This is a **basic Unix-like shell written in C++**.  
It is an incremental project aimed at understanding how a shell works internally by building core features step by step.

Currently, the project consists of a **single source file (`main.cpp`)** and focuses on correctness and clarity rather than completeness.

---

## ✨ Features Implemented

- Interactive shell prompt (`$`)
- Command parsing with:
  - Whitespace handling
  - Single quotes (`'`)
  - Double quotes (`"`) with basic escape support
- Built-in commands:
  - `echo`
  - `pwd`
  - `cd`
  - `type`
  - `exit`
- External command execution using:
  - `fork()` and `execv()` (Linux / Unix)
- Output redirection support:
  - `>` redirect standard output
  - `>>` append standard output
  - `2>` redirect standard error
- Executable lookup using the `PATH` environment variable
- Basic permission checks before execution

---

## 🛠️ High-Level Design

1. The shell reads a full line of input from the user.
2. Input is parsed into command arguments and redirection rules.
3. Built-in commands are handled directly in the shell process.
4. For external commands:
   - A child process is created using `fork()`
   - Output redirections are applied using `open()` and `dup2()`
   - The command is executed using `execv()`
5. The parent process waits for the child to complete.

---

## ▶️ Build and Run

### Requirements
- Linux / Unix system
- `g++` compiler with C++17 support or newer

### Compile
```bash
g++ -std=c++17 main.cpp -o minishell
