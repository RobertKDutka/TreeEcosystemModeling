CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

test: main.cpp tree.o world.o graphics
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

graphics: visualizer.cpp
	g++ $(CFLAGS) -o skeletongraph visualizer.cpp $(LDFLAGS)

graphicdebug: visualizer.cpp
	g++ $(CFLAGS) -g -o skeletongraphdebug visualizer.cpp $(LDFLAGS)
	gdb skeletongraphdebug

.PHONY: clean

clean:
	rm -f *.o
	rm -f test
	rm -f testdebug
	rm -f tsi.txt
	rm -f tsp.txt
	rm -f skeletongraph
	rm -f skeletongraphdebug
