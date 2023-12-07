#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MAX_LINE 80

// Calls exit if ctrl-D is entered
// Reads args
void setup(char inputBuffer[], char *args[], short *isBackgroundProcess)
{
    int length; // # of characters in the command line
    int i;      // loop index for accessing inputBuffer array
    int start;  // index where beginning of next command parameter is
    int ct = 0; // index of where to place the next parameter into args[]

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

    printf(">>%s<<", inputBuffer);
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

    for (i = 0; i <= ct; i++)
        printf("args %d = %s\n", i, args[i]);
} /* end of setup routine */

int main(void)
{

    char inputBuffer[MAX_LINE];
    short isBackroudProcess; // equals 1 if a command is followed by &
    char *args[MAX_LINE / 2 + 1];

    while (1)
    {
        isBackroudProcess = 0;
        printf("heroshell: ");
        setup(inputBuffer, args, &isBackroudProcess);

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

        // This is the parent process
        if (fork_pid)
        {
            printf("fork_pid: %d PID: %d PPID: %d\n", fork_pid, getpid(), getppid());
        }
        else //This is the child process
        {
            printf("fork_pid: %d PID: %d PPID: %d\n", fork_pid, getpid(), getppid());
        }
    }
    return 0;
}
