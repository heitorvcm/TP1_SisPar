build:
	gcc threads.c -pthread -o threads

clean:
	rm -f threads