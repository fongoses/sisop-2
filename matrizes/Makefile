all: matrizes_processos matrizes_threads

matrizes_processos: matrizes_processos.c
	gcc -o matrizes_processos matrizes_processos.c -lpthread -lrt

matrizes_threads: matrizes_threads.c
	gcc -o matrizes_threads matrizes_threads.c -lpthread -lrt

clean: 
	rm matrizes_processos
	rm matrizes_threads
    
