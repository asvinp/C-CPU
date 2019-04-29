/*
 *  cpu_main.c: File for designing and implementing basic CPU design 
 *  operations.
*/

#include "cpu_constants.h"
#include "cpu_utils.c"
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

extern SIZE_TYPE GPRS[MAX_GPRS];

void setFlagsRegister(SIZE_TYPE val1, SIZE_TYPE val2, SIZE_TYPE result);
bool getFlagStatusFromFlagsRegister(status_flags input_flag);
bool isSubtract = false;
void checkValidMemoryAccess(SIZE_TYPE memory_address);

//#############################################################################
////////////////////////// General Functions Section //////////////////////////
//#############################################################################

/*
 * Function to read given number of bytes from memory.
 * Input arguments:
 *
 *  start_index: Start location in the memory to read data from.
 *  num_bytes: Number of bytes to be read from memory starting from the start_index.
 *  
 * Return Value:
 *  Result data read from memory.
 */
SIZE_TYPE
readFromMemory(SIZE_TYPE start_index, int num_bytes) {
    SIZE_TYPE index;
    SIZE_TYPE result = 0;

    // Read memory contents byte by byte and form final result
    for (index = (start_index + num_bytes - 1); 
         index >= start_index; 
         index--) 
    {
        // Shift the bytes read from memory for final result 
        result = result << 8;
        result = result | MEMORY[index];
    }
    return result;
}


/*
 * Function to write given number of bytes to memory.
 * Input arguments:
 *
 *  start_index: Start location of memory to write data into.
 *  num_bytes: Number of contiguous bytes to be written into memory starting at start_index.
 *  data: Data to be written into memory.
 */
void
writeIntoMemory(SIZE_TYPE start_index, int num_bytes, data_ptr data) {
    int i;    
    SIZE_TYPE index;
    // Storing data in Little Endian format with LSB at the start memory location
    for (i = 0, index = start_index; i < num_bytes; i++, index++) {
        MEMORY[index] = data[i];
    }
}


/*
 * Function to save the binary opcode to instruction memory region.
 */
void
saveInstructionToMemory(SIZE_TYPE opcode) {
    writeIntoMemory(INSTR_MEMORY_PTR, NUM_BYTES_IN_WORD, (data_ptr) &opcode);
    printf("====> Memory Location: %u, Binary Opcode: %x\n", INSTR_MEMORY_PTR, opcode);
    INSTR_MEMORY_PTR = INSTR_MEMORY_PTR + NUM_BYTES_IN_WORD;
}


/*
 * Function to display contents of all registers.
 */
void
displayRegisters() {
    printf("\n--------------------------------Displaying Register contents---------------------------\n");
    printf("Register Name \t : Value (Hex) \t : Value (Unsigned Decimal) : Value (Two's Complement) \n");
    printf("---------------------------------------------------------------------------------------\n");
    
    int i;
    for (i = 0; i < MAX_GPRS; i++) {
        printf("R%u \t\t : 0x%10x : %25u : %20d \n", i, GPRS[i], GPRS[i], GPRS[i]); 
    }
    printf("HI \t\t : 0x%10x : %25u : %20d \n", HI, HI, HI);
    printf("LO \t\t : 0x%10x : %25u : %20d \n", LO, LO, LO);
    printf("\nMDR \t\t : 0x%10x : %25u : %20d \n", MDR, MDR, MDR);
    printf("MAR \t\t : 0x%10x : %25u : %20d \n", MAR, MAR, MAR);
    printf("FLAGS \t\t : 0x%10x : %25u : %20d \n", FLAGS, FLAGS, FLAGS);
    printf("PC \t\t : 0x%10x : %25u : %20d \n", PC, PC, PC);
    printf("SP R14\t\t : 0x%10x : %25u : %20d \n", GPRS[14], GPRS[14], GPRS[14]);
    printf("FP R15\t\t : 0x%10x : %25u : %20d \n", GPRS[15], GPRS[15], GPRS[15]);

    // Explicitly display the flags values
    bool sf_status = getFlagStatusFromFlagsRegister(SF);
    bool of_status = getFlagStatusFromFlagsRegister(OF);
    bool pf_status = getFlagStatusFromFlagsRegister(PF);
    bool zf_status = getFlagStatusFromFlagsRegister(ZF);
    bool cf_status = getFlagStatusFromFlagsRegister(CF);
    printf("\n\nCondition Codes/Status Flags: SF: %d    OF: %d    PF: %d    ZF: %d    CF: %d    \n\n",
            sf_status, of_status, pf_status, zf_status, cf_status);
}


/*
 * Function to display memory contents for specific memory locations.
 * Input arguments:
 *
 *  start_index: Start index of memory location to display data. Default set to 0.
 *  end_index: End inxdex of memory location to display data. Default set to 0.
 */
void 
displayMemoryInRange(SIZE_TYPE start_index, SIZE_TYPE end_index) {
    SIZE_TYPE final_index = end_index;
    int i;

    // Validate the two indices and set range accordingly
    if (start_index > end_index) {
        final_index = start_index;
    }

    /*
    // Display memory contents
    printf("Displaying Memory contents (%#x to %#x)\n", start_index, final_index);
    printf("-----------------------------------------------------------------\n");
    printf("Location(Hex) \t : \t Contents(Hex) \t : \t Contents(Decimal)\n");
    printf("-----------------------------------------------------------------\n");
    for (i = final_index; i >= start_index; i--) {
        printf("0x%x     \t : \t 0x%.2x     \t : \t %16d \n", i, MEMORY[i], MEMORY[i]);
    }
    */
    // Display memory contents
    printf("Displaying Memory contents (%#x to %#x)\n", start_index, final_index);
    printf("---------------------------------------------------------------------------\n");
    printf("Location(Hex) \t : \t Contents(Hex) \t : \t Contents(Decimal)\n");
    printf("---------------------------------------------------------------------------\n");
    for (i = final_index; i >= start_index; i = i - 4) {
        printf("0x%x     \t : \t 0x%.8x     \t : \t %16d \n", i, readFromMemory(i, 4), readFromMemory(i, 4));
    }
    printf("---------------------------------------------------------------------------\n\n");
    printf("---------------------------------------------------------------------------\n\n");
}


/*
 * Function to display instruction memory contents from start of instruction memory
 * to INSTR_MEMORY_PTR.
 */
void
displayInstructionMemory() {
    // Display instruction memory contents
    displayMemoryInRange(INSTRUCTION_MEMORY_MIN, INSTR_MEMORY_PTR - 1);
}

/*
 * Function to load data from a memory location into register.
 * Input arguments:
 *
 *  reg: Address of register to load the data into.
 *  memory_addr: Address/memory location to load data into given register.
 */
void
loadRegister(SIZE_TYPE *reg, SIZE_TYPE memory_addr){
    *reg = readFromMemory(memory_addr, NUM_BYTES_IN_WORD);
}


/*
 * Function to store data to a memory location from register.
 * Input arguments:
 *
 *  reg: Address of register to store data from.
 *  memory_addr: Address/memory location to load data into.
 */
void
storeRegister(SIZE_TYPE *reg, SIZE_TYPE memory_addr) {
    SIZE_TYPE reg_value = *reg;
    writeIntoMemory(memory_addr, NUM_BYTES_IN_WORD, (data_ptr) &reg_value);
}

/*
 * Function to perform bitwise AND of two values. The function performs 
 * val2 & val1 and sends the result.
 */
SIZE_TYPE 
and(SIZE_TYPE val1, SIZE_TYPE val2) {
    return val1 & val2;
}

/*
 * Function to perform bitwise OR of two values. The function performs 
 * val2 | val1 and sends the result.
 */
SIZE_TYPE 
or(SIZE_TYPE val1, SIZE_TYPE val2) {
    return val1 | val2;
}

/*
 * Function to perform bitwise XOR of two values. The function performs 
 * val2 ^ val1 and sends the result.
 */
SIZE_TYPE 
xor(SIZE_TYPE val1, SIZE_TYPE val2) {
    return val1 ^ val2;
}

/*
 * Function to add two values.
 */
SIZE_TYPE 
add(SIZE_TYPE val1, SIZE_TYPE val2) {
    SIZE_TYPE carry;
    // Iterate till there is no carry
    while (val1 != 0)
    {
        // carry now contains common set bits of x and y
        carry = val2 & val1;

        // Sum of bits of x and y where at least one of the bits is not set
        val2 = val2 ^ val1;

        // Carry is shifted by one so that adding it to x gives the required sum
        val1 = carry << 1;
    }
    return val2;
}

/*
 * Funtion to return 2's Complement of a value
 */
SIZE_TYPE 
Twos_Complement(SIZE_TYPE x){
    return add(~x, 1);
}

/*
 * Funtion to return negation of a value
 */
SIZE_TYPE 
not(SIZE_TYPE x){
    return ~x;
}

/*
 * Function to perform NOR operation. The function performs ~(val2 | val1) and sends the result.
 */
SIZE_TYPE 
nor(SIZE_TYPE val1, SIZE_TYPE val2) {
    return not(val1 | val2);
}


/*
 * Function to subtract two values. The function performs val2 - val1 and sends the result.
 */
SIZE_TYPE 
subtract(SIZE_TYPE val1, SIZE_TYPE val2) {
    isSubtract = true;
    SIZE_TYPE borrow;
    // Iterate till there is no carry
    while (val1 != 0)
    {
        // borrow contains common set bits of y and unset
        // bits of x
        borrow = (~val2) & val1;

        // Subtraction of bits of x and y where at least
        // one of the bits is not set
        val2 = val2 ^ val1;

        // Borrow is shifted by one so that subtracting it from
        // x gives the required sum
        val1 = borrow << 1;
    }
    return val2;
}

SIZE_TYPE tmpsubtract(SIZE_TYPE val1, SIZE_TYPE val2){
    return add(val1, Twos_Complement(val2));
}

/*
 * Function to multiply two values
 */
SIZE_TYPE 
multiply(SIZE_TYPE val1, SIZE_TYPE val2) {
    SIZE_TYPE result = 0;
    while (val2 != 0)                  // Iterate the loop till b == 0
    {
        if (val2 & 0x01)               // Bitwise & of the value of b with 01
        {
            result = result + val1;    // Add a to result if b is odd .
        }
        val1 <<= 1;                    // Left shifting the value contained in 'a' by 1
                                       // Multiplies a by 2 for each loop
        val2 >>= 1;                    // Right shifting the value contained in 'b' by 1.
    }
    return result;
}

/*
 * Function to perform divide operation.
 */
SIZE_TYPE 
divide(SIZE_TYPE val1, SIZE_TYPE val2) {
    SIZE_TYPE c=0,sign=0;

    if(val1 < 0) {
       val1 = Twos_Complement(val1);
           sign ^= 0x01;
   }

   if(val2 < 0) {
       val2 = Twos_Complement(val2);
       sign ^= 0x01;
   }

   if(val2 != 0) {
       while (val1 >= val2) {
           val1 = tmpsubtract(val1,val2);
           c = add(c,1);
       }
       HI = val1 ; // this gives mod answer
   }
   if (sign) {
       c = Twos_Complement(c);
   }
   LO = c;
   return c;
}


/*
 * Function to set CF if the val2 is less than val1. 
 */
SIZE_TYPE 
slt(SIZE_TYPE val1, SIZE_TYPE val2) {
    if ((subtract(val1, val2)) < 0)
        return 1;
    else
        return 0;
}


/*
 * Function to left shift logical val2 by val1 positions.
 */
SIZE_TYPE 
sll(SIZE_TYPE val1, SIZE_TYPE val2) {
     return val2 << val1;
}


/*
 * Function to right shift logical val2 by val1 positions.
 */
SIZE_TYPE 
srl(SIZE_TYPE val1, SIZE_TYPE val2) {
     return (val2 >> val1);
}


/*
 * Function to right shift arithmetic val2 by val1 positions.
 */
SIZE_TYPE
sra(SIZE_TYPE val1, SIZE_TYPE val2){
    // Get the MSB of val2
    int msb_temp = val2 >> (WORD_SIZE - 1);
    int z = 0x00;
    if (msb_temp) {
        z = ~z;
    }
    int temp = srl(val1, val2);
    int x = sll(z, val1);
    return (x | temp);
}
 

/*
 * Function to set different condition flags bit based on the result of the
 * previously executed ALU instruction.
 * The FLAGS register bits representing basic condition flags are:
 *   7th bit: SF
 *   6th bit: OF
 *   5th bit: -
 *   4th bit: PF
 *   3rd bit: -
 *   2nd bit: ZF
 *   1st bit: -
 *   0th bit: CF
 */
void
setFlagsRegister(SIZE_TYPE val1, SIZE_TYPE val2, SIZE_TYPE result) {
    // Set 7th bit: SF
    // MSB of result = 1, SF = 1 else SF = 0
    int res_msb = (result >> (WORD_SIZE - 1)) & 0x01;
    if (res_msb) {
        FLAGS = FLAGS | HEX_SF;
    } else {
        FLAGS = FLAGS & (~HEX_SF);
    }

    // Set 6th bit: OF
    // Set for signed addition or subraction
    // Addition:
    //  if (a > 0 && b > 0 && res <= 0) || (a < 0 && b < 0 && res >=0)
    //  result = 1 else 0
    // Subtraction:
    //  if (a > 0 && b < 0 && res < 0) || (a < 0 && b > 0 && res > 0)
    //  result = 1 else 0
    int val1_signed = (int) val1;
    int val2_signed = (int) val2;
    int result_signed = (int) result;
    bool sum_condition = ((val1_signed > 0 && val2_signed > 0 && result_signed <= 0) || (val1_signed < 0 && val2_signed < 0 && result_signed >= 0));
    bool sub_condition = ((val2_signed > 0 && val1_signed < 0 && result_signed < 0) || (val2_signed < 0 && val1_signed > 0 && result_signed > 0));
    if (isSubtract && sub_condition){
        FLAGS = FLAGS | HEX_OF;
    } else if(!isSubtract && sum_condition) {
        FLAGS = FLAGS | HEX_OF;
    } else {
        FLAGS = FLAGS & (~HEX_OF);
    }

    // Set 4th bit: PF
    // PF = 1 if result has even parity else 0
    // Count the one's in the result
    SIZE_TYPE count_ones = 0;
    int i =0;
    for (i = 0; i < WORD_SIZE; i++) {
        SIZE_TYPE res_temp = result >> i;
        int val = res_temp & 0x01;
        count_ones += val;
    }
    SIZE_TYPE rem = divide(count_ones, 2);
    // Divide instruction stores the mod in HI register
    if (HI != 0) {
        // Odd parity, set PF = 1
        FLAGS = FLAGS | HEX_PF; 
    } else {
        FLAGS = FLAGS & (~HEX_PF);
    }

    // Set 2th bit: ZF
    // ZF = 1 if result = 0 else 0
    if (result == 0) {
        FLAGS = FLAGS | HEX_ZF;
    } else {
        FLAGS = FLAGS & (~HEX_ZF);
    }

    // Set 0th bit: CF
    // Set for unsigned carry CF = 1 in case of carry from MSB
    // Carry occurs in unsigned addition when sum < one of the operands
    if (result < val1 || result < val2) {
        FLAGS = FLAGS | HEX_CF;
    } else {
        FLAGS = FLAGS & (~HEX_CF);
    }
}

/*
 * Function to get values of different status flags
 */
bool
getFlagStatusFromFlagsRegister(status_flags input_flag) {
    bool is_flag_set = false;
    SIZE_TYPE flag_hex_value = 0x00;
    switch(input_flag) {
        case SF:       
            flag_hex_value = HEX_SF;
            break;
        case OF:
            flag_hex_value = HEX_OF;
            break;
        case PF:
            flag_hex_value = HEX_PF;
            break;
        case ZF:
            flag_hex_value = HEX_ZF;
            break;
        case CF:
            flag_hex_value = HEX_CF;
            break;
        default:
            printf("ERROR: Unsupported Flag Register Type passed.\n");
            exit(0);
    }
    // This checks if the flag's value is 0 or 1
    // If 1, it returns true else false
    is_flag_set = ((FLAGS & flag_hex_value) != 0) ? true : false;
    return is_flag_set;
}


//#############################################################################
/////////////////////////// Functions for Assembly Commands ///////////////////
//#############################################################################

/*
 * Function to execute load command.
 * Input Arguments:
 * 
 * reg: Register to load memory contents into.
 * memory_addr: Address of memory to load data from.
 */
void
executeLoad(SIZE_TYPE reg_to_load, SIZE_TYPE memory_addr) {
    loadRegister(&GPRS[reg_to_load], memory_addr);
    MAR = memory_addr;
    MDR = GPRS[reg_to_load];
}


/*
 * Function to execute store command.
 * Input Arguments:
 * 
 * reg: Register to store memory contents from.
 * memory_addr: Address of memory to load data into.
 */
void
executeStore(SIZE_TYPE reg_to_store, SIZE_TYPE memory_addr) {
    storeRegister(&GPRS[reg_to_store], memory_addr);
    MAR = memory_addr;
    MDR = GPRS[reg_to_store];
}

/*
 * Function to execute mem command.
 */
void
executeMem(char *start_addr, char *end_addr) {
    char *ptr;
    SIZE_TYPE start = strtol(start_addr, &ptr, 10);
    SIZE_TYPE end = strtol(end_addr, &ptr, 10);

    // Invalid address rabge passed
    if (start > end) {
        printf("ERROR: Invalid address range passed.\n");
        exit(1);
    }
    displayMemoryInRange(start, end);
}

/*
 * Function to execute 'NOT' command.
 */ 
void 
executeNOT(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = not (op1);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute 'AND' command.
 */ 
void 
executeAND(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = and (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute 'ANDI' command.
*/
void 
executeANDI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = and (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute 'OR' command.
 */ 
void 
executeOR(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = or (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute 'ORI' command.
*/
void 
executeORI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = or (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute 'XOR' command.
 */ 
void 
executeXOR(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = xor (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute 'XORI' command.
*/
void 
executeXORI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = xor (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute 'NOR' command.
 */ 
void 
executeNOR(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = nor (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute 'NORI' command.
*/
void 
executeNORI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = nor (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute 'SLT' command.
 */ 
void 
executeSLT(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = subtract (op1, op2);
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute 'SLTI' command.
*/
void 
executeSLTI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = subtract (constant, op2);
    //*ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute 'SLL' command.
 */ 
void 
executeSLL(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = sll (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute 'SLLI' command.
*/
void 
executeSLLI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = sll (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute 'SRL' command.
 */ 
void 
executeSRL(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = srl (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute 'SRLI' command.
*/
void 
executeSRLI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = srl (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/* 
 * Function to execute SRA command. 
 */
void 
executeSRA(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = sra (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute SRAI command.
 */
void 
executeSRAI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = sra(op2, constant);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute Push command
 */
void
executePush(SIZE_TYPE* arg1){
    SIZE_TYPE op1 = *arg1;
    SP = SP - 4;
    writeIntoMemory(SP, NUM_BYTES_IN_WORD, (data_ptr) &op1);
}

/*
 * Function to execute Pop command
 */
void
executePop(SIZE_TYPE* reg) {
    *reg = readFromMemory(SP, NUM_BYTES_IN_WORD);
    SP = SP + 4;
}

/*
 * Function to execute Add command.
 */ 
void 
executeAdd(SIZE_TYPE* arg1, SIZE_TYPE* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = add (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute addi command.
*/
void 
executeAddI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = add (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute sub command.
*/
void 
executeSub(int* arg1, int* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = subtract (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute subi command.
 */
void 
executeSubI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = subtract (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute addi command.
 */
void 
executeMul(int* arg1, int* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = multiply (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute multiplication with immediate value.
 */
void 
executeMulI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = multiply (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute division.
 */
void 
executeDiv(int* arg1, int* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = divide (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/*
 * Function to execute division with immediate value.
 */
void 
executeDivI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = divide (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute modulas.
 */
void
executeMod(int* arg1, int* arg2) {
    SIZE_TYPE op1 = *arg1;
    SIZE_TYPE op2 = *arg2;
    SIZE_TYPE result = divide (op1, op2);
    *arg2 = result;
    setFlagsRegister(op1, op2, result);
}

/* 
 * Function to execute modulas with immediate value.
 */
void 
executeModI(SIZE_TYPE constant, SIZE_TYPE* ptr) {
    SIZE_TYPE op2 = *ptr;
    SIZE_TYPE result = divide (constant, op2);
    *ptr = result;
    setFlagsRegister(constant, op2, result);
}

/*
 * Function to execute MOV instruction.
 */
void
executeMov(SIZE_TYPE *op1, SIZE_TYPE* op2) {
    // Move the contents of op1 to op2.
    *op2 = *op1;
}

/*
 * Function to execute MOVI command.
 */
void
executeMovI(SIZE_TYPE constant, SIZE_TYPE *ptr) {
    // Copy the constant value to the ptr location.
    *ptr = constant;
}

/*
 * Function to execute LEA instruction.
 */
void
executeLea(SIZE_TYPE addr, SIZE_TYPE reg) {
    SIZE_TYPE *reg_ptr = (SIZE_TYPE *) &GPRS[reg];
    *reg_ptr = addr;
}


/*
 * Function to execute CALL command.
 */
void
executeCall(int label_offset) {
    // Push the return address to stack
    executePush(&PC);

    // Set the new value of PC = PC + label_offset * 4
    PC = PC + (label_offset * 4);
}

/*
 * Function to execute JMP command.
 */
void
executeJmp(int label_offset) {
    // Set the new value of PC = PC + label_offset * 4
	PC = PC + (label_offset * 4);  
}

/*
 * Function to execute JE command. It checks the status of ZF.
 */
void
executeJE(int label_offset) {
    bool ZeroF = getFlagStatusFromFlagsRegister(ZF);
	if (ZeroF) {
		PC = PC + (label_offset * 4); 
	}     
}

/*
 * Function to execute JNE command.
 */
void
executeJNE(int label_offset) {
    bool ZeroF = getFlagStatusFromFlagsRegister(ZF);
	if (!ZeroF) {
		PC = PC + (label_offset * 4); 
	}     
}

/*
 * Function to execute JS command.
 */
void
executeJS(int label_offset) {
    bool SignedF = getFlagStatusFromFlagsRegister(SF);
	if (SignedF) {
		PC = PC + (label_offset * 4); 
	}     
}

/*
 * Function to execute JNS command. It checks the SF.
 */
void
executeJNS(int label_offset) {
    bool SignedF = getFlagStatusFromFlagsRegister(SF);
	if (!SignedF) {
		PC = PC + (label_offset * 4); 
	}     
}

/*
 * Function to execute JG (signed) command. The instruction gets executed
 * after checking the following flags:
 *  ~(SF ^ OF) & ~ZF
 */
void
executeJG(int label_offset) {
    bool SignedF = getFlagStatusFromFlagsRegister(SF);
    bool OverflowF = getFlagStatusFromFlagsRegister(OF);
    bool ZeroF = getFlagStatusFromFlagsRegister(ZF);
    
    if (!ZeroF && !(SignedF ^ OverflowF)) {
		PC = PC + (label_offset * 4); 
	}
}

/*
 * Function to execute JGE (signed) command. The instruction gets executed
 * after checking the following status condition:
 *  ~(SF ^ OF)
 */
void
executeJGE(int label_offset) {
    bool SignedF = getFlagStatusFromFlagsRegister(SF);
    bool OverflowF = getFlagStatusFromFlagsRegister(OF);
	
    if( !(SignedF ^ OverflowF)) {
		PC = PC + (label_offset * 4); 
	}
}

/*
 * Function to execute JL command. The instruction gets executed after checking
 * the following status condition:
 *  (SF ^ OF)
 */
void
executeJL(int label_offset) {
    bool SignedF = getFlagStatusFromFlagsRegister(SF);
    bool OverflowF = getFlagStatusFromFlagsRegister(OF);

	if (SignedF ^ OverflowF) {
		PC = PC + (label_offset * 4); 
	}
}

/*
 * Function to execute JLE command. The instruction gets executed after checking
 * the following status condition:
 *  (SF ^ OF) | ZF
 */
void
executeJLE(int label_offset) {
    bool SignedF = getFlagStatusFromFlagsRegister(SF);
    bool OverflowF = getFlagStatusFromFlagsRegister(OF);
    bool ZeroF = getFlagStatusFromFlagsRegister(ZF);

	if((SignedF ^ OverflowF) || ZeroF) {
		PC = PC + (label_offset * 4); 
	}
}

/*
 * Function to execute RET command.
 */
void
executeRet() {
    executePop(&PC);
}



//#############################################################################
///////////////////// Decode and Execution Section ////////////////////////////
//#############################################################################

/*
 * Function to compute memory address in case of generic memory addressing operand.
 */
SIZE_TYPE
computeMemoryAddressFromOpcode(struct instruction_attr* instr_attr_ptr) {
    SIZE_TYPE address;
    int base_reg = instr_attr_ptr->base_register;
    int index_reg = instr_attr_ptr->index_register;
    int scale = instr_attr_ptr->scale;
    int offset = instr_attr_ptr->offset;

    address = GPRS[base_reg] + (GPRS[index_reg] * scale) + offset;
    checkValidMemoryAccess(address);
    return address;
}

/*
 * Function to execute Load/Store i.e. memory type instructions.
 */
void
executeMemoryTypeInstructions(struct instruction_attr* instr_attr_ptr) {
    char *command = instr_attr_ptr->instruction;
    
    // Compute memory address
    SIZE_TYPE memory_address = computeMemoryAddressFromOpcode(instr_attr_ptr);
    int reg = instr_attr_ptr->operand_register;

    // Load command
    if (strcmp(command, LOAD) == 0) {
        executeLoad(reg, memory_address);
    }
    // Store command
    if (strcmp(command, STORE) == 0) {
        executeStore(reg, memory_address);
    }
    // LEA command
    if (strcmp(command, LEA) == 0) {
        executeLea(memory_address, reg);
    }
}

/*
 * Function to execute Stack instructions. These instruction formats are
 * supported:
 *  STACK_REG: e.g. push r1
 *  STACK_REG: e.g. pop r2
 */
void
executeStackInstructions(struct instruction_attr* instr_attr_ptr) {
    char *command = instr_attr_ptr->instruction;
    int reg = instr_attr_ptr->operand_register;
    int *address[1];
    // Find parameters based on specific format reg-reg/reg-mem/mem-reg.
    switch(instr_attr_ptr->format) {
        default:
            printf("ERROR: Unsupported instruction format for Stack instructions.\n");
            exit(0);
        case STACK_REG:
            address[0] = &GPRS[instr_attr_ptr->operand_register];
            break;
    }

    // PUSH command
    if (strcmp(command, PUSH) == 0) {
        executePush(address[0]);
    }
    
    // POP command
    if (strcmp(command, POP) == 0) {
        executePop(address[0]);
    }
}

/*
 * Function to execute all R-Type instructions. These instruction formats are
 * supported:
 *  REG_REG: e.g. add r2, r3
 *  REG_MEM: e.g. add r2, 4(r3 + r4)
 *  MEM_REG: e.g. add 4(r3 + r4), r2
 */
void
executeRTypeInstructions(struct instruction_attr* instr_attr_ptr) {
    char *command = instr_attr_ptr->instruction;
    int *address[2];

    // Find parameters based on specific format reg-reg/reg-mem/mem-reg.
    switch(instr_attr_ptr->format) {
        default:
            printf("ERROR: Unsupported instruction format for R-Type instructions.\n");
            exit(0);
        case REG_REG:
            address[0] = &GPRS[instr_attr_ptr->operand_register];
            address[1] = &GPRS[instr_attr_ptr->base_register];
            break;
        case REG_MEM:
            address[0] = &GPRS[instr_attr_ptr->operand_register];
            address[1] = (SIZE_TYPE*) &MEMORY[computeMemoryAddressFromOpcode(instr_attr_ptr)];
            break;
        case MEM_REG:
            address[1] = &GPRS[instr_attr_ptr->operand_register];
            address[0] = (SIZE_TYPE *) &MEMORY[computeMemoryAddressFromOpcode(instr_attr_ptr)];
            break;
    }

    // ADD command
    if (strcmp(command, ADD) == 0) {
        executeAdd(address[0], address[1]);
    }
    // SUB command
    if (strcmp(command, SUB) == 0) {
        executeSub(address[0], address[1]);
    }
    // MUL command
    if (strcmp(command, MUL) == 0) {
        executeMul(address[0], address[1]);
    }
    // DIV command
    if (strcmp(command, DIV) == 0) {
        executeDiv(address[0], address[1]);
     }
    // MOD command
    if (strcmp(command, MOD) == 0) {
        executeMod(address[0], address[1]);
    }
    // AND Command
    if (strcmp(command, AND) == 0) {
    	executeAND(address[0], address[1]);
    }
    // OR Command
    if(strcmp(command, OR) == 0) {
	    executeOR(address[0], address[1]);
    }
    // XOR Command
    if(strcmp(command, XOR) == 0) {
    	executeXOR(address[0], address[1]);
    }
    // NOT Command
    if(strcmp(command, NOT) == 0) {
	    executeNOT(address[0], address[1]);
    }
    // NOR Command
    if(strcmp(command, NOR) == 0) {
	    executeNOR(address[0], address[1]);
    }
    // SLT Command
    if(strcmp(command, SLT) == 0) {
	    executeSLT(address[0], address[1]);
    }
    // SLL Command
    if(strcmp(command, SLL) == 0) {
	    executeSLL(address[0], address[1]);
    }
    // SRL Command
    if(strcmp(command, SRL) == 0) {
    	executeSRL(address[0], address[1]);
    }
    //SRA command
    if(strcmp(command, SRA) == 0) {
	    executeSRA(address[0], address[1]);
    }
}

/*
 * Function to execute all Immediate-Type instructions. Two instruction formats
 * are supported:
 *  IMM_REG: e.g. addi $0x01, r4
 *  IMM_MEM: e.g. addi $0x01, (r4)
 */
void
executeITypeInstructions(struct instruction_attr* instr_attr_ptr) {
    char *command = instr_attr_ptr->instruction;
    SIZE_TYPE constant = instr_attr_ptr->const_or_label;

    SIZE_TYPE *p;
    int reg_index;
    switch(instr_attr_ptr->format) {
         default:
            printf("ERROR: Unsupported instruction format for Imm-Type instructions.\n");
            exit(0);
         case IMM_REG:
            reg_index = instr_attr_ptr->operand_register;
            p = &GPRS[reg_index];
            break;
        case IMM_MEM:
            p = (SIZE_TYPE *) &MEMORY[computeMemoryAddressFromOpcode(instr_attr_ptr)];
            break;
    }

    // ADDI command
    if (strcmp(command, ADDI) == 0) {
        executeAddI(constant, p);
    }
    // SUBI command
    if (strcmp(command, SUBI) == 0) {
        executeSubI(constant, p);
    }
    // MULI command
    if (strcmp(command, MULI) == 0) {
        executeMulI(constant, p);
    }
    // DIVI command
    if (strcmp(command, DIVI) == 0) {
        executeDivI(constant, p);
     }
    // MODI command
    if (strcmp(command, MODI) == 0) {
        executeModI(constant, p);
     }
    // ANDI Command
    if (strcmp(command, AND) == 0) {
	    executeANDI(constant, p);
    }
    // ORI Command
    if(strcmp(command, ORI) == 0) {
    	executeORI(constant, p);
    }
    // XORI Command
    if(strcmp(command, XORI) == 0) {
	    executeXORI(constant, p);
    }
    // NORI Command
    if(strcmp(command, NORI) == 0) {
	    executeNORI(constant, p);
    }
    // SLTI Command
    if(strcmp(command, SLTI) == 0) {
    	executeSLTI(constant, p);
    }
    // SLLI Command
    if(strcmp(command, SLLI) == 0) {
	    executeSLLI(constant, p);
    }
    // SRLI Command
    if(strcmp(command, SRLI) == 0) {
	    executeSRLI(constant, p);
    }
     //SRAI command
    if(strcmp(command, SRAI) == 0) {
    	executeSRAI(constant, p);
    }
}

/*
 * Function to execute memory display special instruction. Command syntax:
 * mem const, reg
 * It displays the constents of memory from reg to reg + const.
 */
void
executeMemoryDisplayInstructions(struct instruction_attr* instr_attr_ptr) {
    SIZE_TYPE start_addr = GPRS[instr_attr_ptr->operand_register];
    int offset = instr_attr_ptr->const_or_label;
    SIZE_TYPE end_addr = start_addr + offset;

    // Since the offset is signed, hence modifying the range.
    if (end_addr < start_addr) {
        SIZE_TYPE temp = start_addr;
        start_addr = end_addr;
        end_addr = temp;
    }
    displayMemoryInRange(start_addr, end_addr);
}

/*
 * Function to execute control transfer instructions.
 */
void
executeControlTransferInstructions(struct instruction_attr *instr_attr_ptr) {
    char *command = instr_attr_ptr->instruction;
    int label_offset = instr_attr_ptr->const_or_label;

    // CALL command
    if (strcmp(command, CALL) == 0) {
        executeCall(label_offset);
    }

    // JMP command
    if (strcmp(command, JMP) == 0) {
        executeJmp(label_offset);
    }

    // JE command
    if (strcmp(command, JE) == 0) {
        executeJE(label_offset);
    }

    // JNE command
    if (strcmp(command, JNE) == 0) {
        executeJNE(label_offset);
    }

    // JS command
    if (strcmp(command, JS) == 0) {
        executeJS(label_offset);
    }

    // JNS command
    if (strcmp(command, JNS) == 0) {
        executeJNS(label_offset);
    }

    // JG command
    if (strcmp(command, JG) == 0) {
        executeJG(label_offset);
    }

    // JGE command
    if (strcmp(command, JGE) == 0) {
        executeJGE(label_offset);
    }

    // JL command
    if (strcmp(command, JL) == 0) {
        executeJL(label_offset);
    }

    // JLE command
    if (strcmp(command, JLE) == 0) {
        executeJLE(label_offset);
    }
}

/*
 * Function to execute MOV instructions.
 */
void
executeMovInstructions(struct instruction_attr *instr_attr_ptr) {
    char *command = instr_attr_ptr->instruction;
    SIZE_TYPE *address[2];

    // MOV reg,reg command
    if (strcmp(command, MOV) == 0) {
        address[0] = &GPRS[instr_attr_ptr->base_register];
        address[1] = &GPRS[instr_attr_ptr->operand_register];
        executeMov(address[0], address[1]);
    }

    // MOVI command
    if (strcmp(command, MOVI) == 0) {
        address[0] = &GPRS[instr_attr_ptr->operand_register];
        executeMovI(instr_attr_ptr->const_or_label, address[0]);
    }
}

/*
 * Function to execute No Operand instructions.
 */
void
executeNoOperandInstructions(struct instruction_attr *instr_attr_ptr) {
    char *command = instr_attr_ptr->instruction;

    // RET instruction
    if (strcmp(command, RET) == 0) {
        executeRet();
    }
}

/*
 * Function to decode the instructions from the instruction memory and execute.
 */
void
decodeAndExecuteInstructions() {
   SIZE_TYPE binary_opcode = readFromMemory(PC, NUM_BYTES_IN_WORD);
   PC = PC + 4;
   int instr_count = 1;

   while (binary_opcode != 0) {
       struct instruction_attr instr_attr;
       printf("Instruction Count: %d\t Executing opcode: 0x%x", instr_count, binary_opcode);
       isSubtract = false;
       decodeInstructionFromBinary(binary_opcode, &instr_attr);

       printf("\t Assembly Instruction: %s\n", instr_attr.instruction);
       
       // Call functions to execute instructions based on instruction format.
       switch(instr_attr.format) {
           default:
               printf("ERROR: Unsupported instruction format.\n");
               exit(0);
               break;
           case LOAD_STORE:
               executeMemoryTypeInstructions(&instr_attr);
               break;
           case REG_REG:
           case REG_MEM:
           case MEM_REG:
               executeRTypeInstructions(&instr_attr);
               break;
           case IMM_REG:
           case IMM_MEM:
               executeITypeInstructions(&instr_attr);
               break;
    	   case STACK_REG:
               executeStackInstructions(&instr_attr);
               break;
           case MEM_DISPLAY:
               executeMemoryDisplayInstructions(&instr_attr);
               break;
           case CONTROL_LABEL:
               executeControlTransferInstructions(&instr_attr);
               break;
           case MOV_IMM_REG:
           case MOV_REG_REG:
               executeMovInstructions(&instr_attr);
               break;
           case NO_OPERAND:
               executeNoOperandInstructions(&instr_attr);
               break;
       }
       displayRegisters();
       PRINT_CHAR('=', 85);NEWLINE(1);
       PRINT_CHAR('=', 85); NEWLINE(2);
       // Read the next instruction and increment the PC
       binary_opcode = readFromMemory(PC, NUM_BYTES_IN_WORD);
       PC = PC + 4;
       instr_count++;
   }
}


//#############################################################################
//////////////////////// Parsing and Validation Section ///////////////////////
//#############################################################################

/*
 * Returns true if given reg name is a valid register name, else return false.
 */
bool
isValidRegister(char *reg) {
    return IsStringInStringArray(reg, valid_registers, NUM_VALID_REGISTERS);
}

/*
 * Print generic error.
 */
void 
printGenericAddressParsingFailedAndExit(char* arg) {
    printf("ERROR: Could not parse generic addressing mode %s", arg);
    exit(0);
}

/*
 * Function to get address calculated from (rb + ri)S. It returns
 * Rb + Ri * S
*/
long 
getRegisterSumAddress(char* arg, long S, struct instruction_attr *instr_attr_ptr) {
    char* input = arg;
    char reg[5];
    int i = 0;

    // Get value of R(b).
    while(*arg != '+' && *arg != '\0') {
        reg[i++] = *arg;
        ++arg;
    }
    reg[i] = '\0';
    if (!isValidRegister(reg)) {
        printGenericAddressParsingFailedAndExit(input);
    }
    int reg_b = strtol(&reg[1], NULL, 10);
    long R_B = GPRS[reg_b];

    // If only one register is there, consider it as R_I and
    // return R_I * S.
    if (*arg == '\0') {
        instr_attr_ptr->index_register = reg_b;
        instr_attr_ptr->base_register = 0;
        return R_B * S;
    }
    ++arg;
    i = 0;
    while(*arg != '\0') {
        reg[i++] = *arg;
        ++arg;
    }
    reg[i] = '\0';
    if (!isValidRegister(reg)) {
        printGenericAddressParsingFailedAndExit(input);
    }
    int reg_i = strtol(&reg[1], NULL, 10);

    // Set the instruction attributes
    instr_attr_ptr->base_register = reg_b;
    instr_attr_ptr->index_register = reg_i;
    return GPRS[reg_i] * S + R_B;
}

/*
 * Function to convert generic address D(Ra + Rb) S into memory address using
 * Memory = [ Register[b] + Register[i] * S + D ]
 */
long 
getAddressFromGenericAddressingMode(char* arg, struct instruction_attr *instr_attr_ptr) {
    char* input = arg;
    char temp[100];
    int i=0;

    // Get value of D.
    long D = 0;
    while(*arg != '\0' && *arg != '(') {
        temp[i++] = *arg;
        ++arg;
    }

    temp[i] = '\0';
    if (arg == '\0') { 
        printGenericAddressParsingFailedAndExit(input);
    }
    if (temp[0] != '\0') {
        D = getLongFromBaseTenOrHexString(&temp[0]);
        if (D == -1) {
            printGenericAddressParsingFailedAndExit(input);
        }
    }
    // D has 8 bits signed range -128 to 127. Here we compare with max unsigned range.
    if (D > 255) {
        printf("ERROR: Valid Offset for memory address is 8 bit signed value in range [-128, 127].\n");
        exit(0);
    }

    // Get value of (Rb + Ri * S).
    i = 0;
    long R_SUM = 0;
    ++arg;
    while(*arg != '\0' && *arg != ')') {
        temp[i++] = *arg;
        ++arg;
    }
    temp[i] = '\0';
    if (*arg == '\0') { 
        printGenericAddressParsingFailedAndExit(input);
    }
    ++arg;
    long S = 1;
    if (*arg != '\0') { 
         S = getLongFromBaseTenOrHexString(arg);
    }
    if (S != 1 && S != 2 && S != 4 && S != 8) {
        printf("ERROR: Invalid value passed for scale factor. Valid values are 1/2/4/8.\n");
        exit(0);
    }
    R_SUM = getRegisterSumAddress(temp, S, instr_attr_ptr);
    
    // Set offset and scale factor in instruction attribute with proper sign
    // extension for D
    D = (signed char) D;
    instr_attr_ptr->offset = D;
    instr_attr_ptr->scale = (int) S;

    return R_SUM + D;
}

/*
 * Parses given input string to compute memory address.
 * Supported formats are:
 * base_10_number, hexadecimal_number,
 * D (Ra + Rb) S
 *
 * Returns -1 if the input is not valid.
 */
int 
getValidMemoryAddress(char* arg, struct instruction_attr *instr_attr_ptr) {
    char *input = arg;
    if (getIndexOfFirstChar(input, '(') != -1) {
        return getAddressFromGenericAddressingMode(arg, instr_attr_ptr);
    } else {
        printf("ERROR: Incorrect argument passed. Expected a valid address/register.\n");
        exit(0);
    }
}

/*
 * Function to verify that the given memory address is not an invalid i.e
 * should not fall in bootstrap/instruction memory region.
 */
void
checkValidMemoryAccess(SIZE_TYPE memory_address) {
    if (memory_address >= MEMORY_SIZE || memory_address < INSTRUCTION_MEMORY_MAX) {
        printf("ERROR: Invalid Memory Address Access '%u'. The address falls in bootstrap/instruction memory range.\n", memory_address);
        exit(0);
    } 
}

/*
 * Function to calculate a constant's value in case of Immediate-Type instructions.
 */
SIZE_TYPE 
getConstant(char* arg) {
    if (!isStartsWith(arg, "$")) {
        printf("Invalid constant value '%s'. Constant value must start with $.\n",
                arg);
        exit(0);
    }
    return getLongFromBaseTenOrHexString(&arg[1]);
}

/*
 * Function to execute all memory type instructions after performing appropriate
 * validations on the arguments passed.
 */
void 
validateMemoryTypeInstruction(char* command, char* arg1, char* arg2) {
    // First argument should be a register only.
    if (!isValidRegister(arg1)) {
        printf("ERROR: arg1 should be a valid register.\n");
        exit(0);
    }

    // Second argument should be a valid memory address.
    struct instruction_attr instr_attr;
    SIZE_TYPE memory_address = getValidMemoryAddress(arg2, &instr_attr);
    if (memory_address == -1) {
        printf("ERROR: arg2 should be a valid memory address.\n");
        exit(0);
    }

    int reg = (int)strtol(&arg1[1], NULL, 10);

    // Set the remaining instruction attributes.
    instr_attr.format = LOAD_STORE;
    instr_attr.operand_register = reg;
    strcpy(instr_attr.instruction, command);
    
    // Get binary opcode for instruction and save in instruction memory.
    SIZE_TYPE binary_opcode = encodeInstructionToBinary(&instr_attr);
    saveInstructionToMemory(binary_opcode);
}

/*
 * Function to execute all R-type instructions after performing appropriate
 * validations on the arguments passed.
 */
void 
validateRTypeInstruction(char* command, char** args, int arg_count) {
    int i;
    struct instruction_attr instr_attr;

    // Variables to determine the instruction format as
    // reg-reg/reg-mem/mem-reg
    int reg_index = 0;
    int mem_index = 0;
    int reg_count = 0;
    int reg_array[2];

    for (i=0; i < arg_count; ++i) {
        if (isValidRegister(args[i])) {
            int reg = (int)strtol(&args[i][1], NULL, 10);
            reg_index = i;
            ++reg_count;
            reg_array[i] = reg;
        } else {
            SIZE_TYPE memory_address = getValidMemoryAddress(args[i], &instr_attr);
            mem_index = i;
        }
    }

    // Fill appropriate values for instruction attributes
    // Find the type of the instruction
    strcpy(instr_attr.instruction, command);
    if (reg_count == 2) {
        // Case of reg-reg format i.e. instr src_reg, dest_reg
        instr_attr.format = REG_REG;
        instr_attr.operand_register = reg_array[0];
        instr_attr.base_register = reg_array[1];
    } else if (reg_index < mem_index) {
        // Case of reg-mem format i.e. instr src_reg, dest_mem
        instr_attr.format = REG_MEM;
        instr_attr.operand_register = reg_array[0];
    } else {
        // Case of mem-reg format i.e. instr src_mem, reg_dest
        instr_attr.format = MEM_REG;
        instr_attr.operand_register = reg_array[1];
    }

    // Call function to encode the instruction to binary
    SIZE_TYPE binary_opcode = encodeInstructionToBinary(&instr_attr);
    saveInstructionToMemory(binary_opcode);
}

/*
 * Function to execute all stack instructions after performing appropriate
 * validations on the arguments passed.
 * The valid stack format is: PUSH/POP reg
 */
void 
validateStackInstruction(char* command, char* arg1) {
    struct instruction_attr instr_attr;
    int reg_index = 0;
    if (isValidRegister(arg1)) {
            reg_index = (int)strtol(&arg1[1], NULL, 10);
    } else {
        printf("ERROR: '%s' instruction needs a valid General Purpose register argument only. "
                "Invalid register argument passed '%s'.\n", command, arg1);
        exit(0);
    }

    // Fill appropriate values for instruction attributes
    // Find the type of the instruction
    strcpy(instr_attr.instruction, command);
    instr_attr.format = STACK_REG;
    instr_attr.operand_register = reg_index;

    // Call function to encode the instruction to binary
    SIZE_TYPE binary_opcode = encodeInstructionToBinary(&instr_attr);
    saveInstructionToMemory(binary_opcode);
}

/*
 * Function to execute all Immediate-type instructions after performing appropriate
 * validations on the arguments passed.
 */
void 
validateITypeInstruction(char* command, char* arg1, char* arg2) {
    SIZE_TYPE constant = getConstant(arg1);

    // Variable to determine instruction format as
    // imm-reg/imm-mem
    bool is_imm_reg = false;
    int reg_index = 0;
    struct instruction_attr instr_attr;
    
    if (isValidRegister(arg2)) {
        int reg = (int)strtol(&arg2[1], NULL, 10);
        is_imm_reg = true;
        reg_index = reg;
    } else {
        SIZE_TYPE memory_address = getValidMemoryAddress(arg2, &instr_attr);
    }

    // Set instruction attributes
    strcpy(instr_attr.instruction, command);
    instr_attr.const_or_label = (int) constant;

    if (is_imm_reg) {
        instr_attr.format = IMM_REG;
        instr_attr.operand_register = reg_index;    
    } else {
        instr_attr.format = IMM_MEM;
    }

    // Call function to encode the instruction to binary
    SIZE_TYPE binary_opcode = encodeInstructionToBinary(&instr_attr);
    saveInstructionToMemory(binary_opcode);
}

/*
 * Function to execute memory contents display instruction after validations on arguments.
 * Valid Format:
 *      mem const, reg
 */
void
validateMemoryDisplayInstruction(char *command, char* arg1, char* arg2) {
    // The first argument should be a constant and the other should be a GPR.
    SIZE_TYPE constant = getConstant(arg1);
    struct instruction_attr instr_attr;
   
    int reg_index;
    // Validate the second argument should be a GPR.
    if (isValidRegister(arg2)) {
        reg_index = (int)strtol(&arg2[1], NULL, 10);
    } else {
        printf("ERROR: Invalid register argument passed '%s'.\n", arg2);
        exit(0);
    }

    // Set instruction attributes
    strcpy(instr_attr.instruction, command);
    instr_attr.const_or_label = (int) constant;
    instr_attr.format = IMM_REG;
    instr_attr.operand_register = reg_index;    

    // Call function to encode the instruction to binary
    SIZE_TYPE binary_opcode = encodeInstructionToBinary(&instr_attr);
    saveInstructionToMemory(binary_opcode);
}

/*
 * Function to validate a control-transfer type instruction such as
 * jmp/call. Valid syntax is:
 *      jmp label
 *      call label
 *      je label
 */
void
validateControlTransferInstruction(int instr_number, char *command, char *label_arg) {
    struct instruction_attr instr_attr;
   
    // Validate the argument should be a valid label.
    int label_index = getLabelIndex(label_arg);
    if (label_index == -1) {
        printf("ERROR: Invalid label '%s' passed; does not match with any provided labels.\n", label_arg);
        exit(0);
    }

    // Set instruction attributes
    strcpy(instr_attr.instruction, command);
    instr_attr.const_or_label = (int) (LABELS[label_index].position - instr_number - 1);
    instr_attr.format = CONTROL_LABEL;

    // Call function to encode the instruction to binary
    SIZE_TYPE binary_opcode = encodeInstructionToBinary(&instr_attr);
    saveInstructionToMemory(binary_opcode);
}

/*
 * Function to validate Mov type instructions.
 * Valid syntax:
 *      MOV reg, reg
 *      MOVI constant, reg
 */
void
validateMovTypeInstruction(char *command, char *arg1, char *arg2) {
    // The first argument should be a constant or a GPR.
    bool is_mov_imm = false;
    SIZE_TYPE constant = 0;
    int src_reg_index;
    int dest_reg_index;

    if (isStartsWith(arg1, "$")) {
        is_mov_imm = true;
        constant = getConstant(arg1);
    } else {
        if (isValidRegister(arg2)) {
            src_reg_index = (int)strtol(&arg1[1], NULL, 10);
        } else {
            printf("ERROR: Invalid register argument passed '%s'.\n", arg2);
            exit(0);
        }   
    }
    
    struct instruction_attr instr_attr;
    
    // Validate the second argument should be a GPR.
    if (isValidRegister(arg2)) {
        dest_reg_index = (int)strtol(&arg2[1], NULL, 10);
    } else {
        printf("ERROR: Invalid register argument passed '%s'.\n", arg2);
        exit(0);
    }

    // Set instruction attributes
    strcpy(instr_attr.instruction, command);
    instr_attr.operand_register = dest_reg_index;    
    if (is_mov_imm) {
        instr_attr.const_or_label = (int) constant;
        instr_attr.format = MOV_IMM_REG;
    } else {
        instr_attr.base_register = src_reg_index;
        instr_attr.format = MOV_REG_REG;
    }
    
    // Call function to encode the instruction to binary
    SIZE_TYPE binary_opcode = encodeInstructionToBinary(&instr_attr);
    saveInstructionToMemory(binary_opcode);
}

/*
 * Function to validate No Operand instructions.
 */
void
validateNoOperandInstruction(char *command) {
    struct instruction_attr instr_attr;
    strcpy(instr_attr.instruction, command);
    instr_attr.format = NO_OPERAND;

    SIZE_TYPE binary_opcode = encodeInstructionToBinary(&instr_attr);
    saveInstructionToMemory(binary_opcode);
}

/*
 * Generic function to call functions specific to validate, encode and save 
 * different instruction types.
 */
void 
validateEncodeAndSaveInstruction(int instr_number, char* command, char** args, int arg_count) {
    removeWhiteSpaces(args, arg_count);
    // Memory-Type instructions
    if (IsStringInStringArray(command, MEM_INSTR, NUM_VALID_MEM_INSTR)) {
        if (arg_count != 2) {
            printf("ERROR: %s should have 2 arguments.\n", command);
            exit(0);
        }
        validateMemoryTypeInstruction(command, args[0], args[1]);
    } 
    // R-Type instructions
    else if (IsStringInStringArray(command, R_INSTR, NUM_VALID_R_INSTR)) {
         if (arg_count < 1 || arg_count > 3 ) {
            printf("ERROR: %s should have 1-3 arguments.\n", command);
            exit(0);
        }
        validateRTypeInstruction(command, args, arg_count); 
    }
    // Immediate-Type instructions
    else if (IsStringInStringArray(command, I_INSTR, NUM_VALID_I_INSTR)) {
        if (arg_count != 2) {
            printf("ERROR: %s should have 2 arguments.\n", command);
            exit(0);
        }
        validateITypeInstruction(command, args[0], args[1]);
    }
    // Stack instructions
    else if (IsStringInStringArray(command, STACK_INSTR, NUM_VALID_STACK_INSTR)) {
        if (arg_count !=1) {
            printf("ERROR: %s should have only 1 argument i.e. a register.\n", command);
            exit(0);
        }
        validateStackInstruction(command, args[0]);
    }
    // Memory Display(special) instruction
    else if (IsStringInStringArray(command, MEM_DISPLAY_INSTR, NUM_VALID_MEM_DISPLAY_INSTR)) {
        if (arg_count != 2) {
            printf("ERROR: %s should have 2 arguments.\n", command);
            exit(0);
        }
        validateMemoryDisplayInstruction(command, args[0], args[1]);
    }
    // Control transfer type instructions e.g. jmp, call
    else if (IsStringInStringArray(command, CONTROL_INSTR, NUM_VALID_CONTROL_INSTR)) {
        if (arg_count != 1) {
            printf("ERROR: %s should have only 1 argument i.e. label.\n", command);
            exit(0);
        }
        validateControlTransferInstruction(instr_number, command, args[0]);
    }
    // Mov data instructions
    else if (IsStringInStringArray(command, MOV_INSTR, NUM_VALID_MOV_INSTR)) {
        if (arg_count != 2) {
            printf("ERROR: %s should have 2 arguments.\n", command);
            exit(0);
        }
        validateMovTypeInstruction(command, args[0], args[1]);
    }
    // No operand instructions
    else if (IsStringInStringArray(command, NO_OPERAND_INSTR, NUM_VALID_NO_OPERAND_INSTR)) {
        if (arg_count != 0) {
            printf("ERROR: %s should have no arguments.\n", command);
            exit(0);
        }
        validateNoOperandInstruction(command);
    }
}

/*
 * Function to initialize the general purpose registers with some values.
 */
void
initializeRegistersAndMemory() {
    // Set the initial values for PC and instruction memory to instruction memory min value i.e. 1024
    PC = INSTRUCTION_MEMORY_MIN;
    INSTR_MEMORY_PTR = INSTRUCTION_MEMORY_MIN;
    SP = STACK_MEMORY_START-1;
    FP = STACK_MEMORY_START-1;
    
    // Set some initial values to register and memory
    GPRS[0] = 0x0;
    GPRS[1] = 0x4567;
    GPRS[2] = 0x66;
    GPRS[3] = 0x8234;
    GPRS[5] = 9400;
    GPRS[6] = 0x2;
    GPRS[7] = 0x3;

    writeIntoMemory(4096, NUM_BYTES_IN_WORD, (data_ptr) &GPRS[1]);
    writeIntoMemory(4100, NUM_BYTES_IN_WORD, (data_ptr) &GPRS[2]);
    loadRegister(&GPRS[4], 4100);

    // Display CPU information and Register contents
    printf("\n---------------------CPU Architecture Information-----------------\n");
    printf("Number of General Purpose Registers: %d\n", MAX_GPRS);
    printf("Word Size: %d\n", WORD_SIZE);
    printf("Bootstrap memory reserved: 1 KB  Range: 0 - %d\n", BOOTSTRAP_MEMORY_MAX);
    printf("Instruction memory reserved: 8 KB  Range: %d - %d\n", INSTRUCTION_MEMORY_MIN, INSTRUCTION_MEMORY_MAX);
    printf("Data memory reserved: 8 KB  Range: %d - %d\n", DATA_MEMORY_MIN, DATA_MEMORY_MAX);
    printf("Stack memory start location: %d\n", STACK_MEMORY_START-1);
    printf("Byte/Memory Addressing: Little Endian\n");
    printf("------------------------------------------------------------------\n\n");
    
    displayRegisters();
}

/*
 * Main function to start application.
*/
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Correct usage is <binary_name> <file_name>");
    }
    initializeRegistersAndMemory();

    // Read and execute assembly instructions from input file.
    char *input = NULL;
    size_t len = 0;
    ssize_t read;
    int instr_count = 1;
    
    // Read instructions file.
    FILE *fp;

    // First scan of the program for finding out positions of the labels.
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("ERROR: File not available to read. \n");
        exit(EXIT_FAILURE);
    }
    
    int instruction_position = 0;
    while((read = getline(&input, &len, fp)) != -1) {
        if (strlen(input) < 3) {
            continue;
        }

        int colon_index = getIndexOfFirstChar(input, ':');
        int space_index = getIndexOfFirstChar(input, ' ');
        if (colon_index != -1) {
            if (space_index != -1 && space_index < colon_index) {
                continue;
            }
            char *label = (char*) malloc(sizeof(char) * (colon_index + 1));
            snprintf(label, colon_index+1, "%s", input);
            if (storeLabelInformation(label, instruction_position) == -1) {
                printf("Label '%s' defined multiple times.\n", label);
                exit(0);
            }
            free(label);
        }
        ++instruction_position;
    }
    rewind(fp);

    PRINT_CHAR('=', 85); NEWLINE(1);
    PRINT_CHAR('=', 85); NEWLINE(1);
    printf("VALIDATING and DECODING INSTRUCTIONS\n");

    while ((read = getline(&input, &len, fp)) != -1) {
        if (strlen(input) < 3) {
            continue;
        }

        // Remove the label(if any) from the instruction
        int colon_index = getIndexOfFirstChar(input, ':');
        input = &input[colon_index + 1];

        // Remove any leading space from the input
        while (isspace ((unsigned char) *input)) {
            input++;
        }

        NEWLINE(1);
        printf("Instruction %d: %s", instr_count++, input);

        // Parse command name.
        int index_of_first_space = getIndexOfFirstChar(input, ' ');

        char command[10];
        snprintf(command, index_of_first_space + 1, "%s", input);

        // This is special case of no operand instruction.
        if (index_of_first_space == -1) {
            snprintf(command, strlen(input) , "%s", input);
        }

        if (!IsStringInStringArray(command, valid_instructions, 
                    NUM_VALID_OPCODES)) {
          printf("ERROR: Assembly Command '%s' not supported.\n", command);
          exit(0);
        }
 
        // Parse out the arguments to command.
        input = &input[index_of_first_space + 1];

        char *args[10];
        int arg_count = 0;
        char *p = strtok (input, ",");
        while (p != NULL) {
            args[arg_count++] = p;
            p = strtok (NULL, ",");
        }

        if (index_of_first_space == -1) {
            arg_count = 0;
        }
        validateEncodeAndSaveInstruction(instr_count - 2, command, args, arg_count);
    }

    close(fp);
    
    NEWLINE(1);
    PRINT_CHAR('=', 85); NEWLINE(1);
    PRINT_CHAR('=', 85); NEWLINE(1);
    
    printf("EXECUTING INSTRUCTIONS\n\n");
    // Decode the binary opcodes and execute the instructions.
    decodeAndExecuteInstructions();

    return 0;
}
