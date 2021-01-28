#include <isa.h>
#include "local-include/reg.h"

const char *regsl[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  int i;
  printf("pc: 0x%.8x\t", cpu.pc);
  for (i = 0; i < (sizeof(cpu.gpr)/sizeof(cpu.gpr[0])); i++)
  {
    printf("%s: 0x%.8x\t", regsl[i], cpu.gpr[i]._32);
  }
  printf("\n");
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int i;
  if (strcmp("pc", s) == 0)
  {
    *success = true;
    return cpu.pc;
  }
  for (i = 0; i < (sizeof(cpu.gpr)/sizeof(cpu.gpr[0])); i++)
  {
    if (strcmp(regsl[i], s) == 0)
    {
      *success = true;
      return cpu.gpr[i]._32;
    }
  }
  Log("can't find the reg named %s", s);
  *success = false;
  return 0;
}
