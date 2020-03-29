#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "monitor/monitor.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

extern uint32_t vaddr_read(vaddr_t addr,int len); 
extern void vaddr_write(vaddr_t addr, int len, uint32_t data); 

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
    if(head != NULL)
        wp_pool[new_NO-1].next = temp;
    else
        head = temp;
    return temp;
}

void free_wp(WP* wp){
    int free_NO = free_->NO;
    //unlink
    int wp_NO = wp->NO;
    if(head->next == NULL && wp == head)
        head = NULL;
    else{
        for(int i=wp_NO; i<free_NO-1; i++){
            strcpy(wp_pool[i].expr,wp_pool[i+1].expr);
            wp_pool[i].old_val = wp_pool[i+1].old_val;
            wp_pool[i].new_val = wp_pool[i+1].new_val;
            wp_pool[i].type = wp_pool[i+1].type;
            wp_pool[i].breakpoint_addr = wp_pool[i+1].breakpoint_addr;
            wp_pool[i].op = wp_pool[i+1].op;
       }
        wp_pool[free_NO-2].next = NULL;
    }
    //insert
    WP *temp = free_;
    free_ = &wp_pool[free_NO-1];
    free_->next = temp;
    return;
}

extern uint32_t expr(char *e, bool *success);

int set_watchpoint(char *e,int type){
    if(type == WATCH_POINT){
        WP *new_point = new_wp();
        new_point->type = WATCH_POINT;
        int NO = new_point->NO;
        if(strlen(e) >= 30 )
            assert(0);
        strcpy(new_point->expr,e);

        bool success = false;
        new_point->old_val = expr(e,&success);
        if(!success){
            printf("Wrong express!\n");
            assert(0);
        }
        new_point->new_val = new_point->old_val;
        return NO;
    }
    else {
        WP *new_point = new_wp();
        new_point->type = BREAK_POINT;
        //set the NO
        int NO = new_point->NO;
        bool success = false;
        //set the breakpoint addr
        new_point->breakpoint_addr = expr(e, &success);
        if(!success){
            printf("Wrong express!\n");
        }
        new_point->op = vaddr_read(new_point->breakpoint_addr,1);
        vaddr_write(new_point->breakpoint_addr, 1, 0xcc);
        return NO;
    }
}

bool delete_watchpoint(int NO){
    WP *head2 = head;
    if(head2 == NULL)
        return false;
    while(head2->next){
        head2 = head2->next;
    }
    if(NO > head2->NO){
        return false;
    }
    else{
        free_wp(&wp_pool[NO]);
        return true;
    }
}

void list_watchpoint(){
    WP *head2 = head;
    printf("NO Expr               Old Value\n");
    while(head2){
        if(head2->type == WATCH_POINT){
            printf("%d  %-18s %#x\n",head2->NO,head2->expr,head2->old_val);
            head2 = head2->next; 
        }
        else if(head2->type == BREAK_POINT){
            printf("%d  at addr %#010x\n",head2->NO,head2->breakpoint_addr);
            head2 = head2->next;
        }
    }
    return;
}
//set break point
void set_breakpoint(){
    WP *head2 = head;
    while(head2){
        if(head2->type == BREAK_POINT){
            uint32_t old_op = vaddr_read(head2->breakpoint_addr,1);
            if(old_op != 0xcc){
                head2->op = old_op;
                vaddr_write(head2->breakpoint_addr,1,0xcc);
            }
        }
        head2 = head2->next;
    }
    return;
}

//scan the watch point on every comnd
WP* scan_watchpoint(){
    WP *head2 = head;
    bool success = false;
    while(head2){
        if(head2->type == WATCH_POINT){
            head2->new_val = expr(head2->expr,&success);
            if(head2->new_val != head2->old_val){
                return head2;
            }
        }
        head2 = head2->next;
    }
    return NULL;
}
void scan_breakpoint(){
    WP *head2 = head;
    while(head2){
        if(head2->type == BREAK_POINT){
            vaddr_write(head2->breakpoint_addr,head2->op,1);
        }
        head2 = head2->next;
    }

    nemu_state = NEMU_STOP;
    return;
}

