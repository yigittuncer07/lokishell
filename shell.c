#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define MAX_LINE 80

int argCount;

// Calls exit if ctrl-D is entered
// Reads args
void setup(char inputBuffer[], char *args[], short *isBackgroundProcess)
{
    int length; // # of characters in the command line
    int i;      // loop index for accessing inputBuffer array
    int start;  // index where beginning of next command parameter is
    int ct = 0; // index of where to place the next parameter into args[]

    // Use ANSI escape code to set text color to green
    printf("\033[0;32m"); // 0;32 represents green
    printf("heroshell: ");
    printf("\033[0m"); // Resets color
    fflush(stdout);

    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0); /* ^d was entered, end of user command stream */

    /* the signal interrupted the read system call */
    /* if the process is in the read() system call, read returns -1
      However, if this occurs, errno is set to EINTR. We can check this  value
      and disregard the -1 value */
    if ((length < 0) && (errno != EINTR))
    {
        perror("error reading the command");
        exit(-1); /* terminate with error code of -1 */
    }

    // printf(">>%s<<", inputBuffer);
    for (i = 0; i < length; i++)
    { /* examine every character in the inputBuffer */
        switch (inputBuffer[i])
        {
        case ' ':
        case '\t': /* argument separators */
            if (start != -1)
            {
                args[ct] = &inputBuffer[start]; /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;

        case '\n': /* should be the final char examined */
            if (start != -1)
            {
                args[ct] = &inputBuffer[start];
                ct++;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL; /* no more arguments to this command */
            break;

        default: /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&')
            {
                *isBackgroundProcess = 1;
                inputBuffer[i - 1] = '\0';
            }
        }            /* end of switch */
    }                /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */
    argCount = ct;
    // for (i = 0; i <= ct; i++)
    // printf("args %d = %s\n", i, args[i]);
} /* end of setup routine */

int main()
{
    char inputBuffer[MAX_LINE];
    short isBackroudProcess; // equals 1 if a command is followed by &
    char *args[MAX_LINE / 2 + 1];
    int status;

    while (1)
    {
        isBackroudProcess = 0;
        setup(inputBuffer, args, &isBackroudProcess);
        printf("background process is : %d and argCount is %d\n", isBackroudProcess, argCount);

        /** the steps are:
        (1) fork a child process using fork()
        (2) the child process will invoke execv()
        (3) if background == 0, the parent will wait,
        otherwise it will invoke the setup() function again. */

        // Fork a child process
        int fork_pid = fork();
        if (fork_pid == -1)
        {
            printf("Error while creating child process!");
            return 1;
        }

        if (fork_pid)
        {
            // This is the parent process
            printf("\tHEROSLOG INFO:\tPARENT PROCESS: fork_pid: %d PID: %d PPID: %d\n", fork_pid, getpid(), getppid());
            if (isBackroudProcess)
            {
                printf("\tHEROSLOG INFO:\tBackground process initiated");
                // Should not wait and instantly prompt
            }
            else
            {

                if (wait())
                {
                }
                else
                {
                    printf("\tHEROSLOG INFO:\tChild process did not exit normally.\n");
                }
            }
        }
        else
        {
            // This is the child process

            printf("\tHEROSLOG INFO:\tCHILD PROCESS: fork_pid: %d PID: %d PPID: %d\n", fork_pid, getpid(), getppid());

            if (isBackroudProcess)
            {
                argCount--;
            }

            // ping -c 5 google.com


            printf("breakpoint1\n");

            execv("/bin/ping", args);

            printf("breakpoint2\n");

            perror("execv");

            printf("\tHEROSLOG INFO:\child process terminated .In child process\n");
        }
    }
    return 0;
}
