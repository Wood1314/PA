#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];
paddr_t page_translate(vaddr_t vaddr, bool is_write);

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mapNo = is_mmio(addr);
  if(mapNo != -1){
      return mmio_read(addr, len, mapNo);
  }
  else
  {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
  
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mapNo = is_mmio(addr);
  if(mapNo != -1){
    mmio_write(addr, len, data, mapNo);
  }
  else
  {
    memcpy(guest_to_host(addr), &data, len);
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging) {
      bool overflow_page = (addr >> 12) != ( (addr  + len -1) >> 12 );
      if (overflow_page) {
        assert(0);
      }
      else {
         paddr_t paddr = page_translate(addr, false);
         return paddr_read(paddr, len);
      }
  }
  else
    return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0.paging) {
      bool overflow_page = (addr >> 12) != ( (addr + len -1) >> 12 );
      if (overflow_page) {
        assert(0);
      }
      else {
          paddr_t paddr = page_translate(addr, true);
          paddr_write(paddr, len, data);
          return;
      }
  }
  else
    paddr_write(addr, len, data);
    return;
}

paddr_t page_translate(vaddr_t vaddr, bool is_write) {
  // 若没有开启分页机制，直接返回
  if (cpu.cr0.paging == 0)
    return vaddr;

  // Log("vaddr is: 0x%x", vaddr);
  // Log("CR3 page_directory_base is: 0x%x", cpu.cr3.page_directory_base);

  // 获取页目录索引，页表索引，页内偏移
  uint32_t pde_index = (vaddr>>22)&0x3ff;
  uint32_t pte_index = (vaddr>>12)&0x3ff;
  uint32_t off = vaddr & 0xfff;
  // Log("页目录索引 is: 0x%x", pde_index);
  // Log("页表索引 is: 0x%x", pte_index);
  // Log("页内偏移 is: 0x%x", off);


  // 查页目录,获取页表基址
  uint32_t pde_base = cpu.cr3.page_directory_base;
  uint32_t pde = (pde_base<<12) + (pde_index<<2);
  PDE pde_obj;
  pde_obj.val = paddr_read(pde, 4);
  assert(pde_obj.present);
  // Log("pde is 0x%x,  pde val is: 0x%x", pde, pde_obj.val);

  // 查页表，获取页框号
  uint32_t pte = (pde_obj.val & 0xfffff000) + (pte_index<<2);
  PTE pte_obj;
  pte_obj.val = paddr_read(pte, 4);
  // Log("pte is 0x%x,  pte val is: 0x%x", pte, pte_obj.val);
  assert(pte_obj.present);


  // 检验 PDE 的 accessed 位
  if (!pde_obj.accessed) {
    pde_obj.accessed = 1;
    paddr_write(pde, 4, pde_obj.val);
  }

  // // 检验 PTE 的 accessed 位
  if (!pte_obj.accessed || (!pte_obj.dirty == 0 && is_write)) {
    pte_obj.accessed = 1;
    pte_obj.dirty = 1;
    paddr_write(pte, 4, pte_obj.val);
  }

  uint32_t paddr = (pte_obj.val & 0xfffff000) | off;
  // Log("paddr is: 0x%x", paddr);

  return paddr;

}
