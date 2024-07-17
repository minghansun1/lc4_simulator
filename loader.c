/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location


void parse_code_or_data(unsigned char buffer[2], unsigned short label, unsigned short memoryAddress, unsigned short n, FILE *file,
                MachineState *CPU);

void parse_symbol(unsigned char buffer[2], unsigned short label, unsigned short memoryAddress, unsigned short n, FILE *file,
                  MachineState *CPU);

void
parse_file_name(unsigned char buffer[2], unsigned short label, unsigned short memoryAddress, unsigned short n, FILE *file,
                MachineState *CPU);

void
parse_line_number(unsigned char buffer[2], unsigned short label, unsigned short memoryAddress, unsigned short n, FILE *file,
                  MachineState *CPU);

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "error2: read_asm_file() failed");
        return 2;
    }
    unsigned char buffer[2];
    unsigned short int label;
    unsigned short int memoryAddress;
    unsigned short int n;
    while (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
        label = (buffer[0] << 8) | buffer[1];
        memoryAddress=0;
        n=0;
        if (label == 51934 || label == 56026) {
            //printf("Label: 0x%04x\n", label);
            parse_code_or_data(buffer, label, memoryAddress, n, file, CPU);
        } else if (label == 50103) {
            //printf("Label: 0x%04x\n", label);
            parse_symbol(buffer, label, memoryAddress, n, file, CPU);
        } else if (label == 61822) {
            //printf("Label: 0x%04x\n", label);
            parse_file_name(buffer, label, memoryAddress, n, file, CPU);
        } else if (label == 29022) {
            //printf("Label: 0x%04x\n", label);
            parse_line_number(buffer, label, memoryAddress, n, file, CPU);
        } else {
            //printf("Invalid label: 0x%04x\n", label);
        }
    }
    fclose(file);
    return 0;
}

void parse_code_or_data(unsigned char buffer[2], unsigned short label, unsigned short memoryAddress, unsigned short n, FILE *file,
                MachineState *CPU) {
    //printf("Label: 0x%04x\n", label);
    if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
        memoryAddress = (buffer[0] << 8) | buffer[1];
        //printf("Memory Address: 0x%04x\n", memoryAddress);
        if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
            n = (buffer[0] << 8) | buffer[1];
            //printf("n: 0x%04x\n", n);
            for (int i=0; i<n; i++) {
                if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
                    //printf("Memory[%d]:", memoryAddress+i);
                    CPU->memory[memoryAddress+i] = (buffer[0] << 8) | buffer[1];
                    //printf("0x%04x\n", CPU->memory[memoryAddress+i]);
                }
            }
        }
    }
}

void parse_symbol(unsigned char buffer[2], unsigned short label, unsigned short memoryAddress, unsigned short n, FILE *file,
                  MachineState *CPU) {
    if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
        memoryAddress = (buffer[0] << 8) | buffer[1];
        //printf("Memory Address: 0x%04x\n", memoryAddress);
        if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
            n = (buffer[0] << 8) | buffer[1];
            //printf("n: 0x%04x\n", n);
            for (int i=0; i<n; i++) {
                if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
                    //printf("Symbols: %c%c\n", buffer[0], buffer[1]);
                }
            }
        }
    }
}

void
parse_file_name(unsigned char buffer[2], unsigned short label, unsigned short memoryAddress, unsigned short n, FILE *file,
                MachineState *CPU) {
    if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
        n = (buffer[0] << 8) | buffer[1];
        printf("n: 0x%04x\n", n);
        for (int i=0; i<n; i++) {
            printf("File Name: ");
            if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
                printf("%c%c", buffer[0], buffer[1]);
            }
            printf("\n");
        }
    }
}

void
parse_line_number(unsigned char buffer[2], unsigned short label, unsigned short memoryAddress, unsigned short n, FILE *file,
                  MachineState *CPU) {
    if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
        memoryAddress = (buffer[0] << 8) | buffer[1];
        printf("Memory Address: 0x%04x\n", memoryAddress);
        if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
            unsigned short int line = (buffer[0] << 8) | buffer[1];
            printf("line: 0x%04x\n", line);
            if (fread(buffer, sizeof(unsigned char), 2, file) == 2) {
                unsigned short int file_index = (buffer[0] << 8) | buffer[1];
                printf("file-index: 0x%04x\n", file_index);
            }
        }
    }
}
