# Plus++ Language Interpreter

An interpreter for the **Plus++ Language**, a custom-designed programming language for handling big integers, performing arithmetic operations, loops, and structured output.  
The project was developed in **C** as part of the *Programming Languages* course (Spring 2025).  

---

## How It's Made:
**Tech used:** C, Parser & Virtual Machine Implementation, Error Handling  

The Plus++ Language was designed with the following features:
- Variable declaration (`number <var>;`)  
- Assignment, increment, and decrement statements  
- Output with `write` statements (supporting strings, integers, and `newline`)  
- Loops using `repeat ... times ...` syntax  
- Code blocks with `{ }` for nested operations  
- Comments using `* comment *`  

The interpreter (`ppp`) works from the command line:  
ppp myscript

## Key Implementation Details

- Parser: Builds a parse tree from the Plus++ code and ensures grammar correctness.
- Error Handling: Reports precise syntax and semantic errors with exact location.
- Virtual Machine: Executes parsed instructions and manages variables.
- Testing: Multiple .ppp files were created to validate loops, arithmetic, I/O, and error detection.
- Big Integer Support: Numbers can have up to 100 digits, enabling large integer computations.

## Optimizations
- Designed modularly with clear separation between lexer, parser, and interpreter.
- Implemented precise error reporting for better debugging.
- Allowed flexible handling of very large integers (up to 100 digits).
- Structured error detection to stop execution immediately at the first invalid statement.
- Reused parsing logic for loops and blocks to reduce duplicate code.

## Lessons Learned
- Gained practical experience in building a custom programming language interpreter.
- Learned the importance of error detection and user-friendly error messages.
- Strengthened understanding of parsing techniques and virtual machines.
- Improved teamwork, version control, and structured documentation skills.
- Understood how even a small language requires careful design decisions for scalability.
