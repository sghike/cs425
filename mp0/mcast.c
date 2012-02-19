#include <string.h>
#include <assert.h>

#include "mp0.h"

void multicast_init(void) {
    unicast_init();
}

void receive(int source, const char *message, int len) {
    assert(message[len-1] == 0);

    // ...
}
