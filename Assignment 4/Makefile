all:
	gcc  -m32 -no-pie -nostdlib -o fib fib.c
	gcc  -m32 -no-pie -nostdlib -o sum sum.c
	gcc  -m32 -no-pie -nostdlib -o multidata multidata.c
	gcc  -m32 -no-pie -nostdlib -o rodata rodata.c
	gcc  -m32 -no-pie -nostdlib -o segfault segfault.c
	gcc  -m32 -no-pie -nostdlib -o sparse sparse.c
	gcc -m32 -o loader loader.c

clean:
	-@rm -f fib sum multidata rodata segfault sparse♦ loader
