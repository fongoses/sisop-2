flags=-Wall
threadlib=-lpthread
ncurseslib=-lncurses
aiolib=-lrt
all: chat_server chat_client

chat_server: chat_server.c
	gcc -o chat_server chat_server.c $(threadlib) $(aiolib) $(flags) 

chat_client: chat_client.c
	gcc -o chat_client chat_client.c $(threadlib) $(aiolib) $(ncurseslib) $(flags)

clean:
	rm chat_server
	rm chat_client
