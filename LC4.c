/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

/*
 * Reset the machine state as Pennsim would do
 */

//TODO: LDR and STR
void Reset(MachineState* CPU) {
    CPU->PC = 33280;
    CPU->PSR = 0;
    for (int i = 0; i < 8; i++) {
        CPU->R[i] = 0;
    }
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    CPU->regInputVal = 0;
    CPU->NZPVal = 0;
    CPU->dmemAddr = 0;
    CPU->dmemValue = 0;
    for (int i = 0; i < 65536; i++) {
        CPU->memory[i] = 0;
    }
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU)
{
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->PSR = CPU->PSR & 0xFFF8;
    CPU->DATA_WE = 0;
    CPU->regInputVal = 0;
    CPU->NZPVal = 0;
    CPU->dmemAddr = 0;
    CPU->dmemValue = 0;
}


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output)
{
    if(CPU->regFile_WE ==1){
        fprintf(output, " %01d", CPU->regFile_WE);
        fprintf(output, " %01d %04X", CPU->regInputVal, CPU->R[CPU->regInputVal]);
    } else {
        fprintf(output, " 0 0 0000");
    }
    if (CPU->NZP_WE == 1) {
        fprintf(output, " %01d", CPU->NZP_WE);
        fprintf(output, " %01d", CPU->PSR & 0x0007);
    } else {
        fprintf(output, " 0 0");
    }
    fprintf(output, " %01d", CPU->DATA_WE);
    fprintf(output, " %04X %04X", CPU->dmemAddr, CPU->dmemValue);
    fprintf(output, "\n");
}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{
    int currRow = CPU->PC;
    int currInstr = CPU->memory[currRow];
    if((currRow>=0x2000 && currRow<=0x7FFF)||(currRow>=0xA000 && currRow<=0xFFFF)){
        return 1;
    }
    if ((currInstr & 0xF000) == 0x8000) {
        // RTI
        CPU->PC = CPU->R[7];
        CPU->PSR = CPU->PSR & 0x7FFF;
        ClearSignals(CPU);
    } else if ((currInstr & 0xF000) == 0x9000){
        // CONST
        int rd = (currInstr & 0x0E00) >> 9;
        int IMM9 = currInstr & 0x01FF;
        IMM9 &= 0x01FF;
        if (IMM9 & 0x0100) {
            IMM9 |= ~0x01FF;
        }
        CPU->R[rd] = IMM9;
        ClearSignals(CPU);
        SetNZP(CPU, CPU->R[rd]);
        CPU->rdMux_CTL = 1;
        CPU->regFile_WE = 1;
        CPU->NZP_WE = 1;
        CPU->regInputVal = rd;
        CPU->NZPVal = CPU->PSR & 0x0007;
        CPU->PC = currRow + 1;
    } else if ((currInstr & 0xF000) == 0xD000) {
        // HICONST
        int rd = (currInstr & 0x0E00) >> 9;
        CPU->R[rd] = (CPU->R[rd] & 0x00FF) | (currInstr & 0x00FF) << 8;
        ClearSignals(CPU);
        SetNZP(CPU, CPU->R[rd]);
        CPU->rsMux_CTL = 2;
        CPU->regFile_WE = 1;
        CPU->NZP_WE = 1;
        CPU->regInputVal = rd;
        CPU->NZPVal = CPU->PSR & 0x0007;
        CPU->PC = currRow + 1;
    } else if ((currInstr & 0xF000) == 0xF000) {
        // TRAP
        CPU->R[7] = currRow + 1;
        CPU->PC = 0x8000 | (currInstr & 0x00FF);
        CPU->PSR = CPU->PSR | 0x8000;
        ClearSignals(CPU);
        SetNZP(CPU, CPU->R[7]);
        CPU->rdMux_CTL = 1;
        CPU->regFile_WE = 1;
        CPU->NZP_WE = 1;
        CPU->regInputVal = 7;
        CPU->NZPVal = CPU->PSR & 0x0007;
    } else if ((currInstr & 0xF000) == 0x6000) {
        //LDR
        int rd = (currInstr & 0x0E00) >> 9;
        int rs = (currInstr & 0x01C0) >> 6;
        int offset = currInstr & 0x003F;
        offset &= 0x3F;
        if (offset & 0x20) {
            offset |= ~0x3F;
        }
        int location = CPU->R[rs] + offset;
        CPU->R[rd] = CPU->memory[CPU->R[rs] + offset];
        ClearSignals(CPU);
        SetNZP(CPU, CPU->R[rd]);
        CPU->rdMux_CTL = 1;
        CPU->regFile_WE = 1;
        CPU->NZP_WE = 1;
        CPU->regInputVal=rd;
        CPU->NZPVal = CPU->PSR & 0x0007;
        CPU->dmemAddr=location;
        if(location<0x2000 || (location>=0x8000 && location<0xA000)){
            return 1;
        }
        if(((CPU-> PC & 0x8000)==0x0000) && ((CPU->PSR & 0x8000)==0x0000) && (location>=0x8000 && location<=0xFFFF)){
            return 1;
        }
        CPU->dmemValue=CPU->memory[CPU->dmemAddr];
        CPU->PC = currRow + 1;
    } else if ((currInstr & 0xF000) == 0x7000) {
        //STR
        int rt = (currInstr & 0x0E00) >> 9;
        int rs = (currInstr & 0x01C0) >> 6;
        int offset = currInstr & 0x003F;
        offset &= 0x3F;
        if (offset & 0x20) {
            offset |= ~0x3F;
        }
        int location = CPU->R[rs] + offset;
        CPU->memory[CPU->R[rs] + offset] = CPU->R[rt];
        ClearSignals(CPU);
        CPU->rtMux_CTL = 1;
        CPU->DATA_WE = 1;
        CPU->dmemAddr=location;
        CPU->dmemValue=CPU->memory[CPU->dmemAddr];
        if(location<0x2000 || (location>=0x8000 && location<0xA000)){
            return 1;
        }
        if(((CPU-> PC & 0x8000)==0x0000) && ((CPU->PSR & 0x8000)==0x0000) && (location>=0x8000 && location<=0xFFFF)){
            return 1;
        }
        CPU->PC = currRow + 1;
    } else if((currInstr & 0xF000) == 0){
        BranchOp(CPU, output);
    } else if ((currInstr & 0xF000) == 0x1000) {
        ArithmeticOp(CPU, output);
    } else if ((currInstr & 0xF000) == 0x2000) {
        ComparativeOp(CPU, output);
    } else if ((currInstr & 0xF000) == 0x5000) {
        LogicalOp(CPU, output);
    } else if ((currInstr & 0xF000) == 0xC000) {
        JumpOp(CPU, output);
    } else if ((currInstr & 0xF000) == 0x4000) {
        JSROp(CPU, output);
    } else if ((currInstr & 0xF000) == 0xA000) {
        ShiftModOp(CPU, output);
    } else {
        CPU->PC = currRow + 1;
    }
    fprintf(output, "%04X ", currRow);
    for(int i=15; i>=0; i--){
        int bit = (currInstr >> i) & 1;
        fprintf(output, "%01d", bit);
    }
    WriteOut(CPU, output);
    return 0;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{
    int operation = CPU->memory[CPU->PC] & 0x0E00;
    int IMM9 = CPU->memory[CPU->PC] & 0x01FF;
    IMM9 &= 0x01FF;
    if (IMM9 & 0x0100) {
        IMM9 |= ~0x01FF;
    }
    int nzp = (CPU->PSR & 0x0007);
    if (operation == 0) {
        CPU->PC = CPU->PC + 1;
    } else if (operation == 0x0800 && nzp == 4) {
        CPU->PC = CPU->PC + IMM9 + 1;
    } else if (operation == 0x0C00 && (nzp == 4 || nzp == 2)) {
        CPU->PC = CPU->PC + IMM9 + 1;
    } else if (operation == 0x0A00 && (nzp== 4 || nzp == 1)) {
        CPU->PC = CPU->PC + IMM9 + 1;
    } else if (operation == 0x0400 && nzp == 2) {
        CPU->PC = CPU->PC + IMM9 + 1;
    } else if (operation == 0x0600 && (nzp == 2 || nzp == 1)) {
        CPU->PC = CPU->PC + IMM9 + 1;
    } else if (operation == 0x0200 && nzp==1) {
        CPU->PC = CPU->PC + IMM9 + 1;
    } else if (operation == 0x0E00) {
        CPU->PC = CPU->PC + IMM9 + 1;
    } else {
        CPU->PC = CPU->PC + 1;
    }
    ClearSignals(CPU);
    CPU->PSR = CPU->PSR | nzp;
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    int operation = CPU->memory[CPU->PC] & 0x0038;
    int rd = (CPU->memory[CPU->PC] & 0x0E00) >> 9;
    int rs = (CPU->memory[CPU->PC] & 0x01C0) >> 6;
    int rt = CPU->memory[CPU->PC] & 0x0007;
    int s = CPU->R[rs];
    int t = CPU->R[rt];
    if (operation == 0x0000) {
        CPU->R[rd] = s+t;
    } else if (operation == 0x0008) {
        CPU->R[rd] = s*t;
    } else if (operation == 0x0010) {
        CPU->R[rd] = s-t;
    } else if (operation == 0x0018) {
        CPU->R[rd] = s/t;
    } else {
        // convert from 2s complement
        int j=(CPU->memory[CPU->PC] & 0x001F);
        j &= 0x1F;
        if (j & 0x10) {
            j |= ~0x1F;
        }
        CPU->R[rd] = s+j;
    }
    ClearSignals(CPU);
    SetNZP(CPU, CPU->R[rd]);
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->regInputVal = rd;
    CPU->NZPVal = CPU->PSR & 0x0007;
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
    unsigned short int num1;
    unsigned short int num2;
    unsigned short int sign1;
    unsigned short int sign2;
    if ((CPU->memory[CPU->PC] & 0x0180) == 0 || (CPU->memory[CPU->PC] & 0x0180) == 0x0080){
        //Registers
        num1 = CPU->R[(CPU->memory[CPU->PC] & 0x0E00) >> 9];
        num2 = CPU->R[CPU->memory[CPU->PC] & 0x0007];
        sign1 = (num1 & 0x8000) >> 15;
        sign2 = (num2 & 0x8000)>> 15;
    } else {
        //Intermediate
        num1 = CPU->R[(CPU->memory[CPU->PC] & 0x0E00) >> 9];
        num2 = CPU->memory[CPU->PC] & 0x007F;
        sign1 = (num1 & 0x8000)>>15;
        sign2 = (num2 & 0x0040)>>6;
    }
    ClearSignals(CPU);
    if((CPU->memory[CPU->PC] & 0x0180) == 0 || (CPU->memory[CPU->PC] & 0x0180) == 0x0100){
        //Signed
        if((sign1 == 0 && sign2 == 0) || (sign1 == 1 && sign2 == 1)){
            SetNZP(CPU, num1-num2);
        } else if(sign1 == 0 && sign2 == 1) {
            CPU->PSR = CPU->PSR | 0x0001;
        } else {
            CPU->PSR = CPU->PSR | 0x0004;
        }
    } else {
        //Unsigned
        SetNZP(CPU, num1-num2);
    }
    CPU->rsMux_CTL = 2;
    CPU->NZP_WE = 1;
    CPU->regInputVal = 0;
    CPU->NZPVal = CPU->PSR & 0x0007;
    CPU->dmemAddr=0;
    CPU->dmemValue=0;
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
    int operation = CPU->memory[CPU->PC] & 0x0038;
    int rd = (CPU->memory[CPU->PC] & 0x0E00) >> 9;
    int rs = (CPU->memory[CPU->PC] & 0x01C0) >> 6;
    int rt = CPU->memory[CPU->PC] & 0x0007;
    int s = CPU->R[rs];
    int t = CPU->R[rt];
    if (operation == 0x0000) {
        CPU->R[rd] = s&t;
    } else if (operation == 0x0008) {
        CPU->R[rd] = ~s;
    } else if (operation == 0x0010) {
        CPU->R[rd] = s|t;
    } else if (operation == 0x0018) {
        CPU->R[rd] = s ^ t;
    } else {
        CPU->R[rd] = s&(CPU->memory[CPU->PC] & 0x001F);
    }
    ClearSignals(CPU);
    SetNZP(CPU, CPU->R[rd]);

    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->regInputVal = rd;
    CPU->NZPVal = CPU->PSR & 0x0007;
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
    if ((CPU->memory[CPU->PC] & 0x0800) == 0){
        int rs = (CPU->memory[CPU->PC] & 0x01C0) >> 6;
        CPU->PC = CPU->R[rs];
    } else {
        int imm11 = CPU->memory[CPU->PC] & 0x07FF;
        imm11 &= 0x07FF;
        if (imm11 & 0x0400) {
            imm11 |= ~0x07FF;
        }
        CPU->PC = CPU->PC + imm11 + 1;
    }
    ClearSignals(CPU);
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
    if ((CPU->memory[CPU->PC] & 0x0800) == 0x0800){
        CPU->R[7] = CPU->PC + 1;
        int IMM11 = CPU->memory[CPU->PC] & 0x07FF;
        IMM11 &= 0x07FF;
        if (IMM11 & 0x0400) {
            IMM11 |= ~0x07FF;
        }
        CPU->PC = (CPU->PC & 0x8000) | (IMM11<<4);
    } else {
        int rs = (CPU->memory[CPU->PC] & 0x01C0) >> 6;
        int r7 = CPU->PC +1;
        CPU->PC = CPU->R[rs];
        CPU->R[7] = r7;
    }
    ClearSignals(CPU);
    SetNZP(CPU, CPU->PC);
    CPU->rdMux_CTL=1;
    CPU->regFile_WE=1;
    CPU->regInputVal=7;
    CPU->NZP_WE=1;
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
    int rd = (CPU->memory[CPU->PC] & 0x0E00) >> 9;
    int rs = (CPU->memory[CPU->PC] & 0x01C0) >> 6;
    int offset = CPU->memory[CPU->PC] & 0x000F;
    if ((CPU->memory[CPU->PC] & 0x0030)==0) {
        CPU->R[rd] = CPU->R[rs] << offset;
    } else if ((CPU->memory[CPU->PC] & 0x0030)==0x0010) {
        int sign = (CPU->R[rs] & 0x8000) >> 15;
        for (int i = 0; i < offset; ++i) {
            if (sign == 0){
                CPU->R[rd] = CPU->R[rd] >> 1;
            } else {
                CPU->R[rd] = (CPU->R[rd] >> 1) | 0x8000;
            }
        }
    } else if ((CPU->memory[CPU->PC] & 0x0030)==0x0020) {
        CPU->R[rd] = CPU->R[rs] >> offset;
    } else if ((CPU->memory[CPU->PC] & 0x0030)==0x0030) {
        int rt = CPU->memory[CPU->PC] & 0x0007;
        CPU->R[rd] = CPU->R[rs] % CPU->R[rt];
    }
    ClearSignals(CPU);
    SetNZP(CPU, CPU->R[rd]);
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->regInputVal = rd;
    CPU->PC = CPU->PC + 1;
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{
    CPU->PSR = CPU->PSR & 0xFFF8;
    if(result == 0){
        CPU->PSR = CPU->PSR | 0x0002;
    } else if(result < 0){
        CPU->PSR = CPU->PSR | 0x0004;
    } else {
        CPU->PSR = CPU->PSR | 0x0001;
    }
}
