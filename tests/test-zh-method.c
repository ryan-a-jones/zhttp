#include <stdio.h>
#include <assert.h>

#include "zhttp.h"

/**
 * Test that strings return correct enumeration
 */
static void test_zh_method_from_str()
{
    assert(ZH_UNKNOWN_METHOD == zh_method_from_str("NOTAMETHOD"));
    assert(ZH_GET == zh_method_from_str("GET"));
    assert(ZH_POST == zh_method_from_str("POST"));
    assert(ZH_HEAD == zh_method_from_str("HEAD"));
    assert(ZH_PUT == zh_method_from_str("PUT"));
    assert(ZH_DELETE == zh_method_from_str("DELETE"));
    assert(ZH_CONNECT == zh_method_from_str("CONNECT"));
    assert(ZH_OPTIONS == zh_method_from_str("OPTIONS"));
    assert(ZH_TRACE == zh_method_from_str("TRACE"));
    assert(ZH_PATCH == zh_method_from_str("PATCH"));
}

/**
 * Test that strings (with length) return correct enumeration
 */
static void test_zh_method_from_strn()
{
    assert(ZH_UNKNOWN_METHOD == zh_method_from_strn("NOTAMETHOD",10));
    assert(ZH_GET == zh_method_from_strn("GET",3));
    assert(ZH_POST == zh_method_from_strn("POST",4));
    assert(ZH_HEAD == zh_method_from_strn("HEAD",4));
    assert(ZH_PUT == zh_method_from_strn("PUT",3));
    assert(ZH_DELETE == zh_method_from_strn("DELETE",6));
    assert(ZH_CONNECT == zh_method_from_strn("CONNECT",7));
    assert(ZH_OPTIONS == zh_method_from_strn("OPTIONS",7));
    assert(ZH_TRACE == zh_method_from_strn("TRACE",5));
    assert(ZH_PATCH == zh_method_from_strn("PATCH",5));
}

/**
 * Test that enumerations return correct string
 */
static void test_zh_method_to_str()
{
    assert(ZH_UNKNOWN_METHOD == zh_method_from_str(zh_method_to_str(ZH_UNKNOWN_METHOD)));
    assert(ZH_GET == zh_method_from_str(zh_method_to_str(ZH_GET)));
    assert(ZH_POST == zh_method_from_str(zh_method_to_str(ZH_POST)));
    assert(ZH_HEAD == zh_method_from_str(zh_method_to_str(ZH_HEAD)));
    assert(ZH_PUT == zh_method_from_str(zh_method_to_str(ZH_PUT)));
    assert(ZH_DELETE == zh_method_from_str(zh_method_to_str(ZH_DELETE)));
    assert(ZH_CONNECT == zh_method_from_str(zh_method_to_str(ZH_CONNECT)));
    assert(ZH_OPTIONS == zh_method_from_str(zh_method_to_str(ZH_OPTIONS)));
    assert(ZH_TRACE == zh_method_from_str(zh_method_to_str(ZH_TRACE)));
    assert(ZH_PATCH == zh_method_from_str(zh_method_to_str(ZH_PATCH)));
}

/*Run Tests*/
int main(void)
{
    test_zh_method_from_str();
    test_zh_method_from_strn();
    test_zh_method_to_str();
    return 0;
}
