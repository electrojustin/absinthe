CC=riscv64-linux-gnu-g++
CFLAGS="-g" "-fPIE" "-static" "-fno-exceptions" "-fno-rtti" "-nostdlib" "-I$(shell pwd)"

test: 	boot.o \
	cpu/scratch.o \
	cpu/status.o \
	exec/executor.o \
	io/stdio.o \
	lib/string.o \
	main.o \
	memory/heap.o \
	memory/page_allocator.o \
	thread/hart.o \
	thread/lock.o
	${CC} ${CFLAGS} \
	boot.o \
	cpu/scratch.o \
	cpu/status.o \
	exec/executor.o \
	io/stdio.o \
	lib/queue.o \
	lib/string.o \
	main.o \
	memory/heap.o \
	memory/page_allocator.o \
	thread/hart.o \
	thread/lock.o \
	-T linker.ld -o test
boot.o: boot.S
	${CC} ${CFLAGS} -c boot.S -o boot.o
cpu/scratch.o: cpu/scratch.h cpu/scratch.cc
	${CC} ${CFLAGS} -c cpu/scratch.cc -o cpu/scratch.o
cpu/status.o: cpu/status.h cpu/status.cc
	${CC} ${CFLAGS} -c cpu/status.cc -o cpu/status.o
exec/executor.o: exec/executor.h exec/executor.cc lib/queue.h thread/hart.h
	${CC} ${CFLAGS} -c exec/executor.cc -o exec/executor.o
io/stdio.o: lib/string.h io/stdio.cc io/stdio.h
	${CC} ${CFLAGS} -c io/stdio.cc -o io/stdio.o
lib/string.o: lib/string.cc lib/string.h
	${CC} ${CFLAGS} -c lib/string.cc -o lib/string.o
main.o: main.cc io/stdio.h lib/queue.h
	${CC} ${CFLAGS} -c main.cc -o main.o
memory/heap.o: memory/heap.h memory/heap.cc memory/page_allocator.h
	${CC} ${CFLAGS} -c memory/heap.cc -o memory/heap.o
memory/page_allocator.o: memory/page_allocator.h memory/page_allocator.cc memory/page.h config.h
	${CC} ${CFLAGS} -c memory/page_allocator.cc -o memory/page_allocator.o
thread/hart.o: thread/hart.h thread/hart.cc config.h
	${CC} ${CFLAGS} -c thread/hart.cc -o thread/hart.o
thread/lock.o: thread/lock.h thread/lock.cc
	${CC} ${CFLAGS} -c thread/lock.cc -o thread/lock.o
clean:
	rm \
	boot.o \
	cpu/scratch.o \
	cpu/status.o \
	exec/executor.o \
	io/stdio.o \
	lib/string.o \
	main.o \
	memory/heap.o \
	memory/page_allocator.o \
	thread/hart.o \
	thread/lock.o \
	test
