#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

// should single external commands run in their own thread? 
 // don't wait if given & - for last cmd without a & after it, do execv and make hfsh wait. so last cmd is in "foreground"
// exit' ' (exit with space after it) is an error - intentional?
// have to make paths absolute? include entire path to folders specified
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
    ext = strtok(NULL, " "); // ??? need to delimit by newline/carriage return
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
  char *splitPaths = strtok(tempEnv, ":\n"); // strtok modifies 'buffer' ?
  char slash[1024] = {'/', '\0'};
  strcat(slash, cmd); // slash now holds "/cmd"

  int found = 0;
  while (splitPaths != NULL) {
    // use access() to find program
    char spCopy[1024] = ""; // seems to reset spCopy between path checks
    strcat(spCopy, splitPaths);
    strcat(spCopy, slash);
    
    if (access(spCopy, F_OK) == 0) {
      found = 1;
      int status;
      //pid_t w;
      if (fork() == 0) {
        if (file != NULL) { // if we have a file, write to that instead
          int fp = open(file, O_RDWR | O_CREAT | O_TRUNC, 0666); // change permissions here?
          dup2(fp, 1);
          close(fp);
        }
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
  if (found == 0) {
    //printf("%s", path);
    errorPrint();
  }
  return 0;
}

void cmdChunk(char *splitInput, char *file, int waiting) {
  //char *splitInput = strtok();
  if (strcmp(splitInput, "exit") == 0) {
    splitInput = strtok(NULL, " ");
    if (splitInput != NULL) {
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
    else {
        //errorPrint();
        //printf("null!");
        //argCount = 99; // make error happen
        // TODO: need to make error trip later
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
    /*if (splitInput != NULL) {
    }
    else { // if splitinput is blank, happens in 17.in example?
      extcmd(cmd, cmd, waiting, file);
    }*/
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
      //printf("not null");
      file = malloc(strlen(redir)+1);
      strcpy(file, redir);
    }
    else {
      //errorPrint();
      err = 1;
    }
  }

  if (err == 0) {
    if ((redir = strtok(NULL, " ")) == NULL) { // make sure we only get 1 file
      char *splitInput = strtok(firstChunk, " \t\n");
      if (splitInput == NULL) {
        //printf("null");
      }
      cmdChunk(splitInput, file, waiting);
    }
    else {
      //printf("%s", redir);
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
  int len = strlen(buffer);
  buffer[len-1] = '\0';
  // check whitespace case
  char *whitespaceCheck = malloc(len+1);
  strcpy(whitespaceCheck, buffer);
  char *whitespaceSplit = strtok(whitespaceCheck, " & > \t\n");
  if (whitespaceSplit == NULL) { // if blank input is given or if only '&' or '>' is given
    // do nothing
  }
  else {
    // -----need to check if redirection or &'s------ 
    // check for &, for every & split up, do the next chunk of tasks 
    char *inputCopyCurr = malloc(len + 1); // the current command
    strcpy(inputCopyCurr, buffer);
    //char *inputCopyNext = malloc(len + 1); // what the next command would be
    //strcpy(inputCopyNext, buffer);

    // ------ experiment begin
    if (strstr(buffer, "&")) {
      int count = 0;
      int i;
      for (i = 0; i < len; i++) {
        if (buffer[i] == '&') {
          ++count;
        }
      }
      char *currCmd = strtok(inputCopyCurr, " &");
      while (count-- > 0 && currCmd != NULL) {
        chunkHandle(currCmd, 0);
        currCmd = strtok(NULL, " ");
      }
      if (currCmd != NULL) {
        chunkHandle(currCmd, 1);
      }
    }
    else { // single, 'regular' commands
      chunkHandle(buffer, 1);
    }
    
    ///
    /*
    char *currCmd = strtok(inputCopyCurr, "&");
    char *amp = strtok(inputCopyNext, "&"); // tokenized by '&'
    amp = strtok(NULL, " "); // amp is our next command

    // commands ran from this loop do NOT wait for parent to return
    while (amp != NULL) {
      chunkHandle(currCmd, 0);
      currCmd = strtok(NULL, " ");
      if (currCmd == NULL) {
        break;
      }
      amp = strtok(NULL, " ");
    }
    // then this last command will wait
    if (amp == NULL) {
      chunkHandle(currCmd, 1);
    }
    else {
      chunkHandle(amp, 1);
    }*/
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
  //path = "/bin:"; // our initial path
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