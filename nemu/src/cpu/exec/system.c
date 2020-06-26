#include "cpu/exec.h"
#include "monitor/monitor.h"
void diff_test_skip_qemu();
void diff_test_skip_nemu();
void exec_wrapper(bool);
extern void scan_breakpoint();
uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);
extern void raise_intr(uint8_t , vaddr_t );

make_EHelper(lidt) {
  rtl_mv(&t0, &id_dest->addr);
  cpu.IDTR.IDT_LIMIT = vaddr_read(id_dest->addr,2);
  rtl_addi(&t0, &t0,2);
  if(decoding.is_operand_size_16){
    rtl_lm(&cpu.IDTR.IDT_BASE, &t0, 4);
    rtl_andi(&cpu.IDTR.IDT_BASE, &cpu.IDTR.IDT_BASE, 0x00ffffff);
  }
  else{
    rtl_lm(&cpu.IDTR.IDT_BASE, &t0, 4);
  }
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  //TODO();
  switch (id_dest->reg) {
    case 0: cpu.cr0.val = id_src->val; break;
    case 3: cpu.cr3.val = id_src->val; break;
    default: assert(0);
  }
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  switch (id_src->reg) {
    case 0: id_dest->val = cpu.cr0.val; break;
    case 3: id_dest->val = cpu.cr3.val; break;
    default: assert(0);
  }
  operand_write(id_dest, &id_dest->val);
  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  // //change the op and stop the program
  // decoding.seq_eip = cpu.eip;
  // scan_breakpoint();
  // exec_wrapper(1);
  // nemu_state = NEMU_STOP;
  Log("event is %x", id_dest->val);
  raise_intr(id_dest->val, decoding.seq_eip);
  print_asm("int %s", id_dest->str);
  
#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  rtl_pop(&decoding.jmp_eip);
  rtl_pop(&cpu.CS);
  rtl_pop(&cpu.eflags);
  decoding.is_jmp = 1;
  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
	t0 = pio_read(id_src->val, id_src->width);
  operand_write(id_dest, &t0);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val, id_src->width, id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}