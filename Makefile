all:server.out client.out
server.out:server.c
	gcc server.c -o server.out -lpthread -lsqlite3
client.out:client.c
	gcc client.c -o client.out -lpthread -lsqlite3

clean:
	rm *.out

