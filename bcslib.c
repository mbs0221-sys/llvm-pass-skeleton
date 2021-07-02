#include <stdio.h>
#include <stdint.h>
#include <riscv-pk/encoding.h>

typedef struct {
  uint64_t bcs;
  uint64_t base;
  uint64_t end;
  uint8_t cfg;
  uint64_t addr;
} bcs_t;

void set_bcs(bcs_t p)
{
  write_csr(0x8c0, p.bcs);
  write_csr(0x8c1, p.base);
  write_csr(0x8c2, p.end);
  write_csr(0x8c3, p.cfg);
  write_csr(0x8c4, p.addr);
  asm volatile ("sfence.vma" ::: "memory");
}

bcs_t read_bcs()
{
  bcs_t p = {
    .bcs = read_csr(0x8c0),
    .base = read_csr(0x8c1),
    .end = read_csr(0x8c2),
    .cfg = read_csr(0x8c3),
    .addr = read_csr(0x8c4)
  };  
  return p;
}

void read_time() {
    int t = rdtime();
    printf("[%08d] ", t);
}

void wrapper() {
  
  printf("This wrapper is called.\n");
}