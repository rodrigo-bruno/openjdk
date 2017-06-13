#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

// If 'fixed' is true, anon_mmap() will attempt to reserve anonymous memory
// at 'requested_addr'. If there are existing memory mappings at the same
// location, however, they will be overwritten. If 'fixed' is false,
// 'requested_addr' is only treated as a hint, the return value may or
// may not start from the requested address. Unlike Linux mmap(), this
// function returns NULL to indicate failure.
static char* anon_mmap(char* requested_addr, size_t bytes, bool fixed) {
  char * addr;
  int flags;

  flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;
  if (fixed) {
    flags |= MAP_FIXED;
  }

  // Map reserved/uncommitted pages PROT_NONE so we fail early if we
  // touch an uncommitted page. Otherwise, the read/write might
  // succeed if we have enough swap space to back the physical page.
  addr = (char*)::mmap(requested_addr, bytes, PROT_NONE,
                       flags, -1, 0);

  return addr == MAP_FAILED ? NULL : addr;
}

static int anon_munmap(char * addr, size_t size) {                              
  return ::munmap(addr, size) == 0;                                             
}   

bool uncommit_memory(char* addr, size_t size) {
  int flags = MAP_PRIVATE|MAP_FIXED|MAP_NORESERVE|MAP_ANONYMOUS;
  uintptr_t res = (uintptr_t) ::mmap(addr, size, PROT_NONE, flags, -1, 0);
  return res  != (uintptr_t) MAP_FAILED;
}

// NOTE: Linux kernel does not really reserve the pages for us.
//       All it does is to check if there are enough free pages
//       left at the time of mmap(). This could be a potential
//       problem.
int commit_memory_impl(char* addr, size_t size, bool exec) {
  int prot = exec ? PROT_READ|PROT_WRITE|PROT_EXEC : PROT_READ|PROT_WRITE;
  int flags = MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS;
  uintptr_t res = (uintptr_t) ::mmap(addr, size, prot, flags, -1, 0);
  if (res != (uintptr_t) MAP_FAILED) {
    return 0;
  }

  int err = errno;  // save errno from mmap() call above
  return err;
}

void write(char* addr, size_t bytes, char value) {
  char* tmp = addr;
  int i = 0;
  for (; i++ < bytes;) {
    *tmp++ = value;
  }
}

int sum(char* addr, size_t bytes) {
  int sum = 0;
  char* tmp = addr;
  int i = 0;
  for (; i++ < bytes;) {
    sum += *tmp++;
  }
  return sum;
}

void wait_input() {
  printf("Press Enter to continue...\n");
  getchar();
}

int main(int argc, char** argv) {
  size_t size = 1024*1024*128;
  printf("Starting mmap test.\n");
  char* addr = anon_mmap(NULL, size, false);
  printf("Allocated buffer at %p\n", addr);
  commit_memory_impl(addr, size, false);
  write(addr, size, 1);
  printf("Sum is %d\n",  sum(addr, size));
  wait_input();
  uncommit_memory(addr, size);
  wait_input();
  anon_munmap(addr, size);
  wait_input();
  commit_memory_impl(addr, size, false);
  wait_input();
  write(addr, size, 2);
  printf("Sum is %d\n",  sum(addr, size));
  wait_input();
  printf("mmap test finished.\n");
  return 0;
}
