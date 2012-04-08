namespace cpp mp2

struct MyObject {
    1: i32 i; /* 1st member is i, integer 32 */
    2: string s; /* 2nd member */
    3: double d;
}

/* declare the RPC interface of your network service */
service MyService {
    /* this function takes 3 arguments */
    MyObject an_rpc_func(1:MyObject in_object,
    	     		 2:i32 another_arg,
			 3:list<string> arg3);

    /* functions can return a list as well, or can also return nothing
    "void" */
    list<MyObject> another_rpc_func();
}
