GEN_SRC := Node.cpp mp2_constants.cpp mp2_types.cpp
GEN_OBJ := $(patsubst %.cpp,%.o, $(GEN_SRC))
CXX += -DHAVE_NETINET_IN_H -g
THRIFT_DIR := /class/ece428/libs/include/thrift
BOOST_DIR := /usr/local/include

INC += -I$(THRIFT_DIR) -I$(BOOST_DIR)

LINK += -L/class/ece428/libs/lib -Wl,-rpath,/class/ece428/libs/lib -lm -pthread

.PHONY: all clean

all: node listener

%.o: %.cpp
	$(CXX) -Wall $(INC) -c $< -o $@

node: Node_server.o sha1.c $(GEN_OBJ)
	$(CXX) $(LINK) -lthrift $^ -o $@

listener: listener.cpp sha1.c $(GEN_OBJ)
	$(CXX) $(INC) $(LINK) -lthrift $^ -o $@

clean:
	$(RM) *.o node listener *~
