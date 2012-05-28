# vim: noet

minesweeper_text:	main.o minefield.o mineprob.o
	g++ -o minesweeper_text main.o minefield.o mineprob.o

main.o: minefield.h mineprob.h
	g++ -c -Wall main.cpp

minefield.o: minefield.h
	g++ -c -Wall minefield.cpp

mineprob.o: mineprob.h
	g++ -c -Wall mineprob.cpp
