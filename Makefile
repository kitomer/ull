
all: lib

dynmem.o: dynmem.c
	gcc -g -O2 -fPIC -Wall -Werror -std=c99 -o dynmem.o -c dynmem.c

ull.o: ull.c
	gcc -g -O2 -fPIC -Wall -Werror -std=c99 -o ull.o -c ull.c

lib: dynmem.o ull.o
	gcc -shared -fPIC -Wl,-soname,libdynmem.so.1 -o libdynmem.so.0.1.0 dynmem.o ull.o -lc

install: lib
	cp libdynmem.so.0.1.0 /usr/local/lib/libull.so.1
	ln -s /usr/local/lib/libull.so.1 /usr/local/lib/libull.so

clean:
	rm dynmem.o
	rm ull.o
	rm libull.so.0.1.0
	
