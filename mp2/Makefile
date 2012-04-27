GEN_SRC := Node.cpp mp2_constants.cpp mp2_types.cpp
LISTEN_SRC := ListenerService.cpp
GEN_OBJ := $(patsubst %.cpp,%.o, $(GEN_SRC))
LISTEN_OBJ := $(patsubst %.cpp,%.o, $(LISTEN_SRC)) mp2_constants.o mp2_types.o
CXX += -DHAVE_NETINET_IN_H -g
THRIFT_DIR := /class/ece428/libs/include/thrift
BOOST_DIR := /usr/local/include

INC += -I$(THRIFT_DIR) -I$(BOOST_DIR)

LINK += -L/class/ece428/libs/lib -Wl,-rpath,/class/ece428/libs/lib -lm -pthread

.PHONY: all clean

all: node listener

%.o: %.cpp
	$(CXX) -Wall $(INC) -c $< -o $@

node: Node_server.o $(GEN_OBJ)
	$(CXX) $(LINK) -lthrift $^ -o $@

listener: listener.cpp sha1.c $(LISTEN_OBJ)
	$(CXX) $(INC) $(LINK) -lthrift $^ -o $@

clean:
	$(RM) *.o node listener *~
