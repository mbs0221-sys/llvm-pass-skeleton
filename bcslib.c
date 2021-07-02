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

void write_bcs(bcs_t p)
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

void bcs_init() {
  printf("Function bcs_init is called.\n");
  bcs_t p = {
      .bcs = 0x00000000,
      .base = 0x80001798,
      .end = 0x800018a6,
      .cfg = 0x80,
  };
  write_bcs(p);
  printf("write bcs[%lx, %lx, %lx, %x]\n", p.bcs, p.base, p.end, p.cfg);
}

void bcs_checking() {
  printf("Function bcs_checking is called.\n");
  bcs_t p = read_bcs();
  printf("read bcs[%lx, %lx, %lx, %x]\n", p.bcs, p.base, p.end, p.cfg);
}