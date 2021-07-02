CC=clang
GCC=riscv64-unknown-elf-gcc
CFLAGS=--target=riscv64 -march=rv64gc -I$(RISCV)/riscv64-unknown-elf/include

rtlib.o : rtlib.c
	$(CC) $(CFLAGS) -O -c $< -o $@

bcslib.o : bcslib.c
	$(CC) $(CFLAGS) -O -c $< -o $@

example.o : example.c
	$(CC) $(CFLAGS) -Xclang -load -Xclang build/skeleton/libSkeletonPass.so -c $< -o $@

example : example.o rtlib.o bcslib.o
	$(GCC) -o $@ $+

clean:
	rm *.o example