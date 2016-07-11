carousel: cards.cpp
	g++ -std=c++11 -o carousel cards.cpp -lSOIL -lGL -lglut -lGLU
clean:
	rm carousel
