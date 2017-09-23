#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "zhttp.h"
#include "zh-msg-priv.h"

/**
 * Test message construction of zh_msg_req_str()
 */
static void test_zh_msg_req_str()
{
    zh_msg_t * msg;

    /*Valid input with non-standard method*/
    assert((msg = zh_msg_req_str("AMETHOD", "/a/url", "HTTP/0.8")));
    assert(ZH_MSG_REQ == zh_msg_get_type(msg));
    assert(!memcmp("AMETHOD", msg->priv.req.method.data, msg->priv.req.method.len));
    assert(!memcmp("/a/url", msg->priv.req.url.data, msg->priv.req.url.len));
    assert(!memcmp("HTTP/0.8", msg->priv.req.httpv.data, msg->priv.req.httpv.len));
    assert(!memcmp("AMETHOD /a/url HTTP/0.8\r\n\r\n", msg->raw.data, msg->raw.len));
    assert(zh_msg_req_get_method(msg) == ZH_UNKNOWN_METHOD);
    zh_msg_free(msg);

    /*Test guards*/
    assert(!zh_msg_req_str(NULL, NULL, NULL));
    assert(!zh_msg_req_str(NULL, "/api/v1", "HTTP/1.1"));
    assert(!zh_msg_req_str("GET", NULL, "HTTP/1.1"));
    assert(!zh_msg_req_str("GET", "/api/v1", NULL));
}

/**
 * Test message construction of zh_msg_req()
 */
static void test_zh_msg_req()
{
    zh_msg_t * msg;

    /*Valid input with standard method*/
    assert((msg = zh_msg_req(ZH_GET, "/this/url")));
    assert(ZH_MSG_REQ == zh_msg_get_type(msg));
    assert(!memcmp("GET", msg->priv.req.method.data, msg->priv.req.method.len));
    assert(zh_msg_req_get_method(msg) == ZH_GET);
    assert(!memcmp("/this/url", msg->priv.req.url.data, msg->priv.req.url.len));
    assert(!memcmp("HTTP/1.1", msg->priv.req.httpv.data, msg->priv.req.httpv.len));
    assert(!memcmp("GET /this/url HTTP/1.1\r\n\r\n", msg->raw.data, msg->raw.len));
    zh_msg_free(msg);
}

/*Run Tests*/
int main(void)
{
    test_zh_msg_req();
    test_zh_msg_req_str();
    return 0;
}
