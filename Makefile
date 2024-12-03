main: *.cpp
	rm -f raytracer
	g++ -std=c++11 main.cpp -o main
