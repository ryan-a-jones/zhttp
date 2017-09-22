#ifndef _ZH_MSG_PRIV_H
#define _ZH_MSG_PRIV_H

/**
 * Message Word String
 */
struct zh_msg_word {
    char * word; /**< Reference String Start */
    size_t word_len; /**< Reference String Length */
};

/* zh_msg_t */
struct zh_msg {
    struct zh_msg_word method, /**< HTTP Method */
                       httpv,  /**< HTTP Version */
                       header; /**< HTTP Header */

    struct {
        void * data; /**< Data Reference */
        size_t len; /**< Length of valid data*/
        size_t total_len; /**< Total allocated length*/
    } raw; /**< Raw data segment */
};

#endif
