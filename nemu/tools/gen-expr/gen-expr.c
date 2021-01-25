#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static int buf_pos = 0;

static unsigned int choose(unsigned int n)
{
  unsigned int num = rand();
  return (num % n);
}

static inline void gen_rand_op()
{
  switch (choose(4))
  {
  case 0: 
    buf[buf_pos] = '+';
    break;
  case 1:
    buf[buf_pos] = '-';
    break;
  case 2:
    buf[buf_pos] = '*';
    break;
  case 3:
    buf[buf_pos] = '/';
    break;
  default:
    printf("error, choose(4) gererate wrong number");
    break;
  }
  buf_pos++;
}

static inline void gen_rand_expr() {
  switch (choose(3))
  {
  case 0:
    sprintf(buf+buf_pos, "%u", choose(5000));
    buf_pos += strlen(buf+buf_pos);
    break;
  case 1:
    buf[buf_pos] = '(';
    buf_pos++;
    gen_rand_expr();
    buf[buf_pos] = ')';
    buf_pos++;
    break;
  default:
    gen_rand_expr(); 
    gen_rand_op(); 
    gen_rand_expr(); 
    break;
  }
  buf[buf_pos] = '\0';
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
    buf_pos = 0;
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
