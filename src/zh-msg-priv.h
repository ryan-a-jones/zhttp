#ifndef _ZH_MSG_PRIV_H
#define _ZH_MSG_PRIV_H

/**
 * Message Word String
 */
struct zh_msg_data {
    void * data; /**< Reference String Start */
    size_t len;  /**< Reference String Length */
};

/* zh_msg_t */
struct zh_msg {
    struct zh_msg_data method, /**< HTTP Method */
                       url,    /**< HTTP Url */
                       httpv,  /**< HTTP Version */
                       header, /**< HTTP Header */
                       body;   /**< HTTP Body */

    struct {
        void * data; /**< Data Reference */
        size_t len; /**< Length of valid data*/
        size_t total_len; /**< Total allocated length*/
    } raw; /**< Raw data segment */
};

#endif
