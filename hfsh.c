#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// should single external commands run in their own thread? 
 // don't wait if given & - for last cmd without a & after it, do execv and make hfsh wait. so last cmd is in "foreground"
 // 

char *path;

// first argument is command, second is the arguments for the command
void *extcmd(char *cmd, char *ext) {
  // first, have to put ext into an array of strings
  char **args = NULL;
  int spaces = 0;
  ext = strtok(ext, " ");
  args = realloc(args, sizeof (char*) * ++spaces);
  args[0] = cmd;
  //printf("%s", ext);
  while (ext) {
    args = realloc(args, sizeof (char*) * ++spaces);

    args[spaces - 1] = ext;
    // and move on to the next token
    ext = strtok(NULL, " \n\r"); // ??? need to delimit by newline/carriage return
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

  printf("%s %s %s", args[0], args[1], args[2]);

  while (splitPaths != NULL) {
    // use access() to find program
    //char *currPath = malloc(strlen(strcat(splitPaths, slash)));
    char spCopy[1024];
    strcat(spCopy, splitPaths);
    //printf("1212: %s\n", slash);
    strcat(spCopy, slash);
    //char *currPath = strcat(spCopy, slash); // doesn't like this, splitPaths not working?
    
    //printf("copy: %s\n", spCopy);
    if (access(spCopy, F_OK) == 0) {
      if (fork() == 0) {
        execv(spCopy, args);
      }
      break;
    }
    else {
      splitPaths = strtok(NULL, " ");
    }
  }
  /*else if (access(currPath, F_OK) == 0) {
    printf("%s", currPath);
  }*/
  //free(tempEnv);
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
          chdir(splitInput); // check if chdir returns error
          splitInput = strtok(NULL, " ");
        }
      }
      else if (strcmp(splitInput, "path") == 0) {
        //printf("path cmd");
      }
      else if (strcmp(buffer, "\n") == 0) {
        // do nothing ??????????????
      }
      else {
        char *cmd = malloc(strlen(splitInput));
        strcpy(cmd, splitInput);
        splitInput = strtok(NULL, "");
        //printf("%s | %s", cmd, splitInput);
        extcmd(cmd, splitInput);
      }
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
    
  }
}