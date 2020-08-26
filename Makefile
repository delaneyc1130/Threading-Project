all: server part2 part3

server: 
		gcc quacker.c -pthread -o server
part2:
		gcc part2.c -pthread -o part2
part3:
		gcc part3.c -pthread -o part3

clean:
		rm server part2 part3
