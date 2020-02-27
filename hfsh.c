#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// should single external commands run in their own thread? 
 // don't wait if given & - for last cmd without a & after it, do execv and make hfsh wait. so last cmd is in "foreground"

// need to make path a char[] ??
char *path;

// first argument is command, second is the arguments for the command
void *extcmd(char *cmd, char *ext, int waiting) {
  // first, have to put ext into an array of strings
  char **args = NULL;
  int spaces = 0;
  ext = strtok(ext, " \n");
  args = realloc(args, sizeof (char*) * ++spaces);
  args[0] = cmd;
  while (ext) {
    args = realloc(args, sizeof (char*) * ++spaces);
    args[spaces - 1] = ext;
    // and move on to the next token
    ext = strtok(NULL, " "); // ??? need to delimit by newline/carriage return
  }
  // since strtok overwrites old string, have to make copy
  char *tempEnv;
  tempEnv = malloc(strlen(path)+1); 
  strcpy(tempEnv, path);
  char *splitPaths = strtok(tempEnv, ":\n"); // strtok modifies 'buffer' ?
  char slash[1024] = {'/', '\0'};
  //splitPaths = strtok(NULL, " ");
  strcat(slash, cmd); // slash now holds "/cmd"
  //char spCopy[] = ""; 
  //printf("copy: %s\n", slash);

  //printf("%s %s %s", args[0], args[1], args[2]);

  while (splitPaths != NULL) {
    // use access() to find program
    //char *currPath = malloc(strlen(strcat(splitPaths, slash)));
    char spCopy[1024] = ""; // seems to reset spCopy between path checks
    strcat(spCopy, splitPaths);
    //printf("1212: %s\n", slash);
    strcat(spCopy, slash);
    //char *currPath = strcat(spCopy, slash); // doesn't like this, splitPaths not working?
    
    //printf("copy: %s\n", spCopy);
    if (access(spCopy, F_OK) == 0) {
      //printf("%s", spCopy);
      int status;
      //pid_t w;
      if (fork() == 0) {
        execv(spCopy, args);
      } 
      else if (waiting == 1) {
        wait(&status);
      }
      break;
    }
    else {
      splitPaths = strtok(NULL, " ");
    }
  }
  return 0;
}

void errorPrint() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message)); 
}

void handleInput(char *buffer) {
  // need newline to be delimiter b/c if user just hits 'exit' and then an enter immediately after, 
  //strtok won't split up the result correctly
  char *splitInput = strtok(buffer, " \t\n"); 

  // built-in commands
  if (strcmp(splitInput, "exit") == 0) {
    exit(0);
  }
  else if (strcmp(splitInput, "cd") == 0) {
    int argCount = 0;
    char dir[] = "";
    splitInput = strtok(NULL, " \n");
    if (splitInput != NULL) {
      strcpy(dir, splitInput);
    } 
    else {
        //errorPrint();
        argCount = 99; // make error happen
    }

    while (splitInput) {
      if (argCount++ > 1) {
        errorPrint();
        break;
      }
      splitInput = strtok(NULL, " ");
    }
    if (argCount == 1) {
      if (chdir(dir) == -1) {
        errorPrint();
      }
    } else
      errorPrint();
  }
  else if (strcmp(splitInput, "path") == 0) {
    //printf("path cmd");
  }
  else if (strcmp(buffer, "\r") == 0) {
    // do nothing ??????????????
  }
  else {
    char *cmd = malloc(strlen(splitInput));
    strcpy(cmd, splitInput);
    splitInput = strtok(NULL, "");
    //printf("%s | %s", cmd, splitInput);
    // have to: check for &, >, respond appropriately
    extcmd(cmd, splitInput, 1);
  }
}

void interactiveMode() {
  int running = 1;

  while (running == 1) {
    // vars
    char *buffer;
    size_t buffSize = 32;
    //size_t input;
    buffer = (char *)malloc(buffSize * sizeof(char));
    printf("hfsh> ");
    getline(&buffer, &buffSize, stdin);
    // check for any buffer errors?
    handleInput(buffer);
  }
}

void batchMode(FILE *fp) {
  char *buffer;
  size_t buffSize = 32;
  //size_t input;
  buffer = (char *)malloc(buffSize * sizeof(char));
  getline(&buffer, &buffSize, fp);
  while (buffer != NULL ) {
    handleInput(buffer);
    getline(&buffer, &buffSize, fp);
  }
}

int main(int argc, char *args[]) {
  path = "/bin:"; // our initial path
  // normal mode
  if (argc == 1) {
    interactiveMode();
  }
  // batch mode
  if (argc == 2) {
    FILE *fp = fopen(args[1], "r");
    batchMode(fp);
  }
}