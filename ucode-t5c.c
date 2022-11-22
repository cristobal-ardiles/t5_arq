#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Documentation
// -------------
//
// This program generates the ucode of LV32IM. This ucode contains the
// u-program that interpretes a Risc-V instruction set.  It is stored in a ROM
// inside the control unit module and stores 128 u-instructions with
// the control signals to execute the gcc generated instructions for a
// Risc-V of 32 bits with mul/div extension.
// 
// Each u-instruction of the ucode ROM is 32 bit wide and may combine
// multiple actions. Some actions require multiple bits.
// All these actions are constants with prefix CS.
//
// The ROM is filled in the main function of this file by calling the sig
// function.  The sig function receives one int argument with 32 bits
// of control signal for one word in the ROM.  Control signals are formed
// by the or of several actions.  For example the 2 words required for the
// jalr instruction are:
//
//  defInst("jalr", 0x67); // jump and link register
//  sig(CSXPC | CSY4 | CSWRRD);          Rd <- PC + 4
//  sig(CSITYPE | CSJAL | CSGOFETCH);    PC <- Rs1 + I, goto fetch
//
//  The defInst function states that the opcode for jalr is 0x67.
//
//  These are the control signals that can be specified:

// PC Update choice
// ------------------------------
// Choose one of the following
//
// default: don't change PC
// CSWRPC : PC <- PC+4
// CSWRPC | CSBRANCH : PC <- Taken ? PC + constant value in B format : PC+4
// CSWRPC | CSRS1 | CSJALR : PC <- Rs1 + constant value in I format
// CSWRPC | CSJAL          : PC <- PC + constant value in J format
// CSWRPC | CSBOOT         : PC <- address of bootstrap instruction

// Selection of x operand for ALU
// ------------------------------
// Choose one of the following
//
// default: register specified by the rs1 field
// CSAX: PC register
// CSAX | CSX0: constant 0

// Selection of y operand for ALU
// ------------------------------
// Choose one of the following
//
// default: register specified by the rs2 field
// CSAY | CSUTYPE: constant value in U format
// CSAY | CSSTYPE: constant value in S format
// CSAY | CSITYPE: constant value in I format
// CSAY | CSY0:    constant 0
// CSAY | CSY4:    constant 4

// Selection of operation computed by the ALU
// ------------------------------------------
// Choose one of the following
//
// default: add
// CSALUFUNC3_7: choose alu operation from func3 and func7 field of the
//               Risc-V instruction

// Selection of u-jump destination
// -------------------------------
//
// default  : the current u-address + 1
// CSGOFETCH: u-address of fetch cycle
// CSDECODE : decode Risc-V instruction

// Other useful control commands
// -----------------------------
// CSWR: Write to memory
// CSRD: Read from memory
// CSWRRD: write register specified in rd field
// CSRD | CSWRIR: Update Inst register with value read from memory (Ld)
// CSLSFUNC3: when reading or storing memory, choose conversion from
//            func3 field in the Risc-V instruction

// Implementation
// --------------

// Control signals are the output of the control unit submodule
// inside the control/decode unit module and are needed to generate its
// outputs: AVY AVX AluOp BusOp CtlSig BCond d s1 s2
//
// name  bits   description
// ax       0  Alternate X, if active the alu x operand is the AX output
//             of ctl/dec unit (usually pc).  Otherwise, it is register
//             specified by the rs1 field of the Risc-V instruction.
// x0       1  If active the AVX output of ctl/dec unit is 0.  Otherwise
//             it is the pc register.
// ay       2  Alternate Y, if active the alu y operand is the AY output
//             of ctl/dec unit (a constant value).  Otherwise, it is the
//             register specified by the rs2 field in the Risc-V instruction.
// type   5-3  the type of the AVY output of ctr/dec unit.  It depends on the
//             format of the instruction: I-type, U-type, S-type, 0 or 4
// alufunc3_7  If active choose the ALU operation from the func3 and func7
//          6  field of the Risc-V instruction.  Otherwise add is chosen.
// wrrd     7  If active, writes the register specified in the d field of
//             the Risc-V instruction.
// rd       8  If active, read from memory.
// wr       9  If active, write to memory.
// lsfunc3 10  If active, the data is converted depending on the value of
//             the func3 field of the Risc-V instruction.
// wrir    11  If active, writes the Inst register with the value read
//             from memory.
// wrpc    12  If active, writes the Pc register with the value read
//             from memory if read is asserted, or the alu output if not.
// branch  13  If active, chooses the Pc input as Pc+4 if taken (from
//             branch unit) is false, or the value read from memory if
//             rd is active, or the alu output if not.
// rs1     14  If active, chooses the Pc input from Rs1
// dec     15  If active, the u-address of the next u-instruction is the
//             the opcode field of the Risc-V instruction.
// gofetch 16  If active, a u-jump is performed to the fetch cycle
//             If dec and gofetch are low, the u-pc is incremented in 1.
// ecall   17  Active when executing an unimplemented instruction.
//             May be useful for debugging.
// error   18  Active when executing an invalid instruction
// pcsel 20-19 Select value of offset: taken ? B : 4, I, J or boot address

// Single bit control signals

#define CSAX         (1<<0)
#define CSX0         (1<<1)
#define CSAY         (1<<2)
#define CSALUFUNC3_7 (1<<6)
#define CSWRRD       (1<<7)
#define CSRD         (1<<8)
#define CSWR         (1<<9)
#define CSLSFUNC3    (1<<10)
#define CSWRIR       (1<<11)
#define CSWRPC       (1<<12)
#define CSBRANCH     (1<<13)
#define CSRS1        (1<<14)
#define CSDECODE     (1<<15)
#define CSGOFETCH    (1<<16)

// DUMMY for debugging purposes
#define CSECALL      (1<<17)
#define CSERROR      (1<<18)

// Type field

#define CSITYPE      (0<<3)
#define CSSTYPE      (1<<3)
#define CSUTYPE      (2<<3)
#define CSY0         (3<<3)
#define CSY4         (4<<3)
#define CSRPCTYPE    (5<<3)

#define CSJALR       (1<<19)
#define CSJAL        (2<<19)
#define CSBOOT       (3<<19)

#define TYPEMASK     (7<<3)
#define PCSELMASK    (3<<19)

#define SIZEROM 128

// A uInst is a u-instruction in the ucode ROM
typedef unsigned int uInst;

// A uAddress is an address of a u-instruction in the ucode ROM
typedef int uAddress;

void dumpUcode();

uInst ucode[SIZEROM];        // The microcode ROM content
int endflag= 1;

char *opnames[SIZEROM];      // Names of Risc-V instructions
#define OPCODEBEGIN 0
#define OPCODEEND (SIZEROM-1)

uAddress upc= 0;    // The u-program counter, u-address of next u-instruction

// Defines the Risc-V instruction opname
void defInst(char *opname, int opcode) {
  if (opcode>OPCODEEND || opcode<3) {
    fprintf(stderr, "Opcode error for %s\n", opname);
    exit(1);
  }
  if (opnames[opcode]!=NULL) {
    fprintf(stderr, "Opcode for %s already used for %s\n",
                    opname, opnames[opcode]);
    exit(1);
  }
  upc= opcode;
  opnames[opcode]= opname;
}

// Add a new u-instruction at address upc.  The upc is incremented by 1.
// Parameter sig is a bit mask with control signals (bits 31-7)
void sig(uInst sig) {
  if (upc>=SIZEROM) {
    fprintf(stderr, "uROM size overflow\n");
    exit(1);
  }

  if (ucode[upc]!=0) {
    fprintf(stderr, "This u-instruction is already filled\n");
    exit(2);
  }

  ucode[upc++]= sig;
}

void cleanUcode() {
  upc= 0;
  while (upc<SIZEROM) {
    int sig= ucode[upc];
    if (sig!=0) {
     
      if (upc==SIZEROM-1 || ucode[upc+1]==0 || opnames[upc+1]!=NULL) {
        if (!(sig&CSGOFETCH) && !(sig&CSDECODE)) {
          fprintf(stderr, "u-instruction at %x doesn't finish with gofetch\n",
                          (unsigned)upc);
        }
      }
      if ((sig&CSRD) && !(sig&(CSWRRD|CSWRPC|CSWRIR))) {
        fprintf(stderr, "read operation at %x doesn't write to any register\n",
                        (unsigned)upc);
      }
    }
    upc++;
  }
}

int main(int argc, char *argv[]) {
  // First u-instruction with u-address 0 (upc is 0).  This is the
  // bootstrap, i.e. the first u-instruction to be executed.

  sig(CSWRPC | CSBOOT);
  // PC is zero at bootstrap
  // PC <- boot address + PC (content of fixed RAM address 0xfff8)

  // Fetch Cycle with u-address 1.  All instructions finish jumping to this
  // u-instruction.
  sig(CSAX | CSAY | CSY0 | CSRD | CSWRIR);
  // INST <- M[PC]   This is the only place where IR is written.

  // Decode cycle with u-address 2.
  sig(CSDECODE);
  // The opcode in the Risc-V instruction is chosen as u-address of the
  // next u-instruction to be executed.  So the execution will continue
  // at u-address 3 only when the instruction is a load from memory,
  // because its opcode is 3.

  // Logic and arithmetic instructions

  defInst("reg_alu_inst", 0x33);
  sig(CSALUFUNC3_7 | CSWRRD | CSWRPC | CSGOFETCH);
  // Rd <- Rs1 func3 y func7 Rs2, PC <- PC+4, goto fetch cycle
  // The ALU operation is encoded in func3
  // For sub instruccion, bit 5 in func7 is asserted

  defInst("imm_alu_inst", 0x13);
  sig(CSAY | CSITYPE | CSALUFUNC3_7 | CSWRRD | CSWRPC | CSGOFETCH);
  // Rd <- Rs1 func3 I, PC <- PC+4, goto fetch cycle
  // The ALU operation is encoded in func3 only (doesn't use func7)

  // Memory access

  defInst("load_inst", 0x03);
  sig(CSAY | CSITYPE | CSLSFUNC3 | CSRD | CSWRRD | CSWRPC | CSGOFETCH);
  // Rd <- M[Rs1 + I], PC <- PC+4, goto fetch cycle

  defInst("store_inst", 0x23);
  sig(CSAY | CSSTYPE | CSLSFUNC3 | CSWR | CSWRPC | CSGOFETCH);
  // M[Rs1 + S] <- Rs2, PC <- PC+4, goto fetch cycle

  // Special instructions

  defInst("lui", 0x37); // load upper immediate
  sig(CSAX | CSX0 | CSAY | CSUTYPE | CSWRRD | CSWRPC | CSGOFETCH);
  // Rd <- U, PC <- PC+4, goto fetch cycle

  defInst("auipc", 0x17); // add upper immediate to pc
  sig(CSAX | CSAY | CSUTYPE | CSWRRD | CSWRPC | CSGOFETCH);
  // Rd <- PC + U, PC <- PC+4, goto fetch cycle

  // Call function and return

  defInst("jal", 0x6f); // jump and link
  sig(CSAX | CSAY | CSY4 | CSWRRD | CSWRPC | CSJAL | CSGOFETCH);
  // Rd <- PC + 4, PC <- PC + J, goto fetch cycle

  defInst("jalr", 0x67); // jump and link register
  sig(CSAX | CSAY | CSY4 | CSWRRD | CSWRPC | CSRS1 | CSJALR | CSGOFETCH);
  // Rd <- PC + 4, PC <- Rs1 + I, goto fetch

  defInst("branch", 0x63);
  sig(CSBRANCH | CSWRPC | CSGOFETCH);
  // PC <- taken ? PC + B : PC + 4, goto fetch cycle
  
  defInst("ecall", 0x73);                   // not implemented
  sig(CSECALL | CSWRPC | CSGOFETCH);
  // PC <- PC+4, goto fetch cycle

  defInst("fence", 0x0f);                   // not implemented
  sig(CSECALL | CSWRPC | CSGOFETCH);
  // PC <- PC+4, goto fetch cycle

  // Interrupt handling
  defInst("enable", 0x40);  // 100 0000
  sig(CSECALL | CSWRPC | CSGOFETCH);
  // enable interrupts in control unit, PC <- PC+4, goto fetch cycle

  defInst("disable", 0x41); // 100 0001
  sig(CSECALL | CSWRPC | CSGOFETCH);
  // disable interrupts in control unit, PC <- PC+4, goto fetch cycle

  printf("return from interrupt instruction is 0x%x\n",
         0x42 + (2U<<7) + (2U<<15) + (16U<<20));
  defInst("mret", 0x42); // 100 0010
  sig(CSERROR | CSWRPC | CSGOFETCH);

  defInst("intr", 0x45); // 100 0101
  sig(CSERROR | CSWRPC | CSGOFETCH);

  // =====================================
  // Caution: do not modify before this line
  // =====================================

  // Add here the experimental instructions

  // defInst("blabla", opcode);
  // sig( ... );
  // ...
  // sig( ... );

  defInst("swap", 0x9); // Restriction: all 3 registers must be different
  // Rd <- M[Rs1]
  sig(CSY0 | CSAY | CSLSFUNC3 | CSRD | CSWRRD);
  // M[Rs1] <- Rs2, PC<-PC+4,
  sig(CSY0 | CSAY | CSLSFUNC3 | CSWR | CSWRPC | CSGOFETCH);
                                                 // goto fetch cycle
  // Exercise: what should change for swaph and swapb?

  // swrpc s1, s0, 0x70(pc) # M[pc+0x70] <- s0, s1 <- pc+0x0
  // Agregue aca el codigo para agregar la instruccion swrpc
  // se puede hacer en un ciclo del reloj, pero puede hacerlo en 2 ciclos
  defInst("swrpc",0xa);
  sig(CSAY | CSAX | CSWR | )
  // =====================================
  // Caution: do not modify after this line
  // =====================================

  cleanUcode();

  dumpUcode();

  // Write opcodes
  int len= strlen(argv[0]);
  char romname[len+5];
  sprintf(romname,"%s.rom", argv[0]);
  FILE *romfile= fopen(romname, "w");
  if (romfile==NULL) {
    perror("ucode.rom");
    exit(1);
  }

  fprintf(romfile,"v2.0 raw\n");
  for (int i= 0; i<SIZEROM; i++) {
    fprintf(romfile,"%x\n", ucode[i]);
  }
  fclose(romfile);

  return 0;
}

void prtCs(char *cs) {
  printf(" %s", cs);
}

void dumpUcode() {
  printf("Content of ucode.rom.  Each line corresponds to a u-instruction.\n");
  printf("The fields in the listing are:\n");
  printf("  <u-address>:\n");
  printf("  <u-instruction content>\n");
  printf("  <control signals>\n");
  printf("(all numbers are in hexadecimal notation)\n\n");
  printf("Bootstrap:\n");

  for (uAddress i= 0; i< SIZEROM; i++) {
    uInst sig= ucode[i];
    if (sig==0) {
      ucode[i] |= CSERROR | CSWRPC | CSGOFETCH;
      continue;
    }

    if (opnames[i]!=NULL)
      printf("Inst: %s\n", opnames[i]);

    printf("%02x: %08x ", i, sig); 

    if (sig&CSAX)
      prtCs( ((sig&CSX0)==CSX0) ? "ax x0" : "ax");

    if (sig&CSAY) {
      prtCs("ay");
      int type= sig&TYPEMASK;
      switch (type) {
        case CSUTYPE: prtCs("u-type"); break;
        case CSSTYPE: prtCs("s-type"); break;
        case CSITYPE: prtCs("i-type"); break;
        case CSY0: prtCs("y0"); break;
        case CSY4: prtCs("y4"); break;
        case CSRPCTYPE: prtCs("rpc-type"); break;
        default: fprintf(stderr, "Invalid type %d\n", type); exit(1);
      }
    }

    if (sig&CSWRPC) prtCs("wrpc");
    if (CSRS1 & sig) prtCs("rs1");
    if ((sig&CSJALR)==CSJALR) prtCs("jalr");
    if ((sig&CSJAL)==CSJAL) prtCs("jal");
    if ((sig&CSBOOT)==CSBOOT) prtCs("boot");

    if (CSALUFUNC3_7 & sig) prtCs("alufunc3_7");
    if (CSWRRD & sig) prtCs("wrrd");

    if (CSWR & sig) prtCs("wr");
    if (CSRD & sig) prtCs("rd");
    if (sig&CSLSFUNC3) prtCs("lsfunc3");

    if (CSWRIR & sig) prtCs("wrir");

    if (sig&CSBRANCH) prtCs("branch");

    if (CSDECODE & sig) prtCs("decode");
    if (CSGOFETCH & sig) prtCs("gofetch");

    if (CSECALL & sig) prtCs("ecall");
    if (CSERROR & sig) prtCs("error");

    printf("\n");
  }
}
