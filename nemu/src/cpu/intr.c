#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  
  rtl_push((rtlreg_t*)&cpu.eflags);
  cpu.IF = 0;
  rtl_push((rtlreg_t*)&cpu.CS);
  rtl_push((rtlreg_t*)&ret_addr);

  uint32_t IDT_BASE = cpu.IDTR.IDT_BASE;
  vaddr_t addr = IDT_BASE + NO * sizeof(GateDesc);
  uint32_t offset_low = vaddr_read(addr, 4) & 0x0000ffff;
  uint32_t offset_high = vaddr_read(addr+4, 4) & 0xffff0000;
  uint32_t offset = offset_high | offset_low;
  decoding.jmp_eip = offset;
  decoding.is_jmp = true;
}

void dev_raise_intr() {
  cpu.INTR = 1;
}
