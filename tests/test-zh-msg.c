#include <stdio.h>
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
    zh_msg_free(msg);
}

/*Run Tests*/
int main(void)
{
    test_zh_msg_req_str();
    return 0;
}
