#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val); 
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&id_dest->val);
  operand_write(id_dest, &id_dest->val);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  if (decoding.is_operand_size_16){
    rtl_lr(&t1, R_SP, 2);
    rtl_lr(&t0, R_AX,2);
    rtl_push(&t0);
    rtl_lr(&t0, R_CX, 2);
    rtl_push(&t0);
    rtl_lr(&t0, R_DX, 2);
    rtl_push(&t0);
    rtl_lr(&t0, R_BX, 2);
    rtl_push(&t0);
    rtl_push(&t1);
    rtl_lr(&t0, R_BP, 2);
    rtl_push(&t0);
    rtl_lr(&t0, R_SI, 2);
    rtl_push(&t0);
    rtl_lr(&t0, R_DI, 2);
    rtl_push(&t0);
  }else {
    rtl_lr(&t1, R_ESP, 4);
    rtl_lr(&t0, R_EAX, 4);
    rtl_push(&t0);
    rtl_lr(&t0, R_ECX, 4);
    rtl_push(&t0);
    rtl_lr(&t0, R_EDX, 4);
    rtl_push(&t0);
    rtl_lr(&t0, R_EBX, 4);
    rtl_push(&t0);
    rtl_push(&t1);
    rtl_lr(&t0, R_EBP, 4);
    rtl_push(&t0);
    rtl_lr(&t0, R_ESI, 4);
    rtl_push(&t0);
    rtl_lr(&t0, R_EDI, 4);
    rtl_push(&t0);
  }

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_lr(&t0, R_EBP, 4);
  rtl_sr(R_ESP, 4, &t0);
  rtl_pop(&t1);
  rtl_sr(R_EBP, 4, &t1);
  
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0, R_AL, 1);
    rtl_msb(&t1, &t0, 1);
    t3 = t1 == 0 ? 0 : ~0;
    rtl_mv(&t2, &t3);
    rtl_sr(R_AH,1, &t2);
  }
  else {
    rtl_lr(&t0, R_AX, 2);
    rtl_msb(&t1, &t0, 2);
    t3 = t1 ? (t0 | 0xffff0000) : t0;
    rtl_mv(&t2, &t3);
    rtl_sr(R_EAX, 4, &t2);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0, R_AX, 2);
    rtl_msb(&t1, &t0, 2);
    t3 = t1 == 0 ? 0 : ~0;
    rtl_mv(&t2, &t3);
    rtl_sr(R_DX,2, &t2);
  }
  else {
    rtl_lr(&t0, R_EAX, 4);
    rtl_msb(&t1, &t0, 4);
    t3 = t1 == 0 ? 0 : ~0;
    rtl_mv(&t2, &t3);
    rtl_sr(R_EDX, 4, &t2); 
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
