#ifndef _ZHTTP_PRIV_H
#define _ZHTTP_PRIV_H

#include "zhttp.h"

/**
 * Node for HTTP events
 *
 * A singly-linked list node for events
 */
struct http_ev_node {
    zhttp_ev_fun_t cb;  /**< Event handler callback*/
    void * data;        /**< User data*/
    struct http_ev_node * next; /**< Reference to next node*/
    void (*free)(void *); /**< Optional freeing function*/
};

/*zhttp_t*/
struct zhttp {
    void * socket;
    struct http_ev_node * ev_head,
                        * ev_tail;
};

/**
 * Utility for iterating nodes
 */
#define ev_node_foreach(ev, head) for((ev)=(head);(ev);(ev)=(ev)->next)

#endif
