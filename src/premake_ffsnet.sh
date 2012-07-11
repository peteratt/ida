g++ ffsnet.cpp -c -o ffsnet.o -I../lib/udt4/src -L../lib/udt4/src -ludt -lstdc++ -lpthread
g++ ffsnetd.cpp -o ffsnetd -I../lib/udt4/src -L../lib/udt4/src -ludt -lstdc++ -lpthread





g++ ffsnet_test.cpp -c -o ffsnet_test.o
g++ ffsnet.o ffsnet_test.o -o ffsnet_test -I../lib/udt4/src -L../lib/udt4/src -ludt -lstdc++ -lpthread

g++ ffsnet.o ffsnet_test.o -o ffsnet_test -I../lib/udt4/src -L../lib/udt4/src -ludt -lstdc++ -lpthread
