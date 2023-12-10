#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>

#define MAX_LINE 80
#define MAX_BOOKMARKS 10
#define BOOKMARK_FILE ".bookmarks.txt"
int argCount = 0;
int bookmarkCount = 0;

// Structure to store bookmarks
struct Bookmark
{
    char *args[MAX_LINE];
};

struct Bookmark bookmarks[MAX_BOOKMARKS];

void saveBookmarks()
{
    FILE *file = fopen(BOOKMARK_FILE, "w");
    if (file == NULL)
    {
        perror("Error opening bookmarks file for writing");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < bookmarkCount; i++)
    {
        fprintf(file, "%s\n", bookmarks[i].args);
    }

    fclose(file);
}

void loadBookmarks()
{
    FILE *file = fopen(BOOKMARK_FILE, "r");
    if (file == NULL)
    {
        return;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        addBookmark(line);
    }

    fclose(file);
}

void addBookmark(char *command)
{
    if (bookmarkCount < MAX_BOOKMARKS)
    {
        strcpy(bookmarks[bookmarkCount].args, command);
        bookmarkCount++;
    }
    else
    {
        printf("\tHEROSHEL LOG:\tBookmark limit reached. Unable to add more bookmarks.\n");
    }
}

void listBookmarks()
{
    printf("Bookmarks:\n");
    for (int i = 0; i < bookmarkCount; i++)
    {
        printf("%d \"%s\"\n", i, bookmarks[i].args);
    }
}

void deleteBookmark(int index)
{
    if (index >= 0 && index < bookmarkCount)
    {
        for (int i = index; i < bookmarkCount - 1; i++)
        {
            strcpy(bookmarks[i].args, bookmarks[i + 1].args);
        }
        bookmarkCount--;
    }
    else
    {
        printf("Invalid bookmark index.\n");
    }
}

// Signal Handler for SIGTSTP, executed when ctrl-z is pressed
void sighandler(int sig_num)
{
    // Reset handler to catch SIGTSTP next time
    signal(SIGTSTP, sighandler);
    printf("\n=Ctrl+Z pressed\n");
}

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
    loadBookmarks();
    signal(SIGTSTP, sighandler);
    char inputBuffer[MAX_LINE];
    short isBackroudProcess; // equals 1 if a command is followed by &
    char *args[MAX_LINE / 2 + 1];
    int status;

    while (1)
    {
        isBackroudProcess = 0;
        setup(inputBuffer, args, &isBackroudProcess);

        if (!strcmp(args[0], "exit"))
        {
            exit(3);
            saveBookmarks();
        }

        // Check if the command is a bookmark command
        if (!strcmp(args[0], "bookmark"))
        {
            if (argCount >= 2)
            {
                if (!strcmp(args[1], "-l"))
                {
                    // List bookmarks
                    listBookmarks();
                }
                else if (!strcmp(args[1], "-i") && argCount >= 3)
                {
                    // Execute bookmark by index
                    int index = atoi(args[2]);
                }
                else if (!strcmp(args[1], "-d") && argCount >= 3)
                {
                    // Delete bookmark by index
                    int index = atoi(args[2]);
                    deleteBookmark(index);
                    printf("Bookmark deleted.\n");
                }
                else
                {

                    bookmarkCount++;
                }
            }
            else
            {
                printf("Invalid bookmark command. Usage: bookmark [options] <command>\n");
            }
        }
        else
        {

            char fullPath[255];

            strcpy(fullPath, "/bin/");
            strcat(fullPath, args[0]);

            int fork_pid = fork();
            if (fork_pid == -1)
            {
                printf("\tHEROSLOG ERROR:\tError while creating child process!");
                return 1;
            }

            if (fork_pid)
            {
                // This is the parent process
                // printf("\tHEROSLOG INFO:\tPARENT PROCESS: fork_pid: %d PID: %d PPID: %d\n", fork_pid, getpid(), getppid());
                if (isBackroudProcess)
                {
                    // printf("\tHEROSLOG INFO:\tBackground process initiated");
                    // Should not wait and instantly prompt
                }
                else
                {

                    if (waitpid(fork_pid, &status, 0) > 0)
                    {
                        // printf("\tHEROSLOG INFO:\tForeground process %d exited normally.\n", fork_pid);
                    }
                    else
                    {
                        printf("\tHEROSLOG INFO:\tForeground process %d did not exit normally.\n", fork_pid);
                    }
                }
            }
            else
            {
                // This is the child process

                // printf("\tHEROSLOG INFO:\tCHILD PROCESS: fork_pid: %d PID: %d PPID: %d\n", fork_pid, getpid(), getppid());

                if (isBackroudProcess)
                {
                    argCount--;
                    args[argCount] = NULL;
                    // Redirect standard input, output, and error to /dev/null so output is not printed
                    freopen("/dev/null", "r", stdin);
                    freopen("/dev/null", "w", stdout);
                    freopen("/dev/null", "w", stderr);
                }

                if (!strcmp(args[argCount - 2], ">"))
                {
                    freopen(args[argCount - 1], "w", stdout);
                    args[argCount - 2] = NULL;
                    execv(fullPath, args);
                }
                else if (!strcmp(args[argCount - 2], ">>"))
                {
                    freopen(args[argCount - 1], "a", stdout);
                    args[argCount - 2] = NULL;
                    execv(fullPath, args);
                }
                else if (!strcmp(args[argCount - 2], "<"))
                {
                    freopen(args[argCount - 1], "r", stdin);
                    args[argCount - 2] = NULL;
                    execv(fullPath, args);
                }
                else if (!strcmp(args[argCount - 2], "2>"))
                {
                    freopen(args[argCount - 1], "w", stderr);
                    args[argCount - 2] = NULL;
                    execv(fullPath, args);
                }
                else
                {
                    execv(fullPath, args);
                }

                // ping -c 5 google.com

                perror("execv");

                // printf("\tHEROSLOG INFO:\child process terminated .In child process\n");
            }
        }
        atexit(saveBookmarks);
    }
    return 0;
}
