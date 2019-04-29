/**
 * Author: Sanisha Rehan
 * 
 * This file includes utility functions that could be re-used across all the 
 * system software projects.
 */
#include "cpu_constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Macro to print new lines X numnber of times.
# define NEWLINE(X) { int i=0; for (; i < X; ++i) printf("\n"); }

// Macro to print char X num of times.
# define PRINT_CHAR(ch, X) { int i=0; for(; i < X; ++i) printf("%c", ch);}

/*
 * Check whether a given string is present in array of strings. This function
 * is case-sensitive.
 * 
 * Input arguments: 
 *  str: pointer to string that has to be searched. str should be
 *       null-terminated.
 *  arr: pointer 2-D array of characters. Each 1-D array should be null 
 *      terminated, otherwise output cannot be predicted.
 *  L: Length of the 2-arr where each 1-D array would have different width.
 *
 * Output:
 *  Returns true is str is exact match of one the strings in array of strings.
 *  Otherwise, returns false.
 */
bool IsStringInStringArray(const char* str, const char* arr[], int L) {
    int i;
    for (i = 0; i < L; ++i) {
        int j = 0;
        while(arr[i][j] != '\0' && str[j] != '\0'
                && arr[i][j] == str[j] && ++j) {}
        if (arr[i][j] == str[j]) {
            return true;
        }
    }
    return false;
}

/*
 * Returns true if input string starts with the given prefix, else false.
 * 
 * Both strings should be null-terminated, otherwise output cannot be 
 * predicted.
 */
bool isStartsWith(char* str, char* prefix) {
    int i=0;
    while(str[i] != '\0' && prefix[i] != '\0' && prefix[i] == str[i] && ++i) {}
    if (prefix[i] == '\0') {
        return true;
    }
    return false;
}

/*
 * Parses string and convert into long.
 * Example:
 *   "45" => 45
 *   "0x45" => 69
 *
 * Input string can only be in base 10 or Hex. Only positive numbers are 
 * supported.
 *
 * Returns -1 if input string is not valid.
 */
long getLongFromBaseTenOrHexString(char* str) {
    char* next;
    long result = strtol(str, &next, 0);
    if (next == str || *next != '\0') {
        return -1;
    }
    return result;
}


// Returns index of the first space in str.
// If no ch is found, it returns -1.
int getIndexOfFirstChar(char* str, char ch) {
    int i=0;
    while(str[i] != '\0' && str[i] != ch) {
        ++i;
    }
    if (str[i] == ch) { return i; }
    return -1;
}


// Remove white spaces from string.
char* removeWhiteSpacesFromString(char* str) {
  char *out = str, *put = str;

  for(; *str != '\0'; ++str)
  {
    if(*str != ' ' && *str != '\n')
        *put++ = *str;
  } 
  *put = '\0';
  return out;
}

// Removes white spaces from 2-D array of strings.
void removeWhiteSpaces(char** args, int arg_count) {
    int i;
    for (i=0; i < arg_count; ++i) {
        args[i] = removeWhiteSpacesFromString(args[i]);
    }
}

/*
 * Get opcode corresponding to the instruction.
 */
int
getOpcodeFromInstruction(char *command) {
  int i = 0;
  for (i = 0; i < TOTAL_ASSEMBLY_OPCODES; i++) {
      if (strcmp(command, opcode_map[i].instruction) == 0) {
          return opcode_map[i].opcode;
      }
  }
  return -1;
}


char *
getInstructionFromOpcode(int opcode) {
    int i = 0;
    for (i = 0; i < TOTAL_ASSEMBLY_OPCODES; i++) {
        if (opcode == opcode_map[i].opcode) {
            return opcode_map[i].instruction;
        }
    }
}

/*
 * Function to decode te binary instruction code and return in instruction attribute.
*/
void
decodeInstructionFromBinary(SIZE_TYPE binary_opcode, struct instruction_attr* instr_attr_ptr) {
    int opcode = (binary_opcode >> 26) & 0x3f;
    char *command;
    command = getInstructionFromOpcode(opcode);
    
    // Get instruction attributes
    int op_reg = (binary_opcode >> 22) & 0x0f;
    int base_reg = (binary_opcode >> 18) & 0x0f;
    int index_reg = (binary_opcode >> 14) & 0x0f;
    int scale = 0x01 << ((binary_opcode >> 12) & 0x03);
    char offset = (binary_opcode >> 4) & 0xff;

    // Set instruction attributes 
    strcpy(instr_attr_ptr->instruction, command);
    instr_attr_ptr->operand_register = op_reg;
    instr_attr_ptr->base_register = base_reg;
    instr_attr_ptr->index_register = index_reg;
    instr_attr_ptr->scale = scale;
    instr_attr_ptr->offset = (int) offset;

    // Memory-Type instructions
    if (IsStringInStringArray(command, MEM_INSTR, NUM_VALID_MEM_INSTR)) {
        instr_attr_ptr->format = LOAD_STORE;
    } 
    // R-Type instructions
    else if (IsStringInStringArray(command, R_INSTR, NUM_VALID_R_INSTR)) {
        int instr_format = (binary_opcode >> 2) & 0x03;
        instr_attr_ptr->format = instr_format;    
    }
    // Immediate-Type instructions
    else if (IsStringInStringArray(command, I_INSTR, NUM_VALID_I_INSTR)) {
        char label = binary_opcode & 0xff;
        instr_attr_ptr->const_or_label = (int) label;
        // Imm-register type
        if (base_reg == 0) {
            instr_attr_ptr->format = IMM_REG;
        } 
        // Imm-memory type
        else {
            char offset = (binary_opcode >> 8) & 0xff;
            instr_attr_ptr->base_register = op_reg;
            instr_attr_ptr->index_register = base_reg;
            instr_attr_ptr->scale = 1 << ((binary_opcode >> 16) & 0x03);
            instr_attr_ptr->offset = (int) offset;
            instr_attr_ptr->format = IMM_MEM;
        }
    }
    // Jump/Call instructions
    else if (IsStringInStringArray(command, CONTROL_INSTR, NUM_VALID_CONTROL_INSTR)) {
        instr_attr_ptr->format = CONTROL_LABEL;
        short label = binary_opcode & 0xffff;
        instr_attr_ptr->const_or_label = (int) label;
    }
    // Stack instructions
    else if (IsStringInStringArray(command, STACK_INSTR, NUM_VALID_STACK_INSTR)) {
        instr_attr_ptr->format = STACK_REG;
    }
    // No operand instruction
    else if (IsStringInStringArray(command, NO_OPERAND_INSTR, NUM_VALID_NO_OPERAND_INSTR)) {
        instr_attr_ptr->format = NO_OPERAND;
    }
    // Memory Display Instruction
    else if (IsStringInStringArray(command, MEM_DISPLAY_INSTR, NUM_VALID_MEM_DISPLAY_INSTR)) {
        instr_attr_ptr->format = MEM_DISPLAY;
        char label = binary_opcode & 0xff;
        instr_attr_ptr->const_or_label = (int) label;
    }
    // Mov instructions
    else if (IsStringInStringArray(command, MOV_INSTR, NUM_VALID_MOV_INSTR)) {
        int indicator = (binary_opcode >> 16) & 0x01;
        if (indicator) {
            instr_attr_ptr->format = MOV_IMM_REG;
            short label = binary_opcode & 0xffff;
            instr_attr_ptr->const_or_label = (int) label;
        } else {
            instr_attr_ptr->format = MOV_REG_REG;
        }
    }
}


// Function to encode a given assemply instruction into its corresponding byte code.
SIZE_TYPE
encodeInstructionToBinary(struct instruction_attr *instr_attr_ptr) {
    opcode_formats format = instr_attr_ptr->format;
    
    char *instruction = instr_attr_ptr->instruction;
    int opcode = getOpcodeFromInstruction(instruction);
    int base_reg = instr_attr_ptr->base_register;
    int index_reg = instr_attr_ptr->index_register;
    int offset = (instr_attr_ptr->offset & 0xff);
    int scale = instr_attr_ptr->scale / 2;
    int op_reg = instr_attr_ptr->operand_register;
    int label = instr_attr_ptr->const_or_label;
    int indicator = 0;

    SIZE_TYPE binary_opcode;
    
    opcode = opcode << 26;
    op_reg = op_reg << 22;
    base_reg = base_reg << 18;
    index_reg = index_reg << 14;
    scale = scale << 12;
    offset = offset << 4;

    // Convert the instructions to binary based on specific instruction type
    switch (format) {

        default:
            printf ("ERROR:Invalid instruction format. Format '%d' not supported.\n", format);
            exit(0);

        case LOAD_STORE:
            binary_opcode = opcode | op_reg | base_reg | index_reg | scale | offset;
            break;

        case REG_REG:
            indicator = REG_REG_IND << 2;
            binary_opcode = opcode | op_reg | base_reg | indicator;
            break;

        case REG_MEM:
            indicator = REG_MEM_IND << 2;
            binary_opcode = opcode | op_reg | base_reg | index_reg | scale | offset | indicator;
            break;

        case MEM_REG:
            indicator = MEM_REG_IND << 2;
            binary_opcode = opcode | op_reg | base_reg | index_reg | scale | offset | indicator;
            break;

        case MEM_DISPLAY:
        case IMM_REG:
            label = (label & 0xff);
            binary_opcode = opcode | op_reg | label;
            break;

        case IMM_MEM:
            base_reg = base_reg << 22;
            index_reg = index_reg << 18;
            scale = scale << 16;
            offset = offset << 8;
            label = (label & 0xff);
            binary_opcode = opcode | base_reg | index_reg | scale | offset | label;
            break;

        case CONTROL_LABEL:
            label = (label & 0xffff);
            binary_opcode = opcode | label;
            break;

        case STACK_REG:
            binary_opcode = opcode | op_reg;
            break;

        case MOV_REG_REG:
            indicator = MOV_REG_REG_IND << 16;
            binary_opcode = opcode | op_reg | base_reg | indicator;
            break;

        case MOV_IMM_REG:
            indicator = MOV_IMM_REG_IND << 16;
            label = label & 0xffff;
            binary_opcode = opcode | op_reg | indicator | label;
            break;

        case NO_OPERAND:
            binary_opcode = opcode;
            break;
    }
    return binary_opcode;
}

// Returns index of the label, otherwise returns -1.
int getLabelIndex(char* label) {
    int i;
    for (i = 0; i < LABEL_COUNT; ++i) {
        if (strcmp(label, LABELS[i].label) == 0) {
            return i;
        }
    }
    return -1;
}


// Stores the index of the label in the global "label" array.
// Returns -1 if given label is already present, otherwise 
// returns the total number of labels stored till this point.
int
storeLabelInformation(char* label, int position) {
    if (getLabelIndex(label) != -1) {
        return -1;
    }

    if (LABEL_COUNT >= TOTAL_LABELS) {
        printf("Instruction file cannot have more than %d labels.", TOTAL_LABELS);
    }

    LABELS[LABEL_COUNT].position = position;
    int i=0;
    while(label[i] != '\0') {
        LABELS[LABEL_COUNT].label[i] = label[i];
        ++i;
    }
    //printf("Stored '%s' at %d.\n", LABELS[LABEL_COUNT].label, LABELS[LABEL_COUNT].position);
    return LABEL_COUNT++;
}
