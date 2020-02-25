all: hfsh
hfsh: hfsh.c
	gcc -o hfsh hfsh.c -lpthread