
CCOPTS = -std=c99 -Wall -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -O2 -g

all: lib

deps: cleandeps
	curl -q "https://raw.githubusercontent.com/kitomer/dynmem/master/dynmem.c" > dynmem.c
	curl -q "https://raw.githubusercontent.com/kitomer/dynmem/master/dynmem.h" > dynmem.h

dynmem.o: dynmem.c
	gcc $(CCOPTS) -fPIC -c dynmem.c -o dynmem.o 

ull.o: ull.c
	gcc -$(CCOPTS) -fPIC -c ull.c -o ull.o 

lib: dynmem.o ull.o
	gcc -shared -fPIC -Wl,-soname,libull.so.1 -o libull.so.0.1.0 dynmem.o ull.o -lc

install: lib
	cp libull.so.0.1.0 /usr/local/lib/libull.so.1
	ln -s /usr/local/lib/libull.so.1 /usr/local/lib/libull.so

clean:
	rm -f dynmem.o ull.o libull.so.0.1.0 test
	
cleandeps:
	rm -f dynmem.o dynmem.h dynmem.c

test: test.c
	gcc $(CCOPTS) -L. -lull test.c -o test
	
	