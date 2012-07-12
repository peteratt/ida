# Author:  Corentin Debains
# Email:   cdebains@iit.edu
#
# Author:  Pedro Alvarez-Tabio
# Email:   palvare3@iit.edu
#

CC=gcc
CPP=g++
CFLAGS=-Wall -Llib -Iinc -g -std=c99
CPPFLAGS=-I../lib/udt4/src -L../lib/udt4/src -ludt -lstdc++ -lpthread
CUDAINC=-I $(CUDA_INC_PATH)
CUDALIB=-L $(CUDA_LIB_PATH)
LFLAGS=-lecwrapper
OBJECTS=obj/ec.o

#tocheck
#.PHONY: clean jerasure examples

all:
	make examples

#######################################################
#Jerasure-1.2
#LFLAGS+=-ljerasure
OBJECTS += obj/jerasureCompatibility.o
LIBS+=libjerasure.a

lib/libjerasure.a: 
	cd lib/Jerasure-1.2 && make
	ar rus lib/libjerasure.a lib/Jerasure-1.2/*.o

#######################################################
#Gibraltar (CUDA)
CFLAGS+=$(CUDAINC)
#LFLAGS+=-lgibraltar
LFLAGS+=$(CUDALIB) 
LFLAGS+=-lcudart -lcuda
OBJECTS+= obj/gibraltarCompatibility.o
LIBS+=libgibraltar.a

lib/libgibraltar.a:  
	cd lib/libgibraltar-1.0 && make cuda=1
	ar rus lib/libgibraltar.a lib/libgibraltar-1.0/obj/*.o

#######################################################

examples: lib/libecwrapper.a
	$(CC) $(CFLAGS) examples/example.c -o examples/example $(LFLAGS)
	$(CC) $(CFLAGS) examples/fileEncoder.c -o examples/fileEncoder $(LFLAGS)
	$(CC) $(CFLAGS) examples/fileDecoder.c -o examples/fileDecoder $(LFLAGS)

lib/libecwrapper.a: libs $(OBJECTS)
	ar rus lib/libecwrapper.a obj/*.o 

# If you don't have CUDA, remove lib/libgibraltar.a
libs: lib/libgibraltar.a lib/libjerasure.a
	$(foreach var,$(LIBS),ar x lib/$(var); )
	mv *.o obj/

obj:
	mkdir -p obj
	
obj/ffsnet.o: src/ffsnet.cpp obj
	$(CPP) $(CPPFLAGS) -c src/ffsnet.cpp -o obj/ffsnet.o
	$(CPP) $(CPPFLAGS) -c src/ffsnet_bridger.cpp -o obj/ffsnet_bridger.o

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -c src/$*.c -o obj/$*.o

# A special kind of rule:  These files don't need to be remade if they're
# out of date, just destroyed.
cache:  src/gib_cuda_checksum.cu
	rm -rf cache
	mkdir cache

clean:
	rm -rf obj cache LFLAGS
	rm -f examples/example
	rm -f examples/fileEncoder
	rm -f examples/fileDecoder

mrproper: clean
	rm -f lib/*.a
