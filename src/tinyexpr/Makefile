CC = gcc
CCFLAGS = -Wall -Wshadow -O2
LFLAGS = -lm

.PHONY = all clean

all: smoke smoke_pr repl bench example example2 example3


smoke: smoke.c tinyexpr.c
	$(CC) $(CCFLAGS) -o $@ $^ $(LFLAGS)
	./$@

smoke_pr: smoke.c tinyexpr.c
	$(CC) $(CCFLAGS) -DTE_POW_FROM_RIGHT -DTE_NAT_LOG -o $@ $^ $(LFLAGS)
	./$@

repl: repl.o tinyexpr.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LFLAGS)

repl-readline: repl-readline.o tinyexpr.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LFLAGS) -lreadline

bench: benchmark.o tinyexpr.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LFLAGS)

example: example.o tinyexpr.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LFLAGS)

example2: example2.o tinyexpr.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LFLAGS)

example3: example3.o tinyexpr.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LFLAGS)

repl-readline.o: repl.c
	$(CC) -c -DUSE_READLINE $(CCFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CCFLAGS) $< -o $@

clean:
	rm -f *.o *.exe example example2 example3 bench repl smoke_pr smoke
