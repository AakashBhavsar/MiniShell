#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define BUFFER_LEN 1024
#define MAXLIST 100 // max number of commands to be supported


void handle_sigtstp(int sig)
{
    printf("\nStop not allowed\nMyShell>>");
}


// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
	int i;
    // printf("str: %s\n\n",str);
	for (i = 0; i < 2; i++) 
    {
		strpiped[i] = strsep(&str, "|"); //separate string from pipe ls -ltr | wc -l
		if (strpiped[i] == NULL)
			break;
       // printf("strpiped: %s\n\n",strpiped[i]);
	}
    // printf("charc: %s\n", c);
	if (strpiped[1] == NULL)
		return 0; // returns zero if no pipe is found.
	else 
    {
		return 1;
	}
}
// function for parsing command words
void parseSpace(char* str, char** parsed)
{
	int i;

	for (i = 0; i < MAXLIST; i++) 
    {
		parsed[i] = strsep(&str, " "); // ls   -ltr
        
		if (parsed[i] == NULL)
			break;

		if (strlen(parsed[i]) == 0)
			i--;
	}
    // printf("parsed: %s\n\n",parsed[0]);
    // printf("parsed: %s\n\n",parsed[1]);
}

int processString(char* str, char** parsed, char** parsedpipe)
{

	char* strpiped[2];
	int piped = 0;

	piped = parsePipe(str, strpiped);

	if (piped) 
    {
		parseSpace(strpiped[0], parsed); // ls -ltr
		parseSpace(strpiped[1], parsedpipe);// wc -l
        return 1;
	}
    else
    {
		parseSpace(str, parsed);
        return 0;
	}
}

void processPipe(char** parsed, char** parsedpipe)
{
    int fd[2];
    //printf("This is first parent process with PID %d and PPID %d. \n", getpid(), getppid() );
    pid_t pid1 = fork();
    if(pid1==0)
    {
        pipe(fd);
        pid_t pid2 = fork();
        if(pid2!=0) // parent will write
        {
            int childPid,status;
            //printf("This is second parent process with PID %d and PPID %d. \n", getpid(), getppid() );
            close(fd[0]);
            dup2(fd[1],1); //1 is stdout file descriptor
            close(fd[1]);
            if(execvp(parsed[0],parsed)<0) //ls -ltr
            {
                printf("\nCould not execute command in parent\n");
            }
            childPid = wait(&status);
            // printf("Child %d has terminated\n", pid2);
            perror("connect");
        }
        else //child will read
        {
            //printf("This is child process with PID %d and PPID %d. \n", getpid(), getppid() );
            close(fd[1]);
            dup2(fd[0],0);
            close(fd[0]);
            if(execvp(parsedpipe[0],parsedpipe)<0) //wc -l
            {
                printf("\nCould not execute command in child\n");
            }
            perror("connect");
        }
        // printf("PID %d terminates. \n", getpid() );
    }
    else
    {
        wait(NULL);
        //exit(1);
    }
}
    

void simpleCMD(char** parsed)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1) 
    
    {
		printf("\nFailed forking child..");
		return;
	} 
    else if (pid == 0) 
    {
		if (execvp(parsed[0], parsed) < 0) // ls   ls -ltr
        {
			printf("\nCould not execute command..");
		}
		exit(0);
	} 
    else 
    {
		// waiting for child to terminate
		wait(NULL);
		return;
	}
}

/* The built-in cd command. Changes the working directory to the
   one given in the argument. If the given directory is invalid
   an attempt to go to the HOME directory is made. */
void cd(char* directory) {
    if(directory == NULL) {
      //goHome();
      return;
    }

    if(chdir(directory) == -1) {
      fprintf(stderr, "Unable to go to directory %s. Attempting to go to HOME directory\n", directory);
      //goHome();
    }
}

int main(){
    char line[BUFFER_LEN];  //get command line
    char* parsed[MAXLIST];
    char* parsedpipe[MAXLIST];
    int fd[2], pipeFlag=0;
    char progpath[20];

    char* username = getenv("USER");
	printf("\nUSER is: @%s\n", username);

    
    while(1){
        char* buf;
        signal(SIGTSTP, &handle_sigtstp);
        buf = readline("\nMyShell>> ");
	    if (strlen(buf) != 0) 
        {
		    add_history(buf);
		    strcpy(line, buf);
	    } 
        else 
        {
		    break;
	    }
        
        //printf("%s\n",line);
        size_t length = strlen(line);
        if (line[length - 1] == '\n')
            line[length - 1] = '\0';
        if(strcmp(line, "exit")==0)
        {            //check if command is exit
            break;
        }
        
        pipeFlag = processString(line, parsed, parsedpipe);

        char path[500];
        if(strcmp(line, "cd")==0)
        {
            cd(parsed[1]); // cd folder name
        }
        else if(pipeFlag)
        {
            printf("\n-------------------------------------------------\n");
            processPipe(parsed, parsedpipe);
            sleep(0.1);
            printf("-------------------------------------------------\n");
        }
        else
        {
            printf("\n-------------------------------------------------\n");
            simpleCMD(parsed);
            printf("-------------------------------------------------\n");
        }
    }
} 