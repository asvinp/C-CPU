/*
 * cpu_constants.h: Header file defining all the constants required for best CPU
 * design.
*/

#ifndef CPU_HEAD
#define CPU_HEAD

#include <stdint.h>
#include <limits.h>

// Defining word size for CPU
#define WORD_SIZE 32

// Define a constant for byte size
#define BYTE_SIZE   8

// Define number of bytes in CPU word
#define NUM_BYTES_IN_WORD WORD_SIZE/BYTE_SIZE

// Define maximum number of General Purpose Registers
#define MAX_GPRS	16

// Defining Special Purpose Registers
#define PC  program_counter
#define MDR memory_data_
#define MAR memory_address_register
#define FLAGS   flags
#define HI hi_register
#define LO low_register
// Register for saving instruction obtained from memory
#define INSTR_REG   instruction_register
#define INSTR_MEMORY_PTR    instruction_memory_ptr


// Macro to initialize all SP registers
#define INIT_SPRS(type)   type FLAGS; type PC; type MDR; type MAR; type HI; type LO; type INSTR_REG; type INSTR_MEMORY_PTR;

// Define size of different registers based on word size
#define SIZE_8      char
#define SIZE_16     uint16_t
#define SIZE_32     uint32_t
#define SIZE_64     uint64_t

// Define the data type size to be used for declaring all registers based on
// word size
#define SIZE_TYPE SIZE_32


// Define the memory size based on WORD_SIZE
#define MEMORY_SIZE  (0x01 << 16)  // Memory is created as char array 2 ^ (word_size)


// Define memory location limit reserved for bootstrap code
#define BOOTSTRAP_MEMORY_SIZE	10
#define BOOTSTRAP_MEMORY_MAX	(1 << BOOTSTRAP_MEMORY_SIZE) - 1	// Saving 1 KB for bootstrap code

// Define memory location limit for instruction memory
#define INSTRUCTION_MEMORY_MAX      9215                      // 9215
#define INSTRUCTION_MEMORY_SIZE    INSTRUCTION_MEMORY_MAX - BOOTSTRAP_MEMORY_MAX   // 9215 - 1023 = 8 KB    
#define INSTRUCTION_MEMORY_MIN      BOOTSTRAP_MEMORY_MAX + 1        // 1024

// Define memory location for stack.
#define STACK_MEMORY_START  (MEMORY_SIZE)       // 65536

// Define memory range for data section.
#define DATA_MEMORY_MIN     INSTRUCTION_MEMORY_MAX + 1
#define DATA_MEMORY_SIZE    (1 << 13)       // 8 KB
#define DATA_MEMORY_MAX     DATA_MEMORY_MIN + DATA_MEMORY_SIZE - 1

// Declare global registers
SIZE_TYPE GPRS[MAX_GPRS];


// Define the SP and FP
#define SP  GPRS[14]
#define FP  GPRS[15]


// Declaring global memory
unsigned char MEMORY[MEMORY_SIZE];

// Defining global type pointer for memory access
typedef unsigned char* data_ptr;


// Initialize registers
INIT_SPRS(SIZE_TYPE);

// Define the supported opcodes by CPU
// Data Transfer
#define LOAD	"load"
#define STORE	"store"
#define MEM	    "mem"
#define LEA		"lea"
#define MOV     "mov"
#define MOVI    "movi"

// ALU instructions
#define ADD		"add"
#define ADDI    "addi"
#define SUBI    "subi"
#define SUB 	"sub"
#define DIV 	"div"
#define DIVI 	"divi"
#define MUL		"mul"
#define MULI	"muli"
#define MOD 	"mod"
#define MODI	"modi"
#define AND     "and"
#define ANDI	"andi"
#define OR      "or"
#define ORI     "ori"
#define XOR     "xor"
#define XORI    "xori"
#define NOR     "nor"
#define NORI    "nori"
#define SLT     "slt"
#define SLTI    "slti"
#define SLL	    "sll"
#define SLLI    "slli"
#define SRL	    "srl"
#define SRLI    "srli"
#define SRA     "sra"
#define SRAI    "srai"
#define NOT 	"not"

#define SLTU    "sltu"
// Branching instructions
#define JMP     "jmp"
#define JE      "je"
#define JNE     "jne"
#define JS      "js"
#define JNS     "jns"
#define JG      "jg"
#define JGE     "jge"
#define JL      "jl"
#define JLE     "jle"
// Procedure call instructions
#define RET     "ret"
#define CALL    "call"
// Stack operations instructions
#define PUSH    "push"
#define POP     "pop"


// Define supported register names
const char *valid_registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", \
	"r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "mdr", "mar"};

// Define supported opcodes
const char *valid_instructions[] = {LOAD, STORE, MEM, LEA, ADD, AND, ADDI, SUB, \
    SUBI, DIV, DIVI, MUL, MULI, MOD, MODI, AND, ANDI, OR, ORI, XOR, XORI, \
    NOR, NORI, SLT, SLTI, SLL, SLLI, SRL, SRLI, SRA, SRAI, SLTU, JMP, JE, JNE,\
    JS, JNS, JG, JGE, JL, JLE, RET, CALL, PUSH, POP, NOT, MOVI, MOV};

// Define number of valid register names
const int NUM_VALID_REGISTERS = sizeof(valid_registers)/sizeof(valid_registers[0]);

// Define number of valid cpu operations
const int NUM_VALID_OPCODES = sizeof(valid_instructions)/sizeof(valid_instructions[0]);

// Define different categories instructions
const char *R_INSTR[] = {NOT, AND, OR, XOR, ADD, SUB, DIV, MUL, MOD, NOR, SLT, SLL, SRL, SRA, SLTU};
const char *I_INSTR[] = {ADDI, SUBI, DIVI, MULI, MODI, ANDI, ORI, XORI, NORI, SLTI, SLLI, SRLI, SRAI};
const char *MEM_INSTR[] = {LOAD, STORE, LEA};
const char *CONTROL_INSTR[] = {JMP, JE, JNE, JS, JNS, JG, JGE, JL, JLE, CALL};
const char *STACK_INSTR[] = {PUSH, POP};
const char *NO_OPERAND_INSTR[] = {RET};
const char *MEM_DISPLAY_INSTR[] = {MEM};
const char *MOV_INSTR[] = {MOV, MOVI};

// Define the number of instructions in all categories
const int NUM_VALID_R_INSTR = sizeof(R_INSTR)/sizeof(R_INSTR[0]);
const int NUM_VALID_I_INSTR = sizeof(I_INSTR)/sizeof(I_INSTR[0]);
const int NUM_VALID_MEM_INSTR = sizeof(MEM_INSTR)/sizeof(MEM_INSTR[0]);
const int NUM_VALID_CONTROL_INSTR = sizeof(CONTROL_INSTR)/sizeof(CONTROL_INSTR[0]);
const int NUM_VALID_STACK_INSTR = sizeof(STACK_INSTR)/sizeof(STACK_INSTR[0]);
const int NUM_VALID_NO_OPERAND_INSTR = sizeof(NO_OPERAND_INSTR)/sizeof(NO_OPERAND_INSTR[0]);
const int NUM_VALID_MEM_DISPLAY_INSTR = sizeof(MEM_DISPLAY_INSTR)/sizeof(MEM_DISPLAY_INSTR[0]);
const int NUM_VALID_MOV_INSTR = sizeof(MOV_INSTR)/sizeof(MOV_INSTR[0]);

// Define hex value to set/get condition flags
// These values are used to set/get specific bits from FLAGS register
#define HEX_SF  0x80
#define HEX_OF  0x40
#define HEX_PF  0x10
#define HEX_ZF  0x04
#define HEX_CF  0x01

// Enum to define various flags/condition codes
typedef enum {SF, OF, PF, ZF, CF} status_flags;


// Define a struct for instructions and corresponding binary notation
struct instr_opcode {
    char* instruction;
    int opcode;
};


#define TOTAL_LABELS 100
struct label_pos {
    char label[50];
    int position;
};

int LABEL_COUNT = 0;
struct label_pos LABELS[TOTAL_LABELS];

// Define the opcodes for all instructions
#define TOTAL_ASSEMBLY_OPCODES  46

// Create an array of structs for all instructions and binary opcode mapping
struct instr_opcode opcode_map[TOTAL_ASSEMBLY_OPCODES] = {
    {LOAD, 0x00},
    {STORE, 0x01},
    {MEM, 0x02},
    {MOV, 0x03},
    {MOVI, 0x04},
    {LEA, 0x05},

    {ADD, 0x20},
    {SUB, 0x21},
    {MUL, 0x22},
    {DIV, 0x23},
    {MOD, 0x24},
    {AND, 0x25},
    {OR, 0x26},
    {XOR, 0x27},
    {NOR, 0x28},
    {SLL, 0x29},
    {SLT, 0x2A},
    {SRL, 0x2B},
    {SRA, 0x2C},
    {SLTU, 0x2D},

    {ADDI, 0x30},
    {SUBI, 0x31},
    {MULI, 0x32},
    {DIVI, 0x33},
    {MODI, 0x34},
    {ANDI, 0x35},
    {ORI, 0x36},
    {XORI, 0x37},
    {NORI, 0x38},
    {SLLI, 0x39},
    {SLTI, 0x3A},
    {SRLI, 0x3B},
    {SRAI, 0x3C},

    {JMP, 0x10},
    {JE, 0x11},
    {JNE, 0x12},
    {JS, 0x13},
    {JNS, 0x14},
    {JG, 0x15},
    {JGE, 0x16},
    {JL, 0x17},
    {JLE, 0x18},

    {RET, 0x08},
    {CALL, 0x09},

    {PUSH, 0x0A},
    {POP, 0x0B}
};

// Define constants for differentiating between various instructions opcode
// formats
typedef enum {LOAD_STORE, REG_REG, REG_MEM, MEM_REG, IMM_REG, IMM_MEM, MEM_DISPLAY, \
    CONTROL_LABEL, STACK_REG, NO_OPERAND, MOV_REG_REG, MOV_IMM_REG} opcode_formats;

// Struct to store different attributes of an instruction
struct instruction_attr {
    opcode_formats format;      // Opcode format
    char instruction[10];          // Instruction code
    int base_register;          // Base register for generic memory address
    int index_register;         // Index register for generic memory address
    int offset;                 // Offset/displacement for generic memory address
    int scale;                  // Scale factor for generic memory address
    int operand_register;       // Operand register
    int const_or_label;         // Immediate constant or label offset for control transfer/mov
};

#define REG_REG_IND 0x01
#define REG_MEM_IND 0x02
#define MEM_REG_IND 0x03

#define MOV_REG_REG_IND 0x00
#define MOV_IMM_REG_IND 0x01

#endif
