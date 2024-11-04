/* $begin shellmain */
#include "myshell.h"
#include<errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
void user_command(char **argv);
void pipe_command(char ***argv, int Count, int F);
void free_pipe(char ***argv);
void New_back(char* cmdline, char** argv, int bg);
void Add(pid_t pid, int status, const char* cmd);
void Delete(pid_t pid);
void Print_Job();
void Clear_Job(int op);
void signal_handler(int sig);
void Change_command(char* cmd, int cond);
void Finish(pid_t pid);
void Free_Job();
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
int chk_group(char **argv, char ***cmd, int pipe);
char *** New_argv(char **argv);

int Num_Pipe;
int job_count = 0;
int stop_count = 0;
const char *STATUS[6] = {NULL, "running\t", "terminated", "suspended\t", "done\t", NULL};
const int Fore = 0, Back = 1, Die = 2, Stop = 3, Done = 4, Sint = 5;

typedef struct job {
    pid_t pid;
    int status; 
    char *cmd;
} Job;

Job jobs[MAXARGS];

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    Signal(SIGCHLD, signal_handler);
    Signal(SIGTSTP, signal_handler);
    Signal(SIGINT, signal_handler);

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
    char ***back_argv;
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    int status;
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    New_back(strdup(cmdline), argv, bg);

    if (argv[0] == NULL)  return;

    if (builtin_command(argv)) return;

	if (bg){ 
	    if ((pid = fork()) == 0) {
            back_argv = New_argv(argv);
            if (Num_Pipe) {
                if (chk_group(argv, back_argv, 1))
                    setpgrp();
                pipe_command(back_argv,0,0);
            }
            else {
                if (chk_group(argv, back_argv, 0))
                    setpgrp();
                user_command(argv);
            }
            exit(0); 
        }
        else {
            if (bg) {
                Add(pid, Back, strdup(cmdline));
            } else {
                Add(pid, Fore, strdup(cmdline));
                if (waitpid(pid, &status, WUNTRACED) < 0) {
                    unix_error("waitpid");
                } else {
                    if (WIFEXITED(status)) {
                        Delete(pid);
                    }
                }
            }
        }
	}
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    int job_num;
    pid_t pid;

    if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit")) /* Quit & Exit command */
    {
        Free_Job();
	    exit(0);  
    }

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

    if (!strcmp(argv[0], "jobs")) 
    {
		if (jobs[0].cmd) {
			Print_Job(jobs, 0);
			Clear_Job(0);
		}
		return 1;
	}

    if (!strcmp(argv[0], "bg"))
    {
        if (argv[1][0] != '%') {
            printf("-bash: bg: Invalid job specifier\n");
            return 1;
        }

        job_num = atoi(argv[1] + 1);
        if (job_num <= 0) {
            printf("-bash: bg: Invalid job number\n");
            return 1;
        }

        int found = 0;
        for (int i = 0; jobs[i].cmd; i++)
        {
            if (jobs[i].pid == pid)
            {
                if (jobs[i].status == Back)
                {
                    printf("-bash: bg: job %d already in background\n", job_num);
                    return 1;
                }

                found = 1;
                Change_command(jobs[i].cmd, Back);
                jobs[i].status = Back;
                pid = jobs[i].pid;
                kill(pid, SIGCONT);
                break;
            }
        }

        if (!found)
            printf("-bash: bg: %%%d: no such job\n", job_num);

        return 1;
    }

    if (!strcmp(argv[0], "fg"))
    {
        if (argv[1][0] == '%' && (job_num = atoi(argv[1] + 1)) > 0)
        {
            int found = 0;
            for (int i = 0; jobs[i].cmd; i++)
            {
                if (i + 1 == job_num)
                {
                    found = 1;
                    if (jobs[i].status == Stop)
                        Change_command(jobs[i].cmd, Stop);
                    jobs[i].status = Fore;
                    pid = jobs[i].pid;
                    kill(pid, SIGCONT);
                    waitpid(pid, NULL, WUNTRACED); // 해당 작업이 종료될 때까지 대기
                    break;
                }
            }
            if (!found)
                printf("-bash: fg: %%%d: no such job\n", job_num);
        }
        return 1;
    }

    if (!strcmp(argv[0], "kill"))
    {
        if (argv[1][0] == '%' && (job_num = atoi(argv[1] + 1)) > 0)
        {
            int found = 0;
            for (int i = 0; jobs[i].cmd; i++)
            {
                if (i + 1 == job_num)
                {
                    found = 1;
                    if (jobs[i].status == Stop)
                        jobs[i].status = Die;
                    pid = jobs[i].pid;
                    kill(-pid, SIGINT);
                    break;
                }
            }
            if (!found)
                printf("-bash: kill: %%%d: no such job\n", job_num);
        }
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
            pipe_argv[i][j] = (char*)malloc(sizeof(char) * (strlen(tmp)+1));
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

void New_back(char* cmd, char** argv, int bg)
{
	int i = 0;
    char* tmp;

    while (*argv) {
        tmp = *argv;
        while (*tmp) {
            cmd[i++] = *tmp++;
        }
        cmd[i++] = ' '; 
        argv++;
    }
    cmd[--i] = '\0'; 
    if (bg) strcat(cmd, " &\0");
    
    for (i = 0; cmd[i]; i++) {
        if (cmd[i] == '|' && (i == 0 || cmd[i - 1] != ' ')) {
            memmove(cmd + i + 1, cmd + i, strlen(cmd + i) + 1);
            cmd[i++] = ' ';
        }
    }
}

int chk_group(char **argv, char ***cmd, int pipe)
{
    if (pipe) {
        for (int i = 0; cmd[i][0]; i++) {
            if ((!strcmp(cmd[i][0], "cat") && !cmd[i][1]) ||
                (!strcmp(cmd[i][0], "grep") && ((i == 0 && cmd[i][1]) || (i >= 1 && !cmd[i][1]))) ||
                (!strcmp(cmd[i][0], "less") && ((i == 0 && cmd[i][1]) || (i >= 1 && !cmd[i][1])))) {
                return 0;
            }
        }
        return 1;
    }
    else {
        if ((!strcmp(argv[0], "cat") && !argv[1]) ||
            (!strcmp(argv[0], "grep") && argv[1]) ||
            (!strcmp(argv[0], "less") && argv[1])) {
            return 0;
        }
        return 1;
    }
}

void Add(pid_t pid, int status, const char* cmd) {
    if (job_count < MAXARGS) {
        jobs[job_count].pid = pid;
        jobs[job_count].status = status;
        jobs[job_count].cmd = cmd;
        job_count++;
    } else {
        printf("Maximum number of jobs reached.\n");
    }
}

void Delete(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            free(jobs->cmd);
            job_count--;
            break;
        }
    }
}

void Print_Job()
{
    for (int i = 0; i < job_count; i++)
    {
        char status_prefix = (i == job_count - 1) ? '+' : ' ';
        char stop_suffix = (stop_count == 1) ? '-' : (stop_count >= 2) ? ' ' : '+';

        if (jobs[i].status == Done)
        {
            char *delim;
            if (delim = strchr(jobs[i].cmd, '&'))
                *delim = '\0';
        }

        printf("[%d]%c%c  %s\t\t\t%s\n", i + 1, status_prefix, stop_suffix, STATUS[jobs[i].status], jobs[i].cmd);
    }
}

void signal_handler(int sig) {
    pid_t pid;
    int status;
    int i;

    switch (sig) {
        case SIGCHLD:
            while ((pid = waitpid(-1, &status, WNOHANG | WCONTINUED)) > 0) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    Finish(pid);
                    Delete(pid);
                }
            }
            break;
        case SIGTSTP:
            printf("\n");
            for (i = 0; i < job_count; i++) {
                if (jobs[i].status == Fore) {
                    stop_count += 1;
                    Change_command(jobs[i].cmd, Stop);
                    jobs[i].status = Stop;
                    pid = jobs[i].pid;
                    kill(pid, SIGTSTP);
                    printf("[%d]+ Stopped\t\t\t%s\n", i + 1, jobs[i].cmd);
                    return;
                }
            }
            break;
        case SIGINT:
            printf("\n");
            for (i = 0; i < job_count; i++) {
                if (jobs[i].status == Fore) {
                    pid = jobs[i].pid;
                    jobs[i].status = Sint;
                    kill(pid, SIGINT);
                    break;
                }
            }
            break;
        default:
            fprintf(stderr, "Unhandled signal: %d\n", sig);
            break;
    }
}

void Clear_Job(int op)
{
    for (int i = 0; i < job_count; i++)
    {
        if (jobs[i].status == Done || jobs[i].status == Die || jobs[i].status == Sint)
        {
            Change_command(jobs[i].cmd, Done);
            if (op)
            {
                if (jobs[i].status == Done)
                    printf("[%d]   Done\t\t\t%s\n", i + 1, jobs[i].cmd);
                if (jobs[i].status == Die)
                    printf("[%d]   Terminated\t\t\t%s\n", i + 1, jobs[i].cmd);
            }
            free(jobs[i].cmd);

            for (int j = i; j < job_count - 1; j++)
            {
                jobs[j] = jobs[j + 1];
            }

            job_count--;
            i--;
        }
    }
}

void Change_command(char* cmd, int cond)
{
	int i;

	i = strlen(cmd);
	if (cond == Back) {
		strcat(cmd, " &\0");
	}
	else {
		if (strchr(cmd, '&')) {
			cmd[i - 2] = '\0';
		}
	}
}

void Finish(pid_t pid)
{
    for (int i = 0; i < job_count; i++)
    {
        if (jobs[i].pid == pid)
        {
            jobs[i].status = Done;
            break;
        }
    }
}

void Free_Job(void)
{
    for (int i = 0; jobs[i].cmd; i++) {
        if (jobs[i].status != Die && jobs[i].status != Done && jobs[i].status != Sint)
            kill(-(jobs[i].pid), SIGINT);
        if (jobs[i].cmd) 
            free(jobs[i].cmd);
    }
    memset(jobs, 0, sizeof(jobs)); 
}