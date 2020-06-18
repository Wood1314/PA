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
        //first page
        uint32_t page1_size = 0x1000 - (addr & 0xfff);
        paddr_t page1_paddr = page_translate(addr,false);

        //second page
        uint32_t page2 = addr + page1_size;
        paddr_t page2_paddr = page_translate(page2, false);

        //read
        uint32_t pre_data = paddr_read(page1_paddr, page1_size);
        uint32_t nxt_data = paddr_read(page2_paddr, len - page1_size);

        return pre_data | (nxt_data << ( page1_size * 8 ) );
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
        //first page
        uint32_t page1_size = 0x1000 - (addr & 0xfff);
        paddr_t page1_paddr = page_translate(addr,true);

        //second page
        uint32_t page2 = addr + page1_size;
        paddr_t page2_paddr = page_translate(page2, true);

        //write
        uint32_t nxt_data = data >> (32 - page1_size * 8);
        paddr_write(page1_paddr, page1_size, data);
        paddr_write(page2_paddr, len-page1_size, nxt_data);
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
    if(cpu.cr0.paging == 0)
      return vaddr;
    uint32_t pde_index = (vaddr >> 22) & 0x3ff;
    uint32_t pte_index = (vaddr >> 12) & 0x3ff;
    uint32_t off = vaddr &0xfff;

    //find pde
    uint32_t pde_base = cpu.cr3.page_directory_base;
    uint32_t pde = (pde_base << 12) + (pde_index << 2);
    PDE pde_obj;
    pde_obj.val = paddr_read(pde, 4);
    //Log("pde is 0x%x,  pde val is: 0x%x", pde, pde_obj.val);
    assert(pde_obj.present);

    //find pte
    uint32_t pte = (pde_obj.val & 0xfffff000 ) + (pte_index << 2);
    PTE pte_obj;
    pte_obj.val = paddr_read(pte, 4);
    //Log("pte is 0x%x,  pte val is: 0x%x", pte, pte_obj.val);
    assert(pte_obj.present);


    if (!pde_obj.accessed) {
        pde_obj.accessed = 1;
        paddr_write(pde, 4, pde_obj.val);
    }

    if (!pte_obj.accessed || ( !pte_obj.dirty == 0 && is_write)) {
        pte_obj.accessed = 1;
        pte_obj.dirty = 1;
        paddr_write(pte, 4, pte_obj.val);
    }

    uint32_t paddr = (pte_obj.val & 0xfffff000) | off;
    //Log("paddr is: 0x%x", paddr);
    return paddr;
}
