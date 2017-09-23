#ifndef _ZH_MSG_H
#define _ZH_MSG_H

#include "zh-method.h"

/**
 * Carriage-Return Line-Feed
 */
#define ZH_CRLF "\r\n"

/**
 * Carriage-Return Line-Feed Length
 */
#define ZH_CRLF_LEN 2

/**
 * HTTP Version
 */
#define ZH_HTTP "HTTP/1.1"

/**
 * Initial size for message allocation
 */
#define ZH_MSG_INIT_SIZE 100

/**
 * ZHTTP Message Handle
 *
 * This is an opaque handle and, accordingly,
 * members should never be addressed directly.
 */
typedef struct zh_msg zh_msg_t;

/**
 * Message Request Type
 */
typedef enum {
    ZH_MSG_UNKNOWN = -1, /**< Unknown message type*/
    ZH_MSG_REQ,     /**< Request message type*/
    ZH_MSG_RES,     /**< Response message type*/
    ZH_MSG_CHUNK,   /**< Chunked message type*/
} zh_msg_type_t;

/**
 * Free a ZHTTP Message
 *
 * @param msh ZHTTP message instance
 */
void zh_msg_free(zh_msg_t * msg);

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
 * @param   method_len The length of the method string
 * @param   url     The relative URL to request
 * @param   url_len The length of the url string
 * @param   httpv   The HTTP version to send with.
 *                  NOTE: ZHTTP does not interpret the version sent.
 *                  Therefore, if you create an "HTTP/0.9" message, for example,
 *                  ZHTTP will not prevent you from sending chunked data or other
 *                  options unsupported by HTTP 0.9.
 * @param   httpv_len The length of the httpv string
 *
 * @returns zh_msg_t request instance or NULL if memory allocation
 *          failure occurs.
 */
zh_msg_t * zh_msg_req_strn( const char * method, size_t method_len,
                            const char * url, size_t url_len,
                            const char * httpv, size_t httpv_len);

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
 * Get the method associated with the request message
 *
 * @param msh ZHTTP message instance
 * @returns The HTTP method of the message. If the
 *          message is not a request method or the method
 *          is unknown, ZH_UNKNOWN_METHOD is returned.
 */
zh_method_t zh_msg_req_get_method(const zh_msg_t * msg);

/**
 * Get message type of a message
 *
 * @param msh ZHTTP message instance
 * @returns The type of the message. May be ZH_MSG_UNKNOWN
 *          if the type is not known.
 */
zh_msg_type_t zh_msg_get_type(const zh_msg_t * msg);


#endif
