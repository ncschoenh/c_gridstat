#include "mpi.h"
#include<stdio.h>
#include<stdlib.h>
#include "ggg_reader.h"

int main(int argc, char *args[])
{
	int rank, numproc;
	MPI_Init(&argc, &args);
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	int numRows, numCols, resolution;
	float radius;
	float *data, *outputGrid;
	if(rank == 0) {
		if (argc != 3) {
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
		if (strstr(gggFilePath, "2m") != NULL) {
			resolution = TWO_MINUTE_RESOLUTION;    
		} else {
			fprintf(stderr, "\nResolution not currently supported...\n");
			exit(0);
		}
		float deg2km = 1.8520 * 60.0;
		radius = 100.0f / deg2km;
		printf("%s, %d\n", gggFilePath, resolution);
		data = read_GGG_File(gggFilePath, resolution);
		int *gridResos = getGridResolution(resolution);
		numRows = gridResos[1];
		numCols = gridResos[0];
		int dataSize = numRows*numCols;
		outputGrid = malloc(sizeof(float) * dataSize);
		MPI_Bcast(&numRows, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&numCols, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&resolution, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&radius, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Bcast(data, dataSize, MPI_FLOAT, 0, MPI_COMM_WORLD);
		
		int start = MPI_Wtime();
	}
	// compute my responsibility
	int myIndex;
	int myRows = numRows/numproc;
	int mod = numRows%numproc;
	if(rank < mod) {
		myRows++;
		myIndex = rank*myRows;
	}
	else {
		myIndex = (rank*myRows)+mod;
	}
	int x, y;
	for (y = myIndex; y < myIndex+myRows; y++) {
        for (x = 0; x < numCols; x++) {
           float* coord = getLatLonCoordinate(y, x, resolution);
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
           printf("Mean: %f, Row: %d, Col:%d\n", total/count, y, x);
           free(max);
           free(min);
           free(coord);
        }
	 }
		/**
		MPI_Recv(&myArraySize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		myArray = malloc(myArraySize*sizeof(int));
		MPI_Recv(myArray, myArraySize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		*/
	
	/**
	int tempSize, parent, rightChild;
	while(myLevel < level) {
		parent = (rank & (~(1 << myLevel)));
		if(rank == parent) {
			rightChild = (rank | (1 << myLevel));
			MPI_Recv(&tempSize, 1, MPI_INT, rightChild, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			int *childArray = malloc(tempSize*sizeof(int));
			MPI_Recv(childArray, tempSize, MPI_INT, rightChild, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			merged = malloc((myArraySize+tempSize)*sizeof(int));
			merge(merged, myArray, childArray, myArraySize, tempSize);
			free(childArray);
			free(myArray);
			myArray = merged;
			myArraySize = myArraySize+tempSize;
			myLevel ++;
		}
		else {
			MPI_Send(&myArraySize, 1, MPI_INT, parent, 0, MPI_COMM_WORLD);
			MPI_Send(myArray, myArraySize, MPI_INT, parent, 0, MPI_COMM_WORLD);
			myLevel = level;
		}
	}
	// print sorted array
	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0) {
		end = MPI_Wtime();
		printf("\n");
		printf("sorted array:\n");
		for(i=0; i<myArraySize; i++) {
			printf("%d ", myArray[i]);
		}
		printf("\n");
		free(origArray);
		double time = end-start;
		printf("total process time %.5fs for %d processes\n", time, numproc);
	}
	free(myArray);
	
	*/
	MPI_Finalize();
    return 0;
}
