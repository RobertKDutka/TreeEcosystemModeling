test: main.cpp tree.o world.o
	g++ world.o tree.o main.cpp -o test
	./test

world.o: world.cpp world.hpp tree.hpp tree.cpp
	g++ -c world.cpp

tree.o: tree.cpp tree.hpp world.cpp world.hpp
	g++ -c tree.cpp