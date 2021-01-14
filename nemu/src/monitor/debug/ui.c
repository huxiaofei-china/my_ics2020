#include <isa.h>
#include "expr.h"
#include "watchpoint.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>

void cpu_exec(uint64_t);
int is_batch_mode();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char* args);

static int cmd_x(char* args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Excute N step program, and stop. Usage: si N", cmd_si},
  { "info", "Print information of reg or watchpoint. Usage: info r|w", cmd_info},
  { "x", "Print data start from EXPR with length N. Usage: x N EXPR", cmd_x},

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char* args)
{
  char *arg = strtok(NULL, " ");

  if (arg == NULL) {
    /* no argument given */
    printf("si usage: si N (N is number program to excute)\n");
  }
  else {
    int n = atoi(arg);
    if(n > 0)
    {
      cpu_exec(n);
      Log("si step %d, and current pc is: %x\n", n, cpu.pc);
    }
    else
    {
      Log("error,si N, and N should bigger than 0\n");
    }
  }
  return 0;
}

static int cmd_info(char* args)
{
  char *arg = strtok(NULL, " ");

  if (arg == NULL) {
    /* no argument given */
    printf("info usage: info r|w \n");
  }
  else {
    switch (*arg)
    {
    case 'r': 
      isa_reg_display();
      break;
    case 'w':
      Log("info w is excuted, but it's not implemented yet.\n");
      break;
    default:
      Log("info wrong args! Usage: info r|w\n");
      break;
    }
  }
  return 0;
}

static int cmd_x(char* args)
{
  char *arg = strtok(NULL, " ");
  char *sub_arg = NULL;
  uint32_t N;
  uint32_t EXPR;
  int i;
  if (arg == NULL) {
    /* no argument given */
    printf("missing arg N. x usage: x N EXPR\n");
  }
  else {
    N = atoi(arg);
    if (N <= 0)
    {
      printf("N should bigger than 0.\n");
      return 0;
    }
    
    sub_arg = strtok(NULL, " ");
    if (sub_arg == NULL)
    {
      printf("missing arg EXPR. x usage: x N EXPR\n");
      return 0;
    }
    else
    {
      EXPR = strtol(sub_arg, NULL, 16);
      printf("0x%x :\t", EXPR);
      for (i = 0; i < N; i++)
      {
        printf("0x%.8x\t", paddr_read(EXPR + i*4, 4));
      }
      printf("\n");
    }
     
  }
  return 0;
}

void ui_mainloop() {
  if (is_batch_mode()) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
