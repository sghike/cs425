all: listener

SOURCEFILES = listener.cpp
HEADERS = listener.hpp

chat: $(SOURCEFILES) $(HEADERS)
	gcc ${CFLAGS} -g -o $@ $(SOURCEFILES)

clean:	restart
	-rm -f listener *.o *~

restart:
	-rm -f GROUPLIST
