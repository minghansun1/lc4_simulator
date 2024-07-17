/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv)
{
    if (argc<=2){
        fprintf(stderr, "error3: usage: ./trace <object_file.obj> <memory_limit>");
        return -1;
    }
    char* filename = argv[1];
    char **objFiles= malloc((argc-2) * sizeof(char*));
    CPU = malloc(sizeof(MachineState));
    Reset(CPU);
    for (int i=2; i<argc; i++){
        objFiles[i-2] = argv[i];
        ReadObjectFile(objFiles[i-2], CPU);
    }
    FILE *file = fopen(filename, "w");
    while (CPU->PC != 0x80FF) {
        if (UpdateMachineState(CPU, file) == 1) {
            printf("Error");
            break;
        }
    }
    fclose(file);
    free(objFiles);
    free(CPU);
    return 0;
}
