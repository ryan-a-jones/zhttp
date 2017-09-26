#ifndef _ZH_MSG_H
#define _ZH_MSG_H

#include "zhttp.h"
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
 *  - zh_msg_prop_t     : Enumerated message property types         *
 *  - zh_msg_get_prop() : Get a property of a message               *
 *  - zh_msg_put_body() : Add body data to a message                *
 *  - zh_msg_put_header_str()                                       *
 *  - zh_msg_put_header_strn() : Add a header field to a message    *
 *  - zh_msg_iter_header() : Iterate through headers                *
 *  - zh_msg_get_header_str()                                       *
 *    zh_msg_get_header_strn() : Get header by string               *
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
    ZH_MSG_UNKNOWN, /**< Unknown message type*/
    ZH_MSG_REQ,     /**< Request message type*/
    ZH_MSG_RES,     /**< Response message type*/
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
 * Message Property Type
 */
typedef enum {
    ZH_MSG_REQ_METHOD, /**< Request method*/
    ZH_MSG_REQ_URL,    /**< Request URL*/
    ZH_MSG_REQ_HTTPV,  /**< Request HTTP Version*/
    ZH_MSG_RES_HTTPV,  /**< Response HTTP Version*/
    ZH_MSG_RES_STAT,   /**< Response Status String */
    ZH_MSG_RES_STAT_MSG, /**< Response Status Message*/
    ZH_MSG_HEADER, /**< Message header*/
    ZH_MSG_BODY,   /**< Message body*/
    ZH_MSG_RAW,    /**< Message raw data*/
    ZH_MSG_ID,     /**< 0MQ ID*/
} zh_msg_prop_t;

/**
 * Get property of message
 *
 * @param   msg         ZHTTP message instance
 * @param   prop        Property type
 * @param   prop_len    Length of property size
 *
 * @returns Message property or NULL if property is invalid.
 *          This value may NOT be modified. The size of this
 *          data is set in the prop_len variable.
 */
const void * zh_msg_get_prop(zh_msg_t * msg, zh_msg_prop_t prop, size_t * prop_len);

/**
 * Add data to message body
 *
 * @param   msg         ZHTTP message instance
 * @param   data        Data to append
 * @param   data_len    Data length
 *
 * @return 0 on success, <0 on failure
 */
int zh_msg_put_body(zh_msg_t * msg, const void * data, size_t data_len);

/**
 * Append a header to a message
 *
 * @param   msg         ZHTTP message instance
 * @param   header      Message header key (must be NULL-terminated)
 * @param   value       Value of header key (must be NULL-terminated)
 *
 * @return 0 on success, <0 on failure
 */
int zh_msg_put_header_str(zh_msg_t * msg, const char * header, const char * value);

/**
 * Append a header to a message
 *
 * @param   msg         ZHTTP message instance
 * @param   header      Message header key
 * @param   header_len  Length of header
 * @param   value       Value of header key
 * @param   value_len   Length of value
 *
 * @return 0 on success, <0 on failure
 */
int zh_msg_put_header_strn(zh_msg_t * msg, const void * header, size_t header_len,
                                           const void * value, size_t value_len);

/**
 * Callback function for iterating through header
 *
 * Buffers are NOT null-terminated. Use care.
 *
 * @param   data        User-Defined Data
 * @param   header      Message header key
 * @param   header_len  Length of header
 * @param   value       Value of header key
 * @param   value_len   Length of value
 */
typedef void (*zh_msg_iter_header_fun_t)(void * data, const void * header, size_t header_len,
                                                      const void * value, size_t value_len);

/**
 * Iterate through message headers
 *
 * @param   msg         ZHTTP message instance
 * @param   cb          Callback for iterator function
 * @param   data        User-Defined Data passed to cb
 */
void zh_msg_iter_header(zh_msg_t * msg, zh_msg_iter_header_fun_t cb, void * data);

/**
 * Get the value of a header
 *
 * @param   msg         ZHTTP message instance
 * @param   header      the header key to search for (NULL-terminated)
 * @param   val_len     size of the return buffer
 *
 * @returns The header value or NULL if not found. This value
 *          is a buffer of length val_len and may not be modified
 */
const void * zh_msg_get_header_str(zh_msg_t * msg, const char * header, size_t * val_len);

/**
 * Get the value of a header
 *
 * @param   msg         ZHTTP message instance
 * @param   val_len     size of the return buffer
 * @param   header      the header key to search for
 * @param   header_len  the length of header
 *
 * @returns The header value or NULL if not found. This value
 *          is a buffer of length val_len and may not be modified
 */
const void * zh_msg_get_header_strn(zh_msg_t * msg, const void * header, size_t header_len,
                                    size_t * val_len);

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
 * @param   zh      ZHTTP Handle
 * @param   zsock   The ZMQ socket to make a request against
 * @param   method  The HTTP method to use
 * @param   url     The relative URL to request
 * @returns zh_msg_t request instance or NULL if memory allocation
 *          failure occurs.
 */
zh_msg_t * zh_msg_req(zhttp_t * zh, zh_method_t method, const char * url);

/**
 * Construct a new zh_msg_t message request instance with custom options.
 *
 * Free with call to zh_msg_free().
 *
 * @param   zh      ZHTTP Handle
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
zh_msg_t * zh_msg_req_str(zhttp_t * zh, const char * method, const char * url, const char * httpv);

/**
 * Construct a new zh_msg_t message request instance with custom options.
 *
 * Free with call to zh_msg_free().
 *
 * @param   zh      ZHTTP Handle
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
zh_msg_t * zh_msg_req_strn( zhttp_t * zh,
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
