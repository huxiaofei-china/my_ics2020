#include "watchpoint.h"
#include "expr.h"
#include "stdlib.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].expr_str = NULL;
    wp_pool[i].expr_val = 0;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp()
{
  assert(free_ != NULL);
  WP* ret = free_;
  free_ = free_->next;

  ret->next = head;
  head = ret;
  return ret;
}

void free_wp(int NO)
{
  WP* temp = head;
  WP* wp = NULL;
  if (NO == head->NO)
  {
    wp = head;
    //delete wp from head
    head = wp->next;
    if (wp->expr_str)
    {
      free(wp->expr_str);
      wp->expr_str = NULL;
    }
    
    //add wp to free_
    wp->next = free_;
    free_ = wp;
    return;
  }
  else
  {
    while (temp->next != NULL)
    {
      if (NO == temp->next->NO)
      {
        wp = temp->next;
        temp->next = wp->next;
        if (wp->expr_str)
        {
          free(wp->expr_str);
          wp->expr_str = NULL;
        }
        wp->next = free_;
        free_ = wp;
        return;
      }
      else
      {
        temp = temp->next;
      }
    }
  }

  Log("free watch point %d failed\n", NO);  
}

WP* get_wp_header()
{
  return head;
}