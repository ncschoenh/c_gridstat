#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include "ggg_reader.h"


int main(int numarg, char** args) {
    printf("GRID STAT written in C\n");
    if (numarg != 3) {
        fprintf(stderr, "\nYou did not supply enough arguements. 2 args required... Input GGG, and output folder!\n");
        exit(0);
    }
   char *gggFilePath = args[1];
   char *outputFolderPath = args[2];
   int i;
   if (strlen(gggFilePath) < 6) {
        fprintf(stderr, "\nGGG File provided %s not a valid ggg file\n", gggFilePath);
        exit(0);
   }
   for (i = strlen(gggFilePath)-3; i < strlen(gggFilePath); i++) {
        if (gggFilePath[i] != 'g') {
            fprintf(stderr, "\nGGG File provided %s not a valid ggg file\n", gggFilePath);
            exit(0);
        }
   }
   int resolution;
   if (strstr(gggFilePath, "2m") != NULL) {
        resolution = TWO_MINUTE_RESOLUTION;    
   } else {
        fprintf(stderr, "\nResolution not currently supported...\n");
        exit(0);
   }
   
   float deg2km = 1.8520 * 60.0;
   float radius = 100.0f / deg2km;
   printf("%s, %d\n", gggFilePath, resolution);
   float *data = read_GGG_File(gggFilePath, resolution);
   int *gridResos = getGridResolution(resolution);
   float *outputGrid = malloc(sizeof(float) * 5400 * 10800);
   int j;
   #pragma omp parallel for private(j)
   for (i = 0; i < gridResos[1]; i++) {
        for (j = 0; j < gridResos[0]; j++) {
           float* coord = getLatLonCoordinate(i, j, resolution);
           float maxx = coord[0] + radius;
           float maxy = coord[1] + radius;
           float minx = coord[0] - radius;
           float miny = coord[1] - radius;
           // Do BBOX wrapping here
           if (maxx >= 180.0f) {
                maxx = 179.99f;
           } 
           if (maxy >= 90.0f) {
                maxy = 89.99f;
           }
           if (minx <= -180.0f) {
                minx = -179.99f;
           }
           if (miny <= -90.0) {
                miny = -89.99;
           }
           //printf("Coord: %d", coord[0]);
           //printf("%f\n", minx);
           int* max = getGridCoordinate(maxy, maxx, resolution);
           int* min = getGridCoordinate(miny, minx, resolution);
           int k;
           int l;
           int count = 0;
           float total = 0;
           for (k = min[1]; k < max[1]; k++) {
                for (l = min[0]; l < max[0]; l++) {
                    total += readFloat(data, k, l, resolution);
                    count++;
                }
           }
           //printf("Deg2km: %f\n", deg2km);
           //printf("Radius: %f\n", radius);
           //printf("Maxx: %f, Minx: %f, Maxy: %f, Miny: %f\n", maxx, minx, maxy, miny);
           //printf("Mean: %f, Row: %d, Col:%d\n", total/count, i, j);
           outputGrid[i*10800 + j] = total/count;
           free(max);
           free(min);
           free(coord);
        }
   }
   free(data);
   free(gridResos);
}
