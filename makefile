buildTest:
	gcc -std=c99 -D_POSIX_C_SOURCE=200112L -o s-talk instructorList.o keyboard.c screen.c sender.c recv.c main.c -pthread -Wall -Werror -g

clean:
	rm s-talk