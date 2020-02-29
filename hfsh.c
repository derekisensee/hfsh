#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

char path[9068]="/bin:";

void errorPrint() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

// first argument is command, second is the arguments for the command
void *extcmd(char *cmd, char *ext, int waiting, char *file) {
  // first, have to put ext into an array of strings
  char **args = NULL;
  int spaces = 0;
  ext = strtok(ext, " \n");
  args = realloc(args, sizeof (char*) * ++spaces);
  args[0] = cmd;
  while (ext != NULL) {
    args = realloc(args, sizeof (char*) * ++spaces);
    args[spaces - 1] = ext;
    // and move on to the next token
    ext = strtok(NULL, " ");
  }
  args = realloc(args, sizeof (char*) * ++spaces);
  args[spaces - 1] = NULL;
  // uncomment to print args 
  /*
  int i = 0;
  while (args[i] != NULL) {
    printf("%s\n", args[i++]);
  }
  printf("%s", args[i]);  
  */
  // since strtok overwrites old string, have to make copy
  char *tempEnv;
  tempEnv = malloc(strlen(path)+1); 
  strcpy(tempEnv, path);
  //char *splitPaths = strtok(tempEnv, ":\n"); // strtok modifies 'buffer' ?
  char *splitPaths = strsep(&tempEnv, ":"); // strtok modifies 'buffer' ?
  char slash[1024] = {'/', '\0'};
  strcat(slash, cmd); // slash now holds "/cmd"

  int found = 0;
  while (splitPaths != NULL) {
    // use access() to find program
    char spCopy[1024] = ""; // seems to reset spCopy between path checks
    strcat(spCopy, splitPaths);
    strcat(spCopy, slash);
    
    
    if (access(spCopy, F_OK) == 0) {
      if (waiting == 1) { // this waiting check was the final piece i needed...
        found = 1;
        int status;
        pid_t wpid = fork();
        if (wpid == 0) {
          if (file != NULL) { // if we have a file, write to that instead
            int fp = open(file, O_RDWR | O_CREAT | O_TRUNC, 0666); 
            dup2(fp, 1);
            close(fp);
          }
          execv(spCopy, args);
        } 
        else { 
          wait(&status);
        }
        while ((wpid = wait(&status)) > 0); // parent waits for all children to finish
        break;
      }
      else { // run without waiting
        if (file != NULL) { // if we have a file, write to that instead
            int fp = open(file, O_RDWR | O_CREAT | O_TRUNC, 0666);
            dup2(fp, 1);
            close(fp);
          }
          execv(spCopy, args);
      }
    }
    else {
      splitPaths = strsep(&tempEnv, ":");
    }
  }
  if (found == 0) {
    errorPrint();
  }
  return 0;
}

void cmdChunk(char *splitInput, char *file, int waiting) {
  if (strcmp(splitInput, "exit") == 0) {
    splitInput = strtok(NULL, " ");
    if (splitInput != NULL) {
      //printf("huh?");
      errorPrint();
    }
    else
      exit(0);
  }
  else if (strcmp(splitInput, "cd") == 0) {
    //int argCount = 0;
    char dir[4096] = "";
    splitInput = strtok(NULL, " \n\r");
    // make sure we get at least 1 arg
    //printf("%s", splitInput);
    if (splitInput != NULL) {
      strcpy(dir, splitInput);
    }
    splitInput = strtok(NULL, " ");
    if (splitInput == NULL) {
      if (chdir(dir) == -1) {
        errorPrint();
      }
    } 
    else {
      //printf("Frick");
      errorPrint();
    }
  }
  else if (strcmp(splitInput, "path") == 0) {
    strcpy(path, "");
    while ((splitInput = strtok(NULL, " \n")) != NULL) {
      strcat(path, splitInput);
      strcat(path, ":");
    }
    //printf("%s", path);
  }
  else if (splitInput == NULL) {
    // do nothing ??????????????
  }
  else {
    char *cmd = malloc(strlen(splitInput));
    strcpy(cmd, splitInput);
    splitInput = strtok(NULL, "");
    extcmd(cmd, splitInput, waiting, file);
  }
}

void chunkHandle(char *currCmd, int waiting) {
  // redirection first
  char *chunkCopy = malloc(strlen(currCmd)+1);
  strcpy(chunkCopy, currCmd);
  char *redir = strtok(chunkCopy, ">"); 
  int err = 0;

  char *firstChunk = malloc(strlen(redir)+1);
  strcpy(firstChunk, redir);

  char *file = NULL;

  // if we have a file to redirect to
  if (strstr(currCmd, ">")) {
    redir = strtok(NULL, " \n");
    if (redir != NULL) {
      file = malloc(strlen(redir)+1);
      strcpy(file, redir);
    }
    else {
      err = 1;
    }
  }

  if (err == 0) {
    if ((redir = strtok(NULL, " ")) == NULL) { // make sure we only get 1 file
      char *splitInput = strtok(firstChunk, " \t\n");
      if (splitInput == NULL) {
      }
      cmdChunk(splitInput, file, waiting);
    }
    else {
      errorPrint();
    }
  }
  else {
    errorPrint();
  }
}

void handleInput(char *buffer) {
  // need newline to be delimiter b/c if user just hits 'exit' and then an enter immediately after, 
  //strtok won't split up the result correctly
  char *buffCheck = malloc(strlen(buffer) + 1);
  strcpy(buffCheck, buffer);
  int len = strlen(buffer);
  buffCheck[len-1] = '\0';
  // check whitespace case
  char *whitespaceCheck = malloc(len+1);
  strcpy(whitespaceCheck, buffCheck);
  char *whitespaceSplit = strtok(whitespaceCheck, " & > \t\n");
  if (whitespaceSplit == NULL) { // if blank input is given or if only '&' or '>' is given
  }
  else {
    // ---- splits up '&'s ------
    if (strstr(buffer, "&")) {
      char *currCmd = strsep(&buffCheck, "&");
      pid_t p;
      int status;
      while (currCmd != NULL) {
        if ((p = fork()) == 0) {
          chunkHandle(currCmd, 0);
        }
        else {
          currCmd = strsep(&buffCheck, "&");
        }
      }
      while ((p = wait(&status)) > 0); // parent waits for all children to finish
      if (currCmd != NULL) {
        chunkHandle(currCmd, 0);
      }
    }
    else { // single, 'regular' commands
      chunkHandle(buffCheck, 1);
    }
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
  
  while (getline(&buffer, &buffSize, fp) != -1) {
    handleInput(buffer);
  }
}

int main(int argc, char *args[]) {
  // normal mode
  if (argc == 1) {
    interactiveMode();
  }
  // batch mode
  if (argc == 2) {
    FILE *fp = fopen(args[1], "r");
    if (fp != NULL) {
      batchMode(fp);
      fclose(fp);
    }
    else {
        errorPrint();
        exit(1);
    }
  }
  if (argc > 2) {
    errorPrint();
    exit(1);
  }
}