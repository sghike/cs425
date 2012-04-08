* First, assuming you're on the bash shell, run these two commands
  (and/or add them to your .bashrc/.bash_profile):

export PATH=$PATH:/class/ece428/libs/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/class/ece428/libs/lib

* Copy the files in this directory to your working/scratch space.


* Take a look at the included mp2.thrift example.


* Generate the code with:

thrift --gen cpp --out . mp2.thrift


* The main file you will touch is:

MyService_server.skeleton.cpp


* To avoid accidental overwriting of your work, rename it, e.g,:

mv MyService_server.skeleton.cpp MyService_server.cpp


* Now open up the included MyService_server.cpp, and you will see in the class
  MyServiceHandler:

MyServiceHandler() {
    // Your initialization goes here
}

void an_rpc_func(MyObject& _return, const MyObject& in_object, ...
    // Your implementation goes here
    printf("an_rpc_func\n");
}

void another_rpc_func(std::vector<MyObject> & _return) {
    // Your implementation goes here
    printf("another_rpc_func\n");
}


* You will fill out these stubbed functions to implement your
  service. Note that these functions return their results in the
  "_return" argument.


* Now we can create a client. See the included MyService_client.cpp.
  Note that in the Chord system, nodes are peers. So a node X can
  call/execute an RPC on another node Y, and Y can also call/execute
  an RPC on X. Beware of possible wait cycles, causing deadlocks.


* Create the Makefile (already included).


* Build the server and client:

make


* Now run the server:

./my_server


* In a different window, run the client:

./my_client


* See that the client blocks, waiting for the RPC call to
  complete. Our example server takes 10 seconds to complete the
  function. During this time, the server cannot answer other requests
  -- the server is single-threaded. For concurrency, see the Appendix
  at http://wiki.apache.org/thrift/ThriftUsageC%2B%2B

