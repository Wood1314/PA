#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define WATCH_POINT 1
#define BREAK_POINT 2

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
 char expr[30];
 uint32_t old_val;
 uint32_t new_val;
 
 //add for breakpoint
 int type;
 uint32_t breakpoint_addr;
 uint32_t op;
} WP;

#endif
