all: compile run

compile:
	gcc src/bfs.c -o bin/bfs -Wall

run:
	for i in $$(seq 0 7); \
	do \
		./bin/bfs < data/t$$i.txt; \
	done

clean:
	rm bin/*
