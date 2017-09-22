#include <stdio.h>
#include <assert.h>

#include "zhttp.h"

/**
 * Test that socket is returned correctly.
 */
static void test_zhttp_socket()
{
    void * ctx, * socket;

    int sock_type=-1;
    size_t sock_type_len=sizeof(sock_type);

    /*Test function guard*/
    assert(!zhttp_socket(NULL));

    /*Test proper socket type is returned*/
    assert((ctx = zmq_ctx_new()));
    assert((socket = zhttp_socket(ctx)));
    assert(!zmq_getsockopt(socket, ZMQ_TYPE, &sock_type, &sock_type_len));
    assert(sock_type_len == sizeof(sock_type));
    assert(sock_type == ZMQ_STREAM);
    assert(!(zmq_close(socket)));
    assert(!(zmq_ctx_term(ctx)));
}

/*Run Tests*/
int main(void)
{
    test_zhttp_socket();
    return 0;
}
