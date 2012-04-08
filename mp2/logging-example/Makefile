LOG4CXX_DIR := /class/ece428/libs/include

LINK += -L/class/ece428/libs/lib -Wl,-rpath,/class/ece428/libs/lib

INC += -I$(LOG4CXX_DIR) -I/class/ece428/mp2

.PHONY: all clean

all: logexample

# $^ will be replaced with all the prerequisites, which in this
# example are logexample.cpp /class/ece428/mp2/log.cpp
logexample: logexample.cpp /class/ece428/mp2/log.cpp
	$(CXX) $(INC) $(LINK) -llog4cxx $^ -o $@

clean:
	$(RM) logexample
