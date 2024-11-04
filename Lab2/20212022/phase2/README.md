Overview:
The code implements a simple shell program in C with the ability to handle piped commands.

Main Functionality:
The main() function initiates an infinite loop to continuously accept user input.
It prompts the user with "CSE4100-SP-P2>" and reads the input command line.
Then, it evaluates the command using the eval() function.

Evaluation of Command:
The eval() function parses the command line into arguments and checks if it's a built-in command or a user command.
If it's a built-in command (quit, exit, &, cd), it executes it directly without forking a new process.
If it's a user command, it checks for pipe symbols ("|") and handles piped commands accordingly.

Built-in Commands:
The builtin_command() function checks if the command is a built-in command (quit, exit, &, cd).
If it's a built-in command, it executes it and returns true; otherwise, it returns false.

Parsing Command Line:
The parseline() function parses the command line into arguments and determines if the job should run in the background or foreground.

Piped Commands:
The New_argv() function identifies the presence of pipe symbols and constructs argument arrays for each command in the pipeline.
The pipe_command() function executes piped commands recursively, setting up pipes and forking processes for each command.
It uses dup2() to redirect standard input and output appropriately for each command.

User Command Execution:
The user_command() function is responsible for executing non-piped user commands in a child process using fork() and execvp().

Memory Management:
The free_pipe() function is provided to free dynamically allocated memory for the argument arrays used in piped commands.