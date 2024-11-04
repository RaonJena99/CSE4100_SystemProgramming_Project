/* $begin shellmain */
#include "myshell.h"
#include<errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
void user_command(char **argv);
void pipe_command(char ***argv, int Count, int F);
void free_pipe(char ***argv);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
char *** New_argv(char **argv);

int Num_Pipe;

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
	/* Read */
	printf("CSE4100-SP-P2> ");                   
	fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char ***pipe_argv;
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
	return;   /* Ignore empty lines */

    if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        pipe_argv = New_argv(argv);
        if(!Num_Pipe) user_command(argv);
        else pipe_command(pipe_argv,0,0);
        free_pipe(pipe_argv);

	/* Parent waits for foreground job to terminate */
	if (!bg){ 
	    int status;
	}
	else//when there is backgrount process!
	    printf("%d %s", pid, cmdline);
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit")) /* Quit & Exit command */
	exit(0);  

    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	return 1;

    if (!strcmp(argv[0], "cd"))    /* Navigate the directories */
    { 
        if(argv[1]){
            if(chdir(argv[1])) perror(argv[0]);
        }
        else chdir(getenv("HOME"));
        return 1; 
    }

    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    

    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

void user_command(char **argv) 
{
    int child_status;
    pid_t pid;

    if((pid=fork())==0){
        if (execvp(argv[0], argv) < 0) {
            printf("%s: Command not found. \n", argv[0]);
            exit(0);
        }
    } else{
        if((pid = waitpid(pid,&child_status,WUNTRACED)) < 0)
            unix_error("waitpid");
    }
}


char ***New_argv(char **argv) {
    Num_Pipe = 0;
    int i, j, k, v;

    for (i = 0; argv[i]; i++)
        if (!strcmp(argv[i], "|")) Num_Pipe++;

    char ***pipe_argv = (char***)malloc(sizeof(char**) * (Num_Pipe + 2)); 
    char *tmp;

    pipe_argv[0] = (char**)malloc(sizeof(char*) * MAXARGS);

    for (i = 0, j = 0, v = 0; argv[v]; v++) { 
        tmp = argv[v];
        k = 0;
        if (!strcmp(tmp, "|")) {
            pipe_argv[++i] = (char**)malloc(sizeof(char*) * MAXARGS);
            j = 0;
        } else {
            pipe_argv[i][j] = (char*)malloc(sizeof(char) * MAXARGS);
            while (*tmp) {
                if (*tmp == '\'' || *tmp == '\"') {
                    tmp++;
                    continue;
                }
                pipe_argv[i][j][k] = *tmp;
                k++; tmp++;
            }
            pipe_argv[i][j][k] = '\0';
            j++;
        }
        pipe_argv[i][j] = 0;
    }
    pipe_argv[i + 1] = NULL; 

    return pipe_argv;
}

void pipe_command(char ***argv, int Count, int F)
{
    int fds[2];
    int fd = F;
    int child_status;
    pid_t pid;

    if (pipe(fds) == -1) {
        perror("pipe");
        exit(0);
    }
    
    if((pid=fork())==0){
        close(fds[0]);
        if (Num_Pipe > Count){
            dup2(fd, STDIN_FILENO);
            dup2(fds[1], STDOUT_FILENO);
        } else {
            dup2(fd, STDIN_FILENO);
        }

        if (!builtin_command(*argv)) {
          if (execvp((*argv)[0], *argv) < 0) {
              printf("%s: Command not found. \n", (*argv)[0]);
              exit(0);
          }
        }
    } else {
        if (Num_Pipe > Count){
            close(fds[1]); 
            pipe_command(argv + 1, Count + 1,fds[0]);
        } else {
            close(fds[0]);
        }

        if((pid = waitpid(pid,&child_status,WUNTRACED)) < 0)
        unix_error("waitpid");
    }
}

void free_pipe(char ***argv) {
    int i, j;
    for (i = 0; argv[i] != NULL; i++) {
        for (j = 0; argv[i][j] != NULL; j++) {
            free(argv[i][j]);
        }
        free(argv[i]);
    }
    free(argv);
}
