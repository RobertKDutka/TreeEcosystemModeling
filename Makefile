CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VulkanTest: main.cpp
	g++ $(CFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	./VulkanTest

debug: main.cpp
	g++ $(CFLAGS) -g -o VulkanTestDebug main.cpp $(LDFLAGS)
	gdb VulkanTestDebug

clean:
	rm -f VulkanTest
	rm -f VulkanTestDebug
