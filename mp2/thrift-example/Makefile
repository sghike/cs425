GEN_SRC := MyService.cpp mp2_constants.cpp mp2_types.cpp
GEN_OBJ := $(patsubst %.cpp,%.o, $(GEN_SRC))
CXX += -DHAVE_NETINET_IN_H -g
THRIFT_DIR := /class/ece428/libs/include/thrift
BOOST_DIR := /usr/local/include

INC += -I$(THRIFT_DIR) -I$(BOOST_DIR)

LINK += -L/class/ece428/libs/lib -Wl,-rpath,/class/ece428/libs/lib

.PHONY: all clean

all: my_server my_client

%.o: %.cpp
	$(CXX) -Wall $(INC) -c $< -o $@

my_server: MyService_server.o $(GEN_OBJ)
	$(CXX) $(LINK) -lthrift $^ -o $@

my_client: MyService_client.o $(GEN_OBJ)
	$(CXX) $(LINK) -lthrift $^ -o $@

clean:
	$(RM) *.o my_server my_client
