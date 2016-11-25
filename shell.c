// GoSh - GoShell - Gogol's Shell

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LEN 1024
#define DELIMS " \t\n"

void tildefy(char* path, char* home, size_t hLen)
{
    char *homePos, *newPath;

    // string for new path
    newPath = (char*) malloc(MAX_LEN*sizeof(char));

    // checking if initial path exists in current path
    homePos = strstr(path, home);

    if(homePos != NULL)
    {
        // modifying path with tilde instead of home path
        newPath[0] = '~';
        newPath[1] = '\0';

        homePos += hLen;
        strcat(newPath, homePos);
        strcpy(path, newPath);
    }

    free(newPath);

    return;
}

char* getInput(void)
{
    char *input;
    size_t inputSize = 0;

    // getting input
    getline(&input, &inputSize, stdin);

    return input;
}

char** splitLine(char* line)
{
    int i;
    char **splitLines, *token;

    // array of strings for command and arguments
    splitLines = (char**) malloc(MAX_LEN*sizeof(char*));

    // tokenizing w.r.t whitespace
    token = strtok(line, DELIMS);

    // storing in array of strings
    for(i = 0; token != NULL; i++)
    {
        splitLines[i] = token;
        token = strtok(NULL, DELIMS);
    }
    splitLines[i] = NULL;

    return splitLines;
}

void executeInput(char** sInput)
{
    int chk, quit, procID, waitID;

    // forking process
    procID = fork();

    // error handling
    if(procID < 0)
    {
        perror("GoSh::Error");
    }

    // child process
    else if(procID == 0)
    {
        // running command with arguments
        chk = execvp(sInput[0], sInput);

        // execvp error handling
        if(chk == -1)
        {
            perror("GoSh::Error");
        }

        // exit the child
        exit(1);
    }

    // parent process
    else
    {
        while(1) // waits for the child to exit
        {
            waitID = waitpid(procID, &quit, WUNTRACED); // checks status of child

            // error handling for child process
            if(waitID == -1)
            {
                perror("GoSh::Error");
                break;
            }

            // macros indicate when child process has ended
            if(WIFEXITED(quit) || WIFSIGNALED(quit) || WIFSTOPPED(quit))
            {
                break;
            }
        }
    }

    return;
}

void changeDir(char** sInput, char* home)
{
    int chk;

    // error handling for cd
    if(sInput[1] == NULL)
    {
        printf("GoSh::Error: cd takes 1 argumenti\n");
        return;
    }

    if(sInput[2] != NULL)
    {
        printf("GoSh::Error: cd takes 1 argumenti\n");
        return;
    }

    // changing directory with exception for "cd ~"
    if(strcmp(sInput[1], "~") == 0)
    {
        chk = chdir(home);
    }

    else
    {
        chk = chdir(sInput[1]);
    }

    // more error handling
    if(chk != 0)
    { 
        perror("GoSh::Error");
    }

    return;
}

void echoText(char** sInput, char* text)
{
    int i;

    // printing input string minus command name
    for(i = 1; sInput[i] != NULL; i++)
    {
        printf("%s ", sInput[i]);
    }
    printf("\n");

    return;
}

void currentDir(void)
{
    char path[MAX_LEN];

    // getting path and printing
    getcwd(path, MAX_LEN);

    printf("%s\n", path);    

    return;
}

int checkBuiltins(char** sInput, char* home, char* quotedText)
{
    int i, j;
    char *builtins[] = {"exit", "cd", "echo", "pwd"};

    // checking against builtins and executing corresponding builtin function
    if(strcmp(sInput[0], builtins[0]) == 0)
    {
        return 0;
    }

    else if(strcmp(sInput[0], builtins[1]) == 0)
    {
        changeDir(sInput, home);
    }

    else if(strcmp(sInput[0], builtins[2]) == 0)
    {
        echoText(sInput, quotedText);
    }

    else if(strcmp(sInput[0], builtins[3]) == 0)
    {
        currentDir();
    }

    else
    {
        return 1;
    }

    return 2;
}

char** splitInput(char *input)
{
    int i;
    char* commands, **splitCommands;

    // array of strings for the commands
    splitCommands = (char**) malloc(MAX_LEN*sizeof(char*));

    // tokenizing w.r.t semi colons
    commands = strtok(input, ";");

    // storig in array of strings
    for(i = 0; commands != NULL; i++)
    {
        splitCommands[i] = commands;
        commands = strtok(NULL, ";");
    }
    splitCommands[i] = NULL;

    return splitCommands;
}

int main (int argc, char **argv)
{
    int run = 1, i;
    char *input, **tokens, **commands, pInput[MAX_LEN];
    char username[MAX_LEN], hostname[MAX_LEN]; 
    char path[MAX_LEN], *homeDir, home[MAX_LEN], cHome[MAX_LEN];

    // getting username and hostname
    getlogin_r(username, MAX_LEN);
    gethostname(hostname, MAX_LEN);

    // storing initial directory (home)
    getcwd(path, MAX_LEN);
    strcpy(home, path);

    while(run) // until exit command given
    {
        // new path for display requirement
        getcwd(path, MAX_LEN);

        strcpy(cHome, home);

        // using tilde for home relative paths
        tildefy(path, cHome, strlen(home));

        // display requirement
        printf("<%s@%s:%s> ", username, hostname, path);

        // getting string input
        input = getInput();
        strcpy(pInput, input);

        // splitting into commands by semi colon separation
        commands = splitInput(input);

        // executing each command
        for(i = 0; commands[i] != NULL; i++)
        {
            // tokenizing the command string
            tokens = splitLine(commands[i]);

            //checking against builtins list ("cd", "echo", etc);
            run = checkBuiltins(tokens, home, pInput);

            if(!run)
            {
                break;
            }

            else if(run == 2)
            {
                continue;
            }

            // executing if not a builtin
            else
            {
                executeInput(tokens);
            }

            free(tokens);
        }

        free(commands);
    }
    return 0;
}
