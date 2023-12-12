#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_STRING 300
#define MAX_LINE 256 // this is suposed to be 128 but i like to play with long strings
#define MAX_ARGS 32
#define MAX_BOOKMARKS 10
#define BOOKMARK_FILE ".bookmarks.txt"
int argCount = 0;
int bookmarkCount = 0;

// Structure to store bookmarks
struct Bookmark
{
    char *args[MAX_LINE / 2 + 1];
    int argCount;
};

struct Bookmark bookmarks[MAX_BOOKMARKS];

void searchFiles(char *searchString, char *currentPath, int recursive)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    if ((dir = opendir(currentPath)) == NULL)
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        char filePath[MAX_LINE];
        snprintf(filePath, sizeof(filePath), "%s/%s", currentPath, entry->d_name);

        if (stat(filePath, &fileStat) < 0)
        {
            perror("stat");
            continue;
        }

        if (S_ISDIR(fileStat.st_mode))
        {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            if (recursive)
            {

                searchFiles(searchString, filePath, recursive);
            }
        }
        else
        {

            const char *supportedFormats[] = {".c", ".C", ".h", ".H"};
            const char *fileExtension = strrchr(entry->d_name, '.');

            if (fileExtension != NULL && strcmp(fileExtension, ".") != 0)
            {
                int formatSupported = 0;
                for (size_t i = 0; i < sizeof(supportedFormats) / sizeof(supportedFormats[0]); i++)
                {
                    if (strcmp(fileExtension, supportedFormats[i]) == 0)
                    {
                        formatSupported = 1;
                    }
                }

                if (!formatSupported)
                {
                    continue;
                }
            }

            FILE *file = fopen(filePath, "r");
            if (file == NULL)
            {
                perror("fopen");
                continue;
            }

            char line[MAX_LINE];
            int lineNumber = 0;

            while (fgets(line, sizeof(line), file) != NULL)
            {
                lineNumber++;

                if (strstr(line, searchString) != NULL)
                {
                    printf("%d: %s -> %s", lineNumber, filePath, line);
                }
            }

            fclose(file);
        }
    }

    closedir(dir);
}

void saveBookmarksToFile()
{
    FILE *file = fopen(BOOKMARK_FILE, "w");

    if (file == NULL)
    {
        fprintf(stderr, "\tLOKISHELL LOG:\tError opening bookmark file for writing.\n");
        return;
    }

    for (int i = 0; i < bookmarkCount; i++)
    {
        for (int j = 0; j < bookmarks[i].argCount; j++)
        {
            fprintf(file, "%s ", bookmarks[i].args[j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void callKilloki()
{
    printf("\033[1;31m");
    printf("  ░░███╗░░██████╗░██╗░░██╗██╗██╗░░░░░██╗░░░░░░█████╗░██╗░░██╗██╗ \n");
    printf("  ░████║░░╚════██╗██║░██╔╝██║██║░░░░░██║░░░░░██╔══██╗██║░██╔╝██║  \n");
    printf("  ██╔██║░░░█████╔╝█████═╝░██║██║░░░░░██║░░░░░██║░░██║█████═╝░██║ \n");
    printf("  ╚═╝██║░░░╚═══██╗██╔═██╗░██║██║░░░░░██║░░░░░██║░░██║██╔═██╗░██║ \n");
    printf("  ███████╗██████╔╝██║░╚██╗██║███████╗███████╗╚█████╔╝██║░╚██╗██║  \n");
    printf("  ╚══════╝╚═════╝ ╚═╝  ╚═╝╚═╝╚══════╝╚══════╝ ╚════╝ ╚═╝  ╚═╝╚═╝  \n");

}



void loadBookmarksFromFile()
{
    FILE *file = fopen(BOOKMARK_FILE, "r");

    if (file == NULL)
    {
        fprintf(stderr, "\tLOKISHELL LOG:\tError opening bookmark file for reading.\n");
        return;
    }

    char line[MAX_LINE];
    int index = 0;

    while (fgets(line, sizeof(line), file) != NULL && index < MAX_BOOKMARKS)
    {
        // Remove newline character from the end of the line
        line[strcspn(line, "\n")] = '\0';

        // Tokenize the line into arguments
        char *token = strtok(line, " ");
        int argIndex = 0;

        // Create a new bookmark
        struct Bookmark newBookmark;

        while (token != NULL && argIndex < MAX_LINE / 2 + 1)
        {
            newBookmark.args[argIndex] = strdup(token);
            argIndex++;
            token = strtok(NULL, " ");
        }

        newBookmark.argCount = argIndex;
        bookmarks[index] = newBookmark;
        index++;
    }

    bookmarkCount = index;

    fclose(file);
}

void removeFirstChar(char *str)
{
    if (str != NULL && str[0] != '\0')
    {
        memmove(str, str + 1, strlen(str));
    }
}

void removeLastChar(char *str)
{
    if (str != NULL && str[0] != '\0')
    {
        size_t len = strlen(str);
        if (len > 0)
        {
            str[len - 1] = '\0';
        }
    }
}

void deleteBookmark(int index)
{
    if (index >= 0 && index < bookmarkCount)
    {
        // Free the memory allocated for the command
        for (int i = 0; i < bookmarks[index].argCount; i++)
        {
            free(bookmarks[index].args[i]);
        }

        // Shift the remaining bookmarks up in the list
        for (int i = index; i < bookmarkCount - 1; i++)
        {
            bookmarks[i] = bookmarks[i + 1];
        }

        bookmarkCount--;

        printf("\tLOKISHELL LOG:\tBookmark deleted.\n");
    }
    else
    {
        printf("\tLOKISHELL LOG:\tInvalid bookmark index.\n");
    }
}

// Signal Handler for SIGTSTP, executed when ctrl-z is pressed
void sighandler(int sig_num)
{
    // Reset handler to catch SIGTSTP next time
    signal(SIGTSTP, sighandler);
    printf("\n=Ctrl+Z pressed\n");
}

// Calls exit if ctrl-D is entered and reads args
void setup(char inputBuffer[], char *args[], short *isBackgroundProcess)
{

    int length; // # of characters in the command line
    int i;      // loop index for accessing inputBuffer array
    int start;  // index where beginning of next command parameter is
    int ct = 0; // index of where to place the next parameter into args[]

    // Use ANSI escape code to set text color to green
    printf("\033[1;31m");
    printf("lokishell: ");
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

void forkProcess(char *args[], short isBackgroundProcess)
{
    int status;
    char fullPath[255];

    strcpy(fullPath, "/bin/");
    strcat(fullPath, args[0]);

    int fork_pid = fork();
    if (fork_pid == -1)
    {
        printf("\tLOKISLOG ERROR:\tError while creating child process!");
        return 1;
    }

    if (fork_pid)
    {
        // This is the parent process
        // printf("\tLOKISLOG INFO:\tPARENT PROCESS: fork_pid: %d PID: %d PPID: %d\n", fork_pid, getpid(), getppid());
        if (isBackgroundProcess)
        {
            // printf("\tLOKISLOG INFO:\tBackground process initiated");
            // Should not wait and instantly prompt
        }
        else
        {

            if (waitpid(fork_pid, &status, 0) > 0)
            {
                // printf("\tLOKISLOG INFO:\tForeground process %d exited normally.\n", fork_pid);
            }
            else
            {
                printf("\tLOKISLOG INFO:\tForeground process %d did not exit normally.\n", fork_pid);
            }
        }
    }
    else
    {
        // This is the child process

        // printf("\tLOKISLOG INFO:\tCHILD PROCESS: fork_pid: %d PID: %d PPID: %d\n", fork_pid, getpid(), getppid());

        if (isBackgroundProcess)
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

        // printf("\tLOKISLOG INFO:\child process terminated .In child process\n");
    }
}

int main()
{

    loadBookmarksFromFile();
    signal(SIGTSTP, sighandler);
    char inputBuffer[MAX_LINE];
    short isBackgroundProcess; // equals 1 if a command is followed by &
    char *args[MAX_LINE / 2 + 1];

    while (1)
    {
        isBackgroundProcess = 0;
        setup(inputBuffer, args, &isBackgroundProcess);

        // This part is for handling strings
        int startOfString = 0;
        for (int i = 1; i < argCount; i++)
        {
            // Check if the argument starts with "
            if (strncmp(args[i], "\"", 1) == 0)
            {
                char *string = malloc(MAX_STRING); // Allocate memory
                if (string == NULL)
                {
                    // Handle allocation failure
                    exit(EXIT_FAILURE);
                }

                startOfString = i;

                // Remove the opening double quote if it's not the only character
                if (strlen(args[i]) > 1)
                {
                    removeFirstChar(args[i]);
                }

                // Initialize the string to an empty string
                string[0] = '\0';

                // Append the argument to the string
                strcat(string, args[i]);

                // Check if the string ends with a double quote
                size_t len = strlen(args[i]);
                if (len > 0 && args[i][len - 1] == '\"')
                {
                    removeLastChar(args[i]);
                    strcat(string, args[i]);
                }
                else
                {
                    // Handle case where the string ends with a space or there are more arguments
                    while (++i < argCount)
                    {
                        len = strlen(args[i]);
                        if (len > 0 && args[i][len - 1] == '\"')
                        {
                            removeLastChar(args[i]);
                            strcat(string, " ");
                            strcat(string, args[i]);
                            break;
                        }
                        else
                        {
                            strcat(string, " ");
                            strcat(string, args[i]);
                        }
                    }
                }
                args[startOfString] = strdup(string);
                args[startOfString + 1] = NULL;
                free(string);
            }
        }

        if (!strcmp(args[0], "exit") || !strcmp(args[0], "killoki"))
        {
            printf("\nshell exited\n");
            exit(3);
            saveBookmarksToFile();
        }

        // Check if the command is a bookmark command
        if (!strcmp(args[0], "bookmark"))
        {
            if (argCount >= 2)
            {
                if (!strcmp(args[1], "-l"))
                {
                    // List bookmarks
                    for (int i = 0; i < bookmarkCount; i++)
                    {
                        printf("%d \"", i);
                        for (int j = 0; j < bookmarks[i].argCount; j++)
                        {
                            printf("%s ", bookmarks[i].args[j]);
                        }
                        printf("\"\n");
                    }
                }
                else if (!strcmp(args[1], "-i") && argCount >= 3)
                {
                    // Execute bookmark by index
                    int index = atoi(args[2]);
                    if (!strcmp(bookmarks[index].args[bookmarks[index].argCount - 1], "&"))
                    {
                        isBackgroundProcess = 1;
                    }
                    forkProcess(bookmarks[index].args, isBackgroundProcess);
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
                    // The procedure for adding bookmarks
                    char *bookmarkCommand[MAX_LINE / 2 + 1];

                    for (int i = 1, j = 0; i < argCount; i++, j++)
                    {
                        bookmarkCommand[j] = strdup(args[i]);

                        // Remove quotes from the first argument
                        if (i == 1)
                        {
                            removeFirstChar(bookmarkCommand[j]);
                        }
                        if (i == argCount - 1)
                        {
                            removeLastChar(bookmarkCommand[j]);
                        }
                    }

                    for (int i = 0; i < argCount - 1; i++)
                    {
                        bookmarks[bookmarkCount].args[i] = strdup(bookmarkCommand[i]);
                    }
                    bookmarks[bookmarkCount].argCount = argCount - 1;
                    bookmarkCount++;

                    printf("\tLOKISHELL LOG:\tAdded bookmark\n");
                }
            }
            else
            {
                printf("Invalid bookmark command. Usage: bookmark [options] <command>\n");
            }
        }
        else
        {
            forkProcess(args, isBackgroundProcess);
            atexit(saveBookmarksToFile);
        }

        if (!strcmp(args[0], "search"))
        {
            if (argCount >= 2)
            {
                int recursive = 0;
                const char *searchString = args[1];

                if (argCount >= 3 && !strcmp(args[1], "-r"))
                {
                    recursive = 1;
                    searchString = args[2];
                }

                char currentPath[1000];
                if (getcwd(currentPath, sizeof(currentPath)) == NULL)
                {
                    perror("getcwd");
                }

                searchFiles(searchString, currentPath, recursive);
            }
            else
            {
                printf("Invalid search command. Usage: search [-r] <search_string>\n");
            }
        }
        if (!strcmp(args[0], "13killoki"))
        {
            callKilloki();
        }
    
    }
    return 0;
}
