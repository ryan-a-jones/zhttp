#ifndef _ZH_MSG_PRIV_H
#define _ZH_MSG_PRIV_H

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
 * Maximum ZMQ_IDENTITY size
 */
#define ZMQ_IDENTITY_LEN 255

/**
 * Message Word String
 */
struct zh_msg_data {
    const void * data; /**< Reference String Start */
    size_t len;  /**< Reference String Length */
};

/* zh_msg_t */
struct zh_msg {
    zh_msg_type_t type;

    struct {
        char data[ZMQ_IDENTITY_LEN]; /**< Identity data*/
        size_t len; /**< Identity length*/
    } id; /**< ZMQ Identity*/

    union {
        struct {
            struct zh_msg_data method, /**< HTTP Method */
                               url,    /**< HTTP Url */
                               httpv,  /**< HTTP Version */
                               header; /**< HTTP Header */
        } req; /**< Request specific data*/
    } priv; /**< Fields for message type specific data*/

    struct zh_msg_data body;   /**< HTTP Body */

    struct {
        void * data; /**< Data Reference */
        size_t len; /**< Length of valid data*/
        size_t total_len; /**< Total allocated length*/
    } raw; /**< Raw data segment */
};

#endif
