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
  // TODO();
  // 在 IDTR 中设置好 IDT 的长度
  cpu.IDTR.IDT_LIMIT = vaddr_read(id_dest->addr, 2);

  // 判断操作数size，若为16位，则取(&data[1])起低24位，否则取32位
  if (decoding.is_operand_size_16) {
    cpu.IDTR.IDT_BASE = vaddr_read(id_dest->addr + 2, 4) & 0x00ffffff;
  }
  else {
    cpu.IDTR.IDT_BASE = vaddr_read(id_dest->addr + 2, 4); 
  }

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
  // //change the op and stop the program
  // decoding.seq_eip = cpu.eip;
  // scan_breakpoint();
  // exec_wrapper(1);
  // nemu_state = NEMU_STOP;
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
  t0 = pio_read(id_src->val,id_dest->width);
  rtl_sr(id_dest->reg, id_dest->width, &t0);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  //rtl_lr(&t0, id_src->reg, id_src->width);
  pio_write(id_dest->val, id_src->width,id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
