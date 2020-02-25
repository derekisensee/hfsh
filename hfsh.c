#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void *extcmd(void *vargp) {
  //printf(vargp[0]);
}

int main(int argc, char *args[]) {
  // normal mode
  if (argc == 1) {
    int running = 1;

    while (running) {
      // vars
      char *buffer;
      size_t buffSize = 32;
      size_t input;
      buffer = (char *)malloc(buffSize * sizeof(char));

      // check for any buffer errors?
      // --------------------

      printf("hfsh> ");
      input = getline(&buffer, &buffSize, stdin); // input is stored in 'buffer'
      char * splitInput = strtok(buffer, " "); // strtok modifies 'buffer' ?
      printf("cmd: %s\n", splitInput);
      //printf("\n input: %s", buffer);
    }
  }
  // batch mode
  if (argc == 2) {
    
  }
}
