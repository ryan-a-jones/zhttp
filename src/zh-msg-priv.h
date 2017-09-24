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

    void * socket; /**< 0MQ Socket*/

    union {
        struct {
            struct zh_msg_data method, /**< HTTP Method */
                               url,    /**< HTTP Url */
                               httpv;  /**< HTTP Version */
        } req; /**< Request specific data*/
    } priv; /**< Fields for message type specific data*/

    struct zh_msg_data header, /**< HTTP Header */
                       body;   /**< HTTP Body */

    struct {
        void * data; /**< Data Reference */
        size_t len; /**< Length of valid data*/
        size_t total_len; /**< Total allocated length*/
    } raw; /**< Raw data segment */
};

/**
 * Get a request message from raw data
 *
 * @param socket    0mq socket
 * @param id        0mq id (probably received from the socket)
 * @param id_len    Length of ID
 * @param data      Data to be interpreted as request
 * @param data_len  Length of data
 */
zh_msg_t * __zh_msg_req_from_data(void * socket, const void * id, size_t id_len, const void * data, size_t data_len);

/**
 * Process an HTTP chunk
 *
 * @param data      Data to parse
 * @param data_len  Length of data to parse. The length is over-written
 *                  with the length of data within the chunk
 *
 * @returns Pointer to data within chunk or NULL on failure
 */
void * __zh_msg_proc_chunk(const void * data, size_t * data_len);

/**
 * memmem replacement
 */
void * __zh_memmem(const void * hay, size_t hay_len, const void * need, size_t need_len);


#endif
