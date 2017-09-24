#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "zhttp.h"
#include "zh-msg-priv.h"

/**
 *Test the memmem replacement function
 */

static void test_zh_memmem()
{
    void * ret;

    assert(!memcmp("\r\n", "\r\n", 2));

    assert((ret = __zh_memmem("abcdef", 6, "cd", 2)));
    assert(!memcmp(ret, "cd", 2));

    const char crlfstr[] = "strwith\r\ncrlf";
    assert(sizeof(crlfstr) == 14);
    assert((ret = __zh_memmem(crlfstr, sizeof(crlfstr), ZH_CRLF, ZH_CRLF_LEN)));
}

/**
 * Test message construction of zh_msg_req_str()
 */
static void test_zh_msg_req_str()
{
    void * ctx, * sock;
    zh_msg_t * msg;

    /*Set up 0mq*/
    assert(ctx = zmq_ctx_new());
    assert(sock = zhttp_socket(ctx));
    assert(!zmq_connect(sock, "inproc://test_zh_msg_req_str"));

    /*Valid input with non-standard method*/
    assert((msg = zh_msg_req_str(sock, "AMETHOD", "/a/url", "HTTP/0.8")));
    assert(ZH_MSG_REQ == zh_msg_get_type(msg));
    assert(!memcmp("AMETHOD", msg->priv.req.method.data, msg->priv.req.method.len));
    assert(!memcmp("/a/url", msg->priv.req.url.data, msg->priv.req.url.len));
    assert(!memcmp("HTTP/0.8", msg->priv.req.httpv.data, msg->priv.req.httpv.len));
    assert(!memcmp("AMETHOD /a/url HTTP/0.8\r\n\r\n", msg->raw.data, msg->raw.len));
    assert(zh_msg_req_get_method(msg) == ZH_UNKNOWN_METHOD);
    zh_msg_free(msg);

    /*Test guards*/
    assert(!zh_msg_req_str(sock, NULL, NULL, NULL));
    assert(!zh_msg_req_str(sock, NULL, "/api/v1", "HTTP/1.1"));
    assert(!zh_msg_req_str(sock, "GET", NULL, "HTTP/1.1"));
    assert(!zh_msg_req_str(sock, "GET", "/api/v1", NULL));

    zmq_close(sock);
    zmq_ctx_term(ctx);
}

/**
 * Test message construction of zh_msg_req()
 */
static void test_zh_msg_req()
{
    void * ctx, * sock;
    zh_msg_t * msg;

    /*Set up 0mq*/
    assert(ctx = zmq_ctx_new());
    assert(sock = zhttp_socket(ctx));
    assert(!zmq_connect(sock, "inproc://test_zh_msg_req_str"));

    /*Valid input with standard method*/
    assert((msg = zh_msg_req(sock, ZH_GET, "/this/url")));
    assert(msg->socket == sock);
    assert(ZH_MSG_REQ == zh_msg_get_type(msg));
    assert(!memcmp("GET", msg->priv.req.method.data, msg->priv.req.method.len));
    assert(zh_msg_req_get_method(msg) == ZH_GET);
    assert(!memcmp("/this/url", msg->priv.req.url.data, msg->priv.req.url.len));
    assert(!memcmp("HTTP/1.1", msg->priv.req.httpv.data, msg->priv.req.httpv.len));
    assert(!memcmp("GET /this/url HTTP/1.1\r\n\r\n", msg->raw.data, msg->raw.len));
    zh_msg_free(msg);

    zmq_close(sock);
    zmq_ctx_term(ctx);
}

/**
 * Test request from data
 */
static void test_zh_msg_req_from_data()
{
    zh_msg_t * msg = NULL;

    /*Test malformed data*/
    msg = __zh_msg_req_from_data((void *) 1, "MYID", 4, "MYDATA", 6);
    assert(!msg);
    zh_msg_free(msg);

    const char reqdata[] =
            "GET /some/url HTTP/0.9\r\n"
            "Some-Header:Value\r\n"
            "Another-Header:Something\r\n"
            "\r\n"
            "Payload Data"
            ;

    /*Test good data*/
    msg = __zh_msg_req_from_data((void *) 1, "MYID2", 5, reqdata, sizeof(reqdata)-1);
    assert(msg);
    assert(msg->socket == (void *) 1);
    assert(ZH_MSG_REQ == zh_msg_get_type(msg));

    assert(msg->id.len == 5);
    assert(!memcmp("MYID2", msg->id.data, 5));

    assert(msg->priv.req.method.len == 3);
    assert(!memcmp(msg->priv.req.method.data, "GET", 3));
    assert(ZH_GET == zh_msg_req_get_method(msg));

    assert(msg->priv.req.url.len == 9);
    assert(!memcmp(msg->priv.req.url.data, "/some/url", 9));

    assert(msg->priv.req.httpv.len == 8);
    assert(!memcmp(msg->priv.req.httpv.data, "HTTP/0.9", 8));

    assert(msg->header.len == 45);
    assert(!memcmp(msg->header.data,
                "Some-Header:Value\r\n"
                "Another-Header:Something\r\n",
                45));

    assert(msg->body.len == 12);
    assert(!memcmp(msg->body.data, "Payload Data", 12));

    zh_msg_free(msg);
}

/*
 *Tests chunk processing
 */
static void test_zh_msg_proc_chunk()
{
    void * ret;

    const char chunk[] =
        "F\r\n"
        "-fifteen bytes-\r\n"
        ;
    size_t chunk_len = sizeof(chunk) - 1;

    ret = __zh_msg_proc_chunk(chunk, &chunk_len);
    assert(ret);
    assert(chunk_len == 15);
    assert(!memcmp(ret, "-fifteen bytes-", 15));

    const char badchunk[] =
        "1ffThisisnotvalid\r\n";
    chunk_len = sizeof(badchunk) - 1;
    ret = __zh_msg_proc_chunk(badchunk, &chunk_len);
    assert(!ret);

}

/*Run Tests*/
int main(void)
{
    test_zh_memmem();
    test_zh_msg_req();
    test_zh_msg_req_str();
    test_zh_msg_req_from_data();
    test_zh_msg_proc_chunk();
    return 0;
}
