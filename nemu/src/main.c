//#include "/home/huxiaofei/ics2020/nemu/src/monitor/debug/expr.h"
void init_monitor(int, char *[]);
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  init_monitor(argc, argv);

  /* test express */
  // expr_test();

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
