#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

char *path;

void *extcmd(char *cmd, char *ext) {
  // since strtok overwrites old string, have to make copy
  char *tempEnv;
  tempEnv = malloc(strlen(path)+1); 
  strcpy(tempEnv, path);
  strtok(tempEnv, ";");
  // have to figure out how exactly to split path paths
  char *splitInput = strtok(tempEnv, ";"); // strtok modifies 'buffer' ?

  while (splitInput != NULL) {
    // if command does not execute
    if (execv(strcat(splitInput, cmd), ext) == -1) {
      splitInput = strtok(NULL, ";");
    }
  }
  

  free(tempEnv);
}

void errorPrint() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message)); 
}

void interactiveMode() {
  int running = 1;

    while (running == 1) {
      // vars
      char *buffer;
      size_t buffSize = 32;
      //size_t input;
      buffer = (char *)malloc(buffSize * sizeof(char));

      // check for any buffer errors?
      // --------------------

      printf("hfsh> ");
      getline(&buffer, &buffSize, stdin);
      char *splitInput = strtok(buffer, " \t\n"); // strtok modifies 'buffer' ?

      // built-in commands
      if (strcmp(splitInput, "exit") == 0) {
        exit(0);
      }

      if (strcmp(splitInput, "cd") == 0) {
        int argCount = 0;
        
        while (splitInput) {
          if (argCount++ > 1) {
            errorPrint();
            break;
          }
          chdir(splitInput);
        }
      }

      if (strcmp(splitInput, "path")) {

      }

      else {
        extcmd(splitInput, strtok(NULL, ""));
      }

      // -----------------
    }
}

int main(int argc, char *args[]) {
  path = "/bin";
  // normal mode
  if (argc == 1) {
    interactiveMode();
  }
  // batch mode
  if (argc == 2) {
    
  }
}