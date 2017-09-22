#ifndef _ZH_MSG_H
#define _ZH_MSG_H

#include "zh-method.h"

/**
 * ZHTTP Message Handle
 *
 * This is an opaque handle and, accordingly,
 * members should never be addressed directly.
 */
typedef struct zh_msg zh_msg_t;

/**
 * Construct a new zh_msg_t HTTP/1.1 message request instance.
 *
 * Free with call to zh_msg_free().
 *
 * @param   method  The HTTP method to use
 * @param   url     The relative URL to request
 * @returns zh_msg_t request instance or NULL if memory allocation
 *          failure occurs.
 *
 * Example :
 *      zh_msg_t * msg = zh_msg_req(ZH_GET, "/api/test");
 */
zh_msg_t * zh_msg_req(zh_method_t method, const char * url);

/**
 * Construct a new zh_msg_t message request instance with custom options.
 *
 * Free with call to zh_msg_free().
 *
 * @param   method  The HTTP method string to use
 * @param   url     The relative URL to request
 * @param   httpv   The HTTP version to send with.
 *                  NOTE: ZHTTP does not interpret the version sent.
 *                  Therefore, if you create an "HTTP/0.9" message, for example,
 *                  ZHTTP will not prevent you from sending chunked data or other
 *                  options unsupported by HTTP 0.9.
 * @returns zh_msg_t request instance or NULL if memory allocation
 *          failure occurs.
 *
 * Example :
 *      zh_msg_t * msg = zh_msg_req_str("GET", "/api/test", "HTTP/0.9");
 */
zh_msg_t * zh_msg_req_str(const char * method, const char * url, const char * httpv);

/**
 * Free a ZHTTP Request or Response
 */
void zh_msg_free(zh_msg_t * msg);

#endif
