#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

// should single external commands run in their own thread?
// what should format of bin be? '/bin:/etcetc' ?
// 

char *path;

// first argument is command, second is the arguments for the command
void *extcmd(char *cmd, char *ext) {
  // first, have to put ext into an array of strings
  char **args = NULL;
  int spaces = 0;
  ext = strtok(ext, " ");
  //printf("%s", ext);
  while (ext) {
    args = realloc(args, sizeof (char*) * ++spaces);

    args[spaces - 1] = ext;
    // and move on to the next token
    ext = strtok(NULL, " ");
    //printf("%s", ext);
  }
  //printf("%s", args[1]);
  // since strtok overwrites old string, have to make copy
  char *tempEnv;
  tempEnv = malloc(strlen(path)+1); 
  strcpy(tempEnv, path);
  strtok(tempEnv, ":");
  // have to figure out how exactly to split path paths
  /*char *splitArgs = strtok(tempEnv, ":"); // strtok modifies 'buffer' ?

  while (splitInput != NULL) {
    // if command does not execute
    if (execv(strcat(splitInput, cmd), args) == -1) {
      splitInput = strtok(NULL, ":");
    }
  }*/
  

  free(tempEnv);
  return 0;
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
      // need newline to be delimiter b/c if user just hits 'exit' and then an enter immediately after, 
      //strtok won't split up the result correctly
      char *splitInput = strtok(buffer, " \t\n"); 

      // built-in commands
      if (strcmp(splitInput, "exit") == 0) {
        exit(0);
      }
      else if (strcmp(splitInput, "cd") == 0) {
        int argCount = 0;
        
        while (splitInput) {
          if (argCount++ > 1) {
            errorPrint();
            break;
          }
          chdir(splitInput);
          splitInput = strtok(NULL, " ");
        }
      }
      else if (strcmp(splitInput, "path") == 0) {
        printf("path cmd");
      }
      else {
        char *cmd = malloc(strlen(splitInput));
        strcpy(cmd, splitInput);
        splitInput = strtok(NULL, "");
        printf("%s | %s", cmd, splitInput);
        extcmd(cmd, splitInput);
      }
    }
}

int main(int argc, char *args[]) {
  path = "/bin"; // our initial path
  // normal mode
  if (argc == 1) {
    interactiveMode();
  }
  // batch mode
  if (argc == 2) {
    
  }
}