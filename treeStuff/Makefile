test: main.cpp tree.o world.o
	g++ tree.o world.o main.cpp -o test
	./test

world.o: world.cpp world.hpp tree.hpp tree.cpp
	g++ -c world.cpp

tree.o: tree.cpp tree.hpp world.cpp world.hpp
	g++ -c tree.cpp

debug: main.cpp treedebug.o worlddebug.o
	g++ treedebug.o worlddebug.o main.cpp -o testdebug -g
	gdb testdebug

treedebug.o: 
	g++ -g -c tree.cpp -o treedebug.o

worlddebug.o:
	g++ -g -c world.cpp -o worlddebug.o

.PHONY: clean

clean:
	rm -f *.o
	rm test
	rm testdebug
