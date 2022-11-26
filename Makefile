CC = gcc
CF = -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes \
     -Wdeclaration-after-statement -Wmissing-declarations \
     -Wimplicit-function-declaration -std=c99 -pedantic \
     -Wshadow -Wdouble-promotion -Wconversion -Wformat \
     -Wformat-signedness -Wformat-extra-args \
     -Wmissing-prototypes -Wstrict-prototypes -Wpointer-arith -Wcast-qual
LF = #
SC = main.c hash_map.c
OB = test 

all: $(SC)
	$(CC) -g -O0 $(CF) $(LF) $(SC) -o $(OB)
