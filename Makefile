all: shapes circle square ellipse ring semicircle 


shapes: shapes.o funcs.o
	gcc -g shapes.o funcs.o -o shapes


funcs.o: funcs.c funcs.h
	gcc -c funcs.c

shapes.o: shapes.c
	gcc -c shapes.c

circle: circle.o
	gcc -g circle.o -o circle


circle.o: circle.c
	gcc -c circle.c

semicircle: semicircle.o
	gcc -g semicircle.o -o semicircle


semicircle.o: semicircle.c
	gcc -c semicircle.c


ring: ring.o
	gcc -g ring.o -o ring


ring.o: ring.c
	gcc -c ring.c

square: square.o
	gcc -g square.o -o square


square.o: square.c
	gcc -c square.c


ellipse: ellipse.o
	gcc -g ellipse.o -o ellipse


ellipse.o: ellipse.c
	gcc -c ellipse.c

clean:
	rm *.o shapes circle square ellipse ring semicircle

