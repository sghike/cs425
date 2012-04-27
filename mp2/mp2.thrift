namespace cpp mp2

struct finger_entry {
	1: i32 id;
	2: i32 port;
}


service Node {
        finger_entry find_successor(1:i32 id);
	finger_entry closest_preceding_finger(1:i32 id);
        finger_entry get_successor();
        finger_entry get_predecessor();
        void notify(1:finger_entry n);
        void add_file(1:i32 key_id, 2:string s);
        i32 store_file(1:i32 key_id, 2:string s);
}
