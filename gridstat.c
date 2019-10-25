#include "ggg_reader.h"
#include <string.h>


int main(int numarg, char** args) {
    printf("GRID STAT written in C\n");
    if (numarg != 3) {
        fprintf(stderr, "\nYou did not supply enough arguements. 2 args required... Input GGG, and output folder!\n");
    }
   char *gggFilePath = args[1];
   char *outputFolderPath = args[2];
   int i;
   if (strlen(gggFilePath) <= 4) {
        fprintf(stderr, "\nGGG File provided %s not a valid ggg file\n", gggFilePath);
   }
   for (i = strlen(gggFilePath)-3; i < strlen(gggFilePath); i++) {
        if (gggFilePath[i] != 'g') {
            fprintf(stderr, "\nGGG File provided %s not a valid ggg file\n", gggFilePath);
        }
   }
}
