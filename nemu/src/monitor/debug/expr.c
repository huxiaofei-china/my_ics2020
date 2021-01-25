#include <stdlib.h>
#include <isa.h>
#include <stdio.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM
};

enum {
  CK_IS_PARENTH = 0, CK_NO_PARENTH, CK_WRONG_TYPE 
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"[0-9]+", TK_NUM},   // number
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"\\(", '('},         // parentheses left
  {"\\)", ')'},         // parentheses right
  {"\\,", ','},         // comma
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[100] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_NUM:
            tokens[nr_token].type = TK_NUM;
            Assert(substr_len < sizeof(tokens[nr_token].str), "%s is too long\n", \
                    tokens[nr_token].str);
            memcpy(tokens[nr_token].str, e+position-substr_len, substr_len);
            *(tokens[nr_token].str + substr_len) = '\0';
            nr_token++;
            break;
          default: 
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int check_parentheses(int p, int q)
{
  if ((tokens[p].type == '(') && (tokens[q].type == ')'))
  {
    int i = 0;
    int result_sum = 0;
    int ret = CK_IS_PARENTH;
    for (i = p+1; i <= q-1; i++)
    {
      if (tokens[i].type == '(') result_sum++;
      if (tokens[i].type == ')') result_sum--;
      
      if (result_sum == -1) ret = CK_NO_PARENTH;
      if (result_sum == -2) return CK_WRONG_TYPE;
    }

    if (result_sum == 0) return ret;
    else return CK_WRONG_TYPE; 
  }
  else
    return CK_NO_PARENTH;
}

static int find_master_op(int p, int q)
{
  int i;
  int op_level = 2;
  int ret = -1;
  int parenth_sum = 0;
  for (i = p; i <= q; i++)
  {
    if (tokens[i].type == '(') parenth_sum++;
    if (tokens[i].type == ')') parenth_sum--;

    if (parenth_sum < 0) return -1;

    if (parenth_sum == 0)
    {
      if ((tokens[i].type == '+') || (tokens[i].type == '-')) 
      {
        op_level = 1;
        ret = i;
      }
      if ((tokens[i].type == '*') || (tokens[i].type == '/'))
      {
        if (op_level == 2) ret = i;
      } 
    }
  }
  return ret; 
}

static u_int32_t eval(int p, int q, int* error_flag)
{
  int ck_result;
  if (p > q) {
    *error_flag = 1;
    return 0;
  }
  else if (p == q) {
    if (tokens[p].str == NULL)
    {
      *error_flag = 1;
      return 0;
    }
    else 
    {
      u_int32_t ret;
      sscanf(tokens[p].str, "%u", &ret);
      return ret;
    }
  }
  else if ((ck_result = check_parentheses(p, q)) == CK_IS_PARENTH) {
    return eval(p + 1, q - 1, error_flag);
  }
  else {
    if (ck_result == CK_WRONG_TYPE)
    {
      *error_flag = 1;
      Log("the express's parentheses is wrong\n");
      return 0;
    }
    else
    {
      int op = find_master_op(p, q);
      if (op == -1)
      {
        *error_flag  = 1;
        Log("can't find master op\n");
        return 0;
      }
      u_int32_t val1 = eval(p, op - 1, error_flag);
      u_int32_t val2 = eval(op + 1, q, error_flag);
      if (*error_flag == 1) return 0;
      
      switch (tokens[op].type) {
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': return val1 / val2;
        default: assert(0);
      } 
    }
    
  }
}

word_t expr(char *e, bool *success) {
  int error_flag = 0;
  u_int32_t value = 0;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  value = eval(0, nr_token-1, &error_flag);
  if (error_flag == 1) 
  {
    Log("the eval func wrong, the express is illegal\n");
    *success = false;
    return 0;
  }
  else 
  {
    Log("get the express is %u", value);
    return value;
  }
}

void expr_test()
{
  FILE* fp = fopen("/home/huxiaofei/ics2020/nemu/tools/gen-expr/input", "r");
  u_int32_t result = 0;
  char* express = malloc(1000);
  u_int32_t express_value;
  bool express_success = true;
  while (fscanf(fp, "%u%s", &result, express) != EOF)
  {
    express_value = expr(express, &express_success);
    if (express_success)
    {
      if (express_value == result)
      {
        printf("%u is correct\n", result);
      }
      else
      {
        printf("express %s result %u calculate is wrong\n", express, result);
      }
    }
    else
    {
      printf("expr func is wrong, the express is illegal\n");
    }
    
  }
  
}
