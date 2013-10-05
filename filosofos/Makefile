flags=-Wall

all: filosofos_semaforo filosofos_monitor

filosofos_semaforo: filosofos_semaforo.c
	gcc filosofos_semaforo.c -o filosofos_semaforo -lpthread $(flags)

filosofos_monitor: filosofos_monitor.c
	gcc filosofos_monitor.c -o filosofos_monitor -lpthread $(flags)

clean:
	rm filosofos_semaforo
	rm filosofos_monitor
