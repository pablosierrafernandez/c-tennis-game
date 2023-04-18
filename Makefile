all : winsuport.o tennis0

winsuport2.o : winsuport2.c winsuport2.h
	gcc -c -Wall winsuport2.c -o winsuport2.o 

memoria.o : memoria.c memoria.h
	gcc -c -Wall memoria.c -o memoria.o 

tennis3 : tennis3.c winsuport2.o winsuport2.h
	gcc -Wall tennis3.c memoria.o winsuport2.o -o tennis3 -lcurses -lpthread

clean: 
	rm winsuport.o tennis0
