#include <stdio.h>
#include <assert.h>

#include "zhttp.h"
#include "zhttp-priv.h"

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

/**
 * Test basic allocation functions
 */
static void test_zhttp_alloc()
{
    void * ctx, * socket;
    zhttp_t * zh;

    /*Test with valid socket*/
    assert((ctx = zmq_ctx_new()));
    assert((socket = zhttp_socket(ctx)));
    assert((zh = zhttp_new(socket)));
    assert(zh->socket == socket);
    zhttp_destroy(zh);
    assert(!(zmq_close(socket)));
    assert(!(zmq_ctx_term(ctx)));

    /*Test with invalid socket*/
    assert((ctx = zmq_ctx_new()));
    assert((socket = zmq_socket(ctx, ZMQ_ROUTER)));
    assert(!(zh = zhttp_new(socket)));
    assert(!(zmq_close(socket)));
    assert(!(zmq_ctx_term(ctx)));

    /*Test guards*/
    assert(!zhttp_new(NULL));
    zhttp_destroy(NULL);
}

/**
 * Test Event Registration
 */
static int _free_count = 0;
static void _free_fun(void * data)
{
    (void) data;
    _free_count++;
}
static int _event_pass(const zh_msg_t * msg, void * data)
{
    (void) msg; (void) data;
    return ZH_EV_PASS;
}

static void test_zhttp_ev_reg()
{
    struct http_ev_node * ev;
    void * ctx, * socket;
    zhttp_t * zh;
    long i;

    assert((ctx = zmq_ctx_new()));
    assert((socket = zhttp_socket(ctx)));
    assert((zh = zhttp_new(socket)));
    assert(zh->socket == socket);

    for(i=0;i<10;i++)
        assert(!zhttp_ev_reg(zh, _event_pass, _free_fun, (void *)i));

    /*Verify order and length*/
    i=0;
    ev_node_foreach(ev, zh->ev_head){
        assert(ev->cb = _event_pass);
        assert(ev->data == (void *)i);
        i++;
    }
    assert(i==10);

    /*Verify tail is indeed at the end*/
    assert(!zh->ev_tail->next);

    zhttp_destroy(zh);
    assert(!(zmq_close(socket)));
    assert(!(zmq_ctx_term(ctx)));

    assert(_free_count == 10);
}

/**
 * Test Receive Function
 */
static void test_zhttp_recv()
{

}

/*Run Tests*/
int main(void)
{
    test_zhttp_socket();
    test_zhttp_alloc();
    test_zhttp_ev_reg();
    test_zhttp_recv();
    return 0;
}
