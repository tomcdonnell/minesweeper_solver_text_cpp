minesweeper_text:	main.o minefield.o mineprob.o
	g++ -o minesweeper_text main.o minefield.o mineprob.o

main.o: main.cpp minefield.h mineprob.h
	g++ -c -Wall main.cpp

minefield.cpp: minefield.h
	g++ -c -Wall minefield.cpp

mineprob.cpp: mineprob.h
	g++ -c -Wall mineprob.cpp
