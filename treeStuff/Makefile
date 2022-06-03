test: main.cpp tree.o world.o
	g++ tree.o world.o main.cpp -o test
	./test

world.o: world.cpp world.hpp tree.hpp tree.cpp
	g++ -c world.cpp

tree.o: tree.cpp tree.hpp world.cpp world.hpp
	g++ -c tree.cpp

clean:
	rm -f world.o
	rm -f tree.o
	rm test