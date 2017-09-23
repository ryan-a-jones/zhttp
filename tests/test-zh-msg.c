#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "zhttp.h"
#include "zh-msg-priv.h"

/*
 * Test message construction of zh_msg_req_str()
 */
static void test_zh_msg_req_str()
{
    zh_msg_t * msg;

    /*Valid input with un-standard method*/
    assert((msg = zh_msg_req_str("AMETHOD", "/a/url", "HTTP/0.8")));
    assert(!memcmp("AMETHOD", msg->method.data, msg->method.len));
    assert(!memcmp("/a/url", msg->url.data, msg->url.len));
    assert(!memcmp("HTTP/0.8", msg->httpv.data, msg->httpv.len));
    assert(!memcmp("AMETHOD /a/url HTTP/0.8\r\n\r\n", msg->raw.data, msg->raw.len));
    zh_msg_free(msg);

    /*Test guards*/
    assert(!zh_msg_req_str(NULL, NULL, NULL));
    assert(!zh_msg_req_str(NULL, "/api/v1", "HTTP/1.1"));
    assert(!zh_msg_req_str("GET", NULL, "HTTP/1.1"));
    assert(!zh_msg_req_str("GET", "/api/v1", NULL));
}

/*
 * Test message construction of zh_msg_req()
 */
static void test_zh_msg_req()
{
    zh_msg_t * msg;

    /*Valid input with un-standard method*/
    assert((msg = zh_msg_req(ZH_GET, "/this/url")));
    assert(!memcmp("GET", msg->method.data, msg->method.len));
    assert(!memcmp("/this/url", msg->url.data, msg->url.len));
    assert(!memcmp("HTTP/1.1", msg->httpv.data, msg->httpv.len));
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
