#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 1000

int nemu_state = NEMU_STOP;

void exec_wrapper(bool);

extern WP* scan_watchpoint(); 

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  bool print_flag = n < MAX_INSTR_TO_PRINT;

  for (; n > 0; n --) {
    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    uint32_t eip_now = cpu.eip;

    exec_wrapper(print_flag);
#ifdef DEBUG
    /* TODO: check watchpoints here. */
    WP* change = scan_watchpoint();
    if(change != NULL){
        printf("Hi watchpoint %d at addr %#010x\n",change->NO,eip_now);
        printf("expr      =  %s\n",change->expr);
        printf("old value =  %#010x\n",change->old_val);
        printf("new_value =  %#010x\n",change->new_val);
        printf("program pause\n");
        change->old_val = change->new_val;
        nemu_state = NEMU_STOP;
    }
#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
