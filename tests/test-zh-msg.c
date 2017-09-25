#include <stdio.h>
#include <stdlib.h>
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
    const void * prop;
    size_t prop_len;

    /*Set up 0mq*/
    assert(ctx = zmq_ctx_new());
    assert(sock = zhttp_socket(ctx));
    assert(!zmq_connect(sock, "inproc://test_zh_msg_req_str"));

    /*Valid input with non-standard method*/
    assert((msg = zh_msg_req_str(sock, "AMETHOD", "/a/url", "HTTP/0.8")));
    assert(ZH_MSG_REQ == zh_msg_get_type(msg));

    assert((prop = zh_msg_get_prop(msg, ZH_MSG_REQ_METHOD, &prop_len)));
    assert(!memcmp("AMETHOD", prop, prop_len));

    assert((prop = zh_msg_get_prop(msg, ZH_MSG_REQ_URL, &prop_len)));
    assert(!memcmp("/a/url", prop, prop_len));

    assert((prop = zh_msg_get_prop(msg, ZH_MSG_REQ_HTTPV, &prop_len)));
    assert(!memcmp("HTTP/0.8", prop, prop_len));

    assert((prop = zh_msg_get_prop(msg, ZH_MSG_RAW, &prop_len)));
    assert(!memcmp("AMETHOD /a/url HTTP/0.8\r\n\r\n", prop, prop_len));

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
    const void * prop;
    size_t prop_len;

    /*Set up 0mq*/
    assert(ctx = zmq_ctx_new());
    assert(sock = zhttp_socket(ctx));
    assert(!zmq_connect(sock, "inproc://test_zh_msg_req_str"));

    /*Valid input with standard method*/
    assert((msg = zh_msg_req(sock, ZH_GET, "/this/url")));
    assert(msg->socket == sock);
    assert(ZH_MSG_REQ == zh_msg_get_type(msg));
    assert(zh_msg_req_get_method(msg) == ZH_GET);

    assert((prop = zh_msg_get_prop(msg, ZH_MSG_REQ_METHOD, &prop_len)));
    assert(!memcmp("GET", prop, prop_len));

    assert((prop = zh_msg_get_prop(msg, ZH_MSG_REQ_URL, &prop_len)));
    assert(!memcmp("/this/url", prop, prop_len));

    assert((prop = zh_msg_get_prop(msg, ZH_MSG_REQ_HTTPV, &prop_len)));
    assert(!memcmp("HTTP/1.1", prop, prop_len));

    assert((prop = zh_msg_get_prop(msg, ZH_MSG_RAW, &prop_len)));
    assert(!memcmp("GET /this/url HTTP/1.1\r\n\r\n", prop, prop_len));
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
    const void * prop;
    size_t prop_len;

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

    assert((prop=zh_msg_get_prop(msg, ZH_MSG_ID, &prop_len)));
    assert(prop_len == 5);
    assert(!memcmp("MYID2", prop, 5));

    assert((prop=zh_msg_get_prop(msg, ZH_MSG_REQ_METHOD, &prop_len)));
    assert(prop_len == 3);
    assert(!memcmp(prop, "GET", 3));

    assert(ZH_GET == zh_msg_req_get_method(msg));

    assert((prop=zh_msg_get_prop(msg, ZH_MSG_REQ_URL, &prop_len)));
    assert(prop_len == 9);
    assert(!memcmp(prop, "/some/url", 9));

    assert((prop=zh_msg_get_prop(msg, ZH_MSG_REQ_HTTPV, &prop_len)));
    assert(prop_len == 8);
    assert(!memcmp(prop, "HTTP/0.9", 8));

    assert((prop=zh_msg_get_prop(msg, ZH_MSG_HEADER, &prop_len)));
    assert(prop_len == 45);
    assert(!memcmp(prop,
                "Some-Header:Value\r\n"
                "Another-Header:Something\r\n",
                45));

    assert((prop=zh_msg_get_prop(msg, ZH_MSG_BODY, &prop_len)));
    assert(prop_len == 12);
    assert(!memcmp(prop, "Payload Data", 12));

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

/*
 * Test response message string
 */
static void test_zh_stat_to_str()
{
    const char * stat_str;

    stat_str = __zh_stat_to_str(200);
    assert(0==strcmp(stat_str, "OK"));
}

/**
 * Test response message
 */

static void test_zh_msg_res_str()
{
    zh_msg_t * req, * res;

    const void * prop;
    size_t prop_len;

    assert((req = calloc(1, sizeof(zh_msg_t))));
    memcpy(req->id.data, "1234", 4);
    req->id.len = 4;

    assert((res = zh_msg_res_str(req, 404, "Not Found")));
    assert(ZH_MSG_RES == zh_msg_get_type(res));

    assert((prop = zh_msg_get_prop(res, ZH_MSG_ID, &prop_len)));
    assert(prop_len == 4);
    assert(!memcmp(prop, "1234", 4));

    assert((prop = zh_msg_get_prop(res, ZH_MSG_RES_HTTPV, &prop_len)));
    assert(prop_len == sizeof(ZH_HTTP)-1);
    assert(!memcmp(ZH_HTTP, prop, prop_len));

    assert((prop = zh_msg_get_prop(res, ZH_MSG_RES_STAT, &prop_len)));
    assert(prop_len == 3);
    assert(!memcmp("404", prop, 3));

    assert((prop = zh_msg_get_prop(res, ZH_MSG_RES_STAT_MSG, &prop_len)));
    assert(prop_len == 9);
    assert(!memcmp("Not Found", prop, 9));

    assert((prop = zh_msg_get_prop(res, ZH_MSG_RAW, &prop_len)));
    assert(prop_len == 26);
    assert(!memcmp("HTTP/1.1 404 Not Found\r\n\r\n", prop, 26));

    zh_msg_free(res);

    free(req);
}

static void test_zh_msg_put_body()
{
    zh_msg_t * req, * res;

    const void * prop;
    size_t prop_len;
    const char body[] = "Some Body Data";
    size_t body_len = sizeof(body) - 1;

    assert((req = calloc(1, sizeof(zh_msg_t))));
    memcpy(req->id.data, "1234", 4);
    req->id.len = 4;

    assert((res = zh_msg_res(req, 200)));

    assert(!zh_msg_put_body(res, body, sizeof(body)-1));

    assert((prop = zh_msg_get_prop(res, ZH_MSG_BODY, &prop_len)));
    assert(prop_len == body_len);
    assert(!memcmp(prop, body, prop_len));

    assert(!zh_msg_put_body(res, body, body_len));

    assert((prop = zh_msg_get_prop(res, ZH_MSG_BODY, &prop_len)));
    assert(prop_len == 2*body_len);
    assert(!memcmp(prop, body, body_len));
    assert(!memcmp(prop+body_len, body, body_len));

    zh_msg_free(res);
    free(req);
}

/*Run Tests*/
int main(void)
{
    test_zh_memmem();
    test_zh_msg_req();
    test_zh_msg_req_str();
    test_zh_msg_req_from_data();
    test_zh_msg_proc_chunk();
    test_zh_stat_to_str();
    test_zh_msg_res_str();
    test_zh_msg_put_body();
    return 0;
}
