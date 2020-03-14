

CC := gcc
CFLAGS += -std=c11 -fomit-frame-pointer
LOCS += -lSDL2 -lSDL2_image
OBJS += main.o

prog: all
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LOCS)
	rm $(OBJS)

all: $(OBJS)
$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -o $*.o -c $<
