make: libcounter.so liblist.so libhash.so

P4:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
	cc -lcounter -llist -lhash -L. -o P4 main.c libcounter.so liblist.so libhash.so -lpthread

libcounter.so:
	cc -shared -fPIC counter.c lock.h lock.c -o libcounter.so -Wall -Werror

liblist.so:
	cc -shared -fPIC list.c lock.h lock.c -o liblist.so -Wall -Werror

libhash.so:
	cc -shared -fPIC hash.c list.h list.c lock.h lock.c -o libhash.so -Wall -Werror
