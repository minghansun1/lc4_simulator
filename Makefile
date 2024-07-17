all: trace

trace: LC4.o loader.o trace.c
	#
	#NOTE: CIS 240 students - this Makefile is broken, you must fix it before it will work!!
	#
	clang -g LC4.o loader.o trace.c -o trace

LC4.o:
	#
	#CIS 240 TODO: update this target to produce LC4.o
	#
	clang -g -c LC4.c

loader.o:
	#
	#CIS 240 TODO: update this target to produce loader.o
	#
	clang -g -c loader.c

clean:
	rm -rf *.o

clobber: clean
	rm -rf trace
