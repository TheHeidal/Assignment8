#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int           reg[8];
unsigned char mem[10000*4];
int           pc = -1;
unsigned char insOpCode;
unsigned char insOp0;
unsigned char insOp1;
unsigned char insOp2;
char          insOpImm;
int           insOpExt;

void fetch() {
    // printf("\n%x", mem[pc]);
    insOpCode = mem [pc] >> 4;
    insOp0    = mem [pc] & 0xf;
    insOp1    = mem [pc+1] >> 4;
    insOp2    = mem [pc+1] & 0xf;
    insOpImm  = mem [pc+1];
    pc += 2;
    switch (insOpCode) {
        case 0x0:
        case 0xb:
            insOpExt = mem [pc] << 24 | mem [pc+1] << 16 | mem [pc+2] << 8 | mem [pc+3];
            pc += 4;
            break;
        default:
            ;
    }
}


int exec() {
    int cont = 1;
    int addr, val;
    
    void* jumptableinsOpCode[] =  {&&C0, &&C1, &&C2, &&C3, &&C4, &&DEFAULTOPCODE, &&C6,
                                    &&C7, &&C8, &&C9, &&CA, &&CB, &&CC, &&CD, &&CE, &&CF};
    void* jumptableinsOp0[] =     {&&C60, &&C61, &&C62, &&C63, &&C64, &&C65, &&C66,
                                    &&C67, &&C6F, &&DEFAULTOP0};

    if (insOpCode == 5 | insOpCode > 15 | insOpCode < 0) {
        goto *jumptableinsOpCode[5]; // default op code
    } else {
        goto *jumptableinsOpCode[insOpCode];
    }

    printf("InsOp %d error", insOpCode); // should never reach this line

    C0: // ld $i, d .............. 0d-- iiii iiii
        printf("\nC0");
        reg [insOp0] = insOpExt;
        goto CONT;

    C1: // ld o(rs), rd .......... 1osd
        printf("\n C1");
        addr = (insOp0 << 2) + reg [insOp1];
        reg [insOp2] = mem [addr] << 24 | mem [addr+1] << 16 | mem [addr+2] << 8 | mem [addr+3];
        goto CONT;

    C2: // ld (rs, ri, 2), rd .... 2sid
        printf("\nC2");
        addr = reg [insOp0] + (reg [insOp1] << 2);
        reg [insOp2] = mem [addr] << 24 | mem [addr+1] << 16 | mem [addr+2] << 8 | mem [addr+3];
        goto CONT;
            
    C3: // st rs, o(rd) .......... 3sod
        printf("\nC3");
        addr = (insOp1 << 2) + reg [insOp2];
        val  = reg [insOp0];
        mem [addr]   = val >> 24 & 0xff;
        mem [addr+1] = val >> 16 & 0xff;
        mem [addr+2] = val >>  8 & 0xff;
        mem [addr+3] = val       & 0xff;
        goto CONT;

    C4: // st rs, (rd, ri, 4) .... 4sdi
        printf("\nC4");
        addr = reg [insOp1] + (reg [insOp2] << 2);
        val  = reg [insOp0];
        mem [addr]   = val >> 24 & 0xff;
        mem [addr+1] = val >> 16 & 0xff;
        mem [addr+2] = val >>  8 & 0xff;
        mem [addr+3] = val       & 0xff;
        goto CONT;
            
    C6: // ALU ................... 6-sd
        printf("\nC6");
        if (insOp0 == 0xf) {
            goto *jumptableinsOp0[8];
        } else if (insOp0 > 7 | insOp0 < 0) {
            goto *jumptableinsOp0[9]; // default op
        } else {
            goto *jumptableinsOp0[insOp0];
        }

    printf("InsOp0 %d error", insOp0); // should never reach this line

    C60: // mov rs, rd ........ 60sd
        printf("\nC60");
        reg [insOp2] = reg [insOp1];
        goto CONT;

    C61: // add rs, rd ........ 61sd
        printf("\nC61");
        reg [insOp2] = reg [insOp1] + reg [insOp2];
        goto CONT;

    C62: // and rs, rd ........ 62sd
        printf("\nC62");
        reg [insOp2] = reg [insOp1] & reg [insOp2];
        goto CONT;

    C63: // inc rr ............ 63-r
        printf("\nC63");
        reg [insOp2] = reg [insOp2] + 1;
        goto CONT;

    C64: // inca rr ........... 64-r
        printf("\nC64");
        reg [insOp2] = reg [insOp2] + 4;
        goto CONT;

    C65: // dec rr ............ 65-r
        printf("\n C65");
        reg [insOp2] = reg [insOp2] - 1;
        goto CONT;

    C66: // deca rr ........... 66-r
        printf("\n C66");
        reg [insOp2] = reg [insOp2] -4;
        goto CONT;

    C67: // not ............... 67-r
        printf("\n C67");
        reg [insOp2] = ~ reg [insOp2];
        goto CONT;

    C6F: // gpc ............... 6f-r
        printf("\n C6F");
        reg [insOp2] = pc + (insOp1 << 1);
        goto CONT;

    DEFAULTOP0:
        printf("\n Default Op 0");
        printf ("Illegal ALU instruction: pc=0x%x fun=0x%x\n", pc, insOp0); // DEFAULT 1
        goto CONT;

    C7: // sh? $i,rd ............. 7dii
        printf("\n C7");
        if (insOpImm > 0)
        reg [insOp0] = reg [insOp0] << insOpImm;
        else
        reg [insOp0] = reg [insOp0] >> -insOpImm;
        goto CONT;

    C8: // br o .................. 8-oo
        printf("\n C8");
        pc += insOpImm << 1;
        goto CONT;

    C9: // beq rs, o ............. 9roo
        printf("\n C9");
        if (reg [insOp0] == 0)
        pc += insOpImm << 1;
        goto CONT;
        
    CA: // bgt rs, o .............. aroo
        printf("\n CA");
        if (reg [insOp0] > 0)
        pc += insOpImm << 1;
        goto CONT;

    CB: // j i ................... b--- iiii iiii
        printf("\n CB");
        pc = insOpExt;
        goto CONT;

    CC: // j o(rr) ............... croo
        printf("\n CC");
        pc = (((unsigned short) insOpImm) << 1) + reg [insOp0];
        goto CONT;

    CD: // j *o(rr) .............. droo
        printf("\n CD");
        addr = (((unsigned short) insOpImm) << 2) + reg [insOp0];
        pc = mem [addr] << 24 | mem [addr+1] << 16 | mem [addr+2] << 8 | mem [addr+3];
        goto CONT;

    CE: // j*(rr,ri,4) ............. eri-
        // TODO
        printf("\n CE");
        addr = ((reg[(unsigned short) insOp1]) << 2) + reg [insOp0];
        pc = mem [addr] << 24 | mem [addr+1] << 16 | mem [addr+2] << 8 | mem [addr+3];
        goto CONT;

    CF: // halt or nop ............. f?--
    printf("\n CF");
        if (insOp0 == 0) {
            cont = 0;
        }
        goto CONT;

    DEFAULTOPCODE:
        printf ("Illegal  instruction: pc=0x%x opCode=0x%x\n", pc, insOpCode);

    CONT:
        return cont;
}

int loadFile (char* filename) {
  char*  line = NULL;
  size_t len  = 0;
  FILE* f = fopen (filename, "r");
  if (f == NULL)
    return 0;
  while (getline (&line, &len, f) > 0) {
    char* sep = line;
    while (*sep != ':')
      sep++;
    *sep = 0;
    int addr = strtol (line, 0, 16);
    if (pc < 0)
      pc = addr;
    line = sep + 1;
    char v[3] = {0,0,0};
    while (1) {
      while (*line && *line == ' ')
        line ++;
      if (*line == 0 || *(line+1) == 0)
        break;
      v[0] = line[0];
      v[1] = line[1];
      mem [addr] = strtol (v, 0, 16);
      line += 2;
      addr += 1;
    }
    line = NULL;
  }
  fclose (f);
  return 1;
}

void showMem (int start, int count) {
  int i;
  for (i=0; i<count*4; i+=4)
    printf ("0x%08x: %02x %02x %02x %02x\n", start+i, mem [start+i], mem [start+i+1], mem [start+i+2], mem [start+i+3]);
}

void compute() {
  int cont = 0;
  do {
    fetch();
    cont = exec();
  } while (cont);
}

struct showMemList {
  int  len;
  int* addr;
  int* count;
} showMemList;

int showReg;

int parseArgs (int argc, char** argv) {
  if (argc < 2)
    return 0;
  for (int i=1; i<argc; i++)
    if (argv[i][0] == '-') {
      char* op = argv[i] + 1;
      if (strcmp (op, "p") == 0) {
        i += 1;
        pc = strtol (argv[i], 0, 16);
      } else if (strcmp (op, "m") == 0) {
        i += 1;
        char* sep = argv[i];
        while (*sep !=':')
          sep++;
        *sep = 0;
        showMemList.len++;
        showMemList.addr = realloc (showMemList.addr, showMemList.len * sizeof (*showMemList.addr));
        showMemList.addr [showMemList.len-1] = strtol (argv[i], 0, 16);
        showMemList.count = realloc (showMemList.count, showMemList.len * sizeof (*showMemList.count));
        showMemList.count [showMemList.len-1] = strtol (sep+1, 0, 16);
      } else if (strcmp (op, "r") == 0)
        showReg = 1;
      else
        return 0;
    } else if (i != argc-1)
      return 0;
  return 1;
}

int main (int argc, char** argv) {
  if (! parseArgs (argc, argv)) {
    fprintf (stderr, "usage: sm [-p startPC] [-m addr:count]* [-r] filename\n");
    return EXIT_FAILURE;
  }
  int ok = loadFile (argv[argc-1]);
  if (! ok) {
    fprintf (stderr, "error reading input file\n");
    return EXIT_FAILURE;
  }
  compute ();
  for (int i=0; i<showMemList.len; i++)
    showMem (showMemList.addr[i], showMemList.count[i]);
  if (showReg)
    for (int i=0; i<8; i++)
      printf ("r%d: 0x%08x (%d)\n", i, reg[i], reg[i]);
}