#include "cpu/exec.h"
#include "monitor/monitor.h"
void diff_test_skip_qemu();
void diff_test_skip_nemu();
void exec_wrapper(bool);
extern void scan_breakpoint();

make_EHelper(lidt) {
  TODO();

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  //change the op and stop the program
  decoding.seq_eip = cpu.eip;
  scan_breakpoint(); 
  exec_wrapper(1);
  print_asm("pause\n");
  nemu_state = NEMU_STOP;
 // print_asm("int %s", id_dest->str);
  
#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  TODO();

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  TODO();

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  TODO();

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
