all: mp0

SOURCEFILES = unicast.c mcast.c mp0.c
HEADERS = mp0.h


mp0: $(SOURCEFILES) $(HEADERS)
	gcc ${CFLAGS} -g -pthread -o $@ $(SOURCEFILES)

clean:	restart
	-rm -f mp0 *.o *~

restart:
	-rm -f GROUPLIST
