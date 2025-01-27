main: *.cpp
	rm -f main
	g++ -std=c++11 -fms-extensions src/*.cpp -framework OpenCL -o main
