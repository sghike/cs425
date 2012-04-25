namespace cpp mp2

struct finger_entry {
	1: i32 id;
	2: i32 port;
}


service Node {
	finger_entry closest_preceding_finger(1:i32 id);
        finger_entry get_successor();
}
