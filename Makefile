all: project2

project2: project2.c
	gcc project2.c -o project2
	mkfifo grades.html
clean:
	rm project2
