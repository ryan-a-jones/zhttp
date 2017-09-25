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

/********************************************************************
 *                        ZHTTP MESSAGING                           *
 ********************************************************************
 * zh_msg_t is the structure used by zhttp for both inbound and     *
 * outbound messages. Some methods are available for all types      *
 * of messages while others can only be used with a specific type.  *
 *                                                                  *
 * SYMBOLS FOR ALL TYPES                                            *
 *  - zh_msg_t          : Message handle                            *
 *  - zh_msg_type_t     : Enumerated types for messages             *
 *  - zh_msg_get_type() : Get the zh_msg_type_t of the message      *
 *  - zh_msg_free()     : Free a message handle                     *
 *                                                                  *
 * SYMBOLS FOR REQUEST MESSAGES (ZH_MSG_REQ)                        *
 *  - zh_msg_req()                                                  *
 *    zh_msg_req_str()                                              *
 *    zh_msg_req_strn()       : Create a new request message        *
 *  - zh_msg_req_get_method() : Get method of request message       *
 * SYMBOLS FOR RESPONSE MESSAGES (ZH_MSG_RES)                       *
 *  - zh_msg_res()                                                  *
 *    zh_msg_res_str()                                              *
 *    zh_msg_res_strn()         : Create a new response message     *
 *                                                                  *
 *******************************************************************/

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
 * Get message type of a message
 *
 * @param msh ZHTTP message instance
 * @returns The type of the message. May be ZH_MSG_UNKNOWN
 *          if the type is not known.
 */
zh_msg_type_t zh_msg_get_type(const zh_msg_t * msg);

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
 * @param   zsock   The ZMQ socket to make a request against
 * @param   method  The HTTP method to use
 * @param   url     The relative URL to request
 * @returns zh_msg_t request instance or NULL if memory allocation
 *          failure occurs.
 */
zh_msg_t * zh_msg_req(void * zsock, zh_method_t method, const char * url);

/**
 * Construct a new zh_msg_t message request instance with custom options.
 *
 * Free with call to zh_msg_free().
 *
 * @param   zsock   The ZMQ socket to make a request against
 * @param   method  The HTTP method string to use
 * @param   url     The relative URL to request
 * @param   httpv   The HTTP version to send with.
 *                  NOTE: ZHTTP does not interpret the version sent.
 *                  Therefore, if you create an "HTTP/0.9" message, for example,
 *                  ZHTTP will not prevent you from sending chunked data or other
 *                  options unsupported by HTTP 0.9.
 * @returns zh_msg_t request instance or NULL if memory allocation
 *          failure occurs.
 */
zh_msg_t * zh_msg_req_str(void * zsock, const char * method, const char * url, const char * httpv);

/**
 * Construct a new zh_msg_t message request instance with custom options.
 *
 * Free with call to zh_msg_free().
 *
 * @param   zsock   The ZMQ socket to make a request against
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
zh_msg_t * zh_msg_req_strn( void * zsock,
                            const char * method, size_t method_len,
                            const char * url, size_t url_len,
                            const char * httpv, size_t httpv_len);

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
 * Instantiate a response message to a request message
 *
 * @param req       Request message to respond to
 * @param status    Status Code
 */
zh_msg_t * zh_msg_res(zh_msg_t * req, int status);

/**
 * Instantiate a response message to a request message
 *
 * @param req       Request message to respond to
 * @param stat      Status code (i.e. 200)
 * @param stat_msg  Status message (i.e. OK)
 * @returns zh_msg_t request instance or NULL if memory allocation
 *          failure occurs.
 */
zh_msg_t * zh_msg_res_str(zh_msg_t * req, int stat, const char * stat_msg);

/**
 * Instantiate a response message to a request message
 *
 * @param req           Request message to respond to
 * @param stat          Status code (i.e. 200)
 * @param stat_msg      Status message (i.e. OK)
 * @param stat_msg_len  Length of Status message
 * @returns zh_msg_t request instance or NULL if memory allocation
 *          failure occurs.
 */
zh_msg_t * zh_msg_res_strn(zh_msg_t * req, int stat, const char * stat_msg, size_t stat_msg_len);

#endif
