Overview:
The code implements a simple shell program in C.
It continuously prompts the user for commands and executes them.

Main Functionality:
The main() function initiates an infinite loop to continuously accept user input.
It prompts the user with "CSE4100-SP-P2>" and reads the input command line.
Then, it evaluates the command using the eval() function.

Evaluation of Command:
The eval() function parses the command line into arguments and checks if it's a built-in command or a user command.
If it's a built-in command (quit, exit, &, cd), it executes it directly without forking a new process.
If it's a user command, it forks a new process and executes the command in the child process while the parent process waits for it to finish.

Built-in Commands:
The builtin_command() function checks if the command is a built-in command (quit, exit, &, cd).
If it's a built-in command, it executes it and returns true; otherwise, it returns false.

Parsing Command Line:
The parseline() function parses the command line into arguments and determines if the job should run in the background or foreground.

User Command Execution:
The user_command() function is responsible for executing user commands in a child process using fork() and execvp().
If the command is not found, it prints an error message.

Error Handling:
Basic error handling is implemented for system calls like execvp() and waitpid().
Termination:
The program terminates gracefully when the user inputs quit or exit.