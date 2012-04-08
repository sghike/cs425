INC += -I/class/ece428/mp2

.PHONY: all clean

all: hashexample

# $^ will be replaced with all the prerequisites, which in this
# example are logexample.cpp /class/ece428/mp2/log.cpp
hashexample: key_gen_test.cpp /class/ece428/mp2/sha1.c
	$(CXX) $(INC) $^ -o $@

clean:
	$(RM) hashexample
