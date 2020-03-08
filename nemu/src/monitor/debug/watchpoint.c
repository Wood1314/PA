#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp(){
    if(free_ == NULL){
        assert(0);
    }
    //unlink
    WP *temp = free_;
    int new_NO = free_->NO;
    free_ = free_->next;
    //insert
    temp->next = NULL;
    wp_pool[new_NO-1].next = temp;
    return temp;
}

void free_wp(WP* wp){
    //insert
    wp->next = free_->next;
    free_->next = wp;
    //unlink
    int n_NO = wp->NO;
    wp_pool[n_NO-1].next = NULL;
    return;
}

extern uint32_t expr(char *e, bool *success);

int set_watchpoint(char *e){
    WP *new_point = new_wp();
    int NO = new_point->NO;
    if(strlen(e) >= 30 )
        assert(0);
    strcpy(new_point->expr,e);
    return NO;
}



