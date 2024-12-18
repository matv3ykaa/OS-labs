all: mutex atomic

mutex: mutex.c
	gcc -o mutex mutex.c -pthread

atomic: atomic.c
	gcc -o atomic atomic.c -pthread

clean:
	rm -f mutex atomic
