#include "common.h"
extern void* new_page(void);
#define DEFAULT_ENTRY ((void *)0x8048000)
//#define DEFAULT_ENTRY ((void *)0x4000000)
void ramdisk_read(void *, off_t , size_t );

size_t get_ramdisk_size();
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_filesz(int fd);
extern int fs_open(const char *pathname, int flags, int mode);
extern int fs_close(int fd);

uintptr_t loader(_Protect *as, const char *filename) {
  //ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  int fd = fs_open(filename, 0, 0);
  //printf("fd = %d\n", fd);
  int file_size = fs_filesz(fd);
  fs_read(fd, DEFAULT_ENTRY, file_size); 
  void *pa;
  void *va = DEFAULT_ENTRY;

  while (file_size > 0) {
    Log("hello");
    pa = new_page();
    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
    _map(as, va, pa);
    fs_read(fd, pa, PGSIZE);
    va += PGSIZE;
    if(file_size < PGSIZE)
      break;
    file_size = file_size - PGSIZE;
  }
  if (file_size > 0) {
    pa = new_page();
    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
    _map(as, va, pa);
    fs_read(fd, pa, file_size);
  }
  fs_close(fd); 
  return (uintptr_t)DEFAULT_ENTRY;
}
