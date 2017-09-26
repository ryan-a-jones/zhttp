#include "zhttp.h"
#include "zhttp-priv.h"

#include <stdlib.h>

/********************************************************************************
 *                          PUBLIC API FUNCTIONS                                *
 *******************************************************************************/

zhttp_t * zhttp_new(void * socket)
{
    zhttp_t * zh;

    int sock_type;
    size_t sock_type_len=sizeof(sock_type);

    /*Check that we have the right socket type*/
    if(zmq_getsockopt(socket, ZMQ_TYPE, &sock_type, &sock_type_len))
        return NULL;

    if(sock_type != ZMQ_STREAM)
        return NULL;

    /*Allocate new instance*/
    if(!(zh = calloc(1, sizeof(zhttp_t))))
        return NULL;

    /*Initialize*/
    zh->socket = socket;

    return zh;
}

void zhttp_destroy(zhttp_t * zh)
{
    struct http_ev_node * ev_node;

    if(!zh) return;

    /*Free Event Nodes*/
    while((ev_node = zh->ev_head)){
        zh->ev_head = zh->ev_head->next;
        if(ev_node->free)
            ev_node->free(ev_node->data);
        free(ev_node);
    }

    free(zh);
}

int zhttp_ev_reg(zhttp_t * zh, zhttp_ev_fun_t ev, zhttp_free_fun_t free, void * data)
{
    struct http_ev_node * new_node = NULL;

    if(!(zh && ev))
        return -1;

    if(!(new_node = calloc(1, sizeof(struct http_ev_node))))
        return -1;

    new_node->cb = ev;
    new_node->data = data;
    new_node->free = free;

    if(!(zh->ev_head && zh->ev_tail)) //This is the first event
        zh->ev_head = new_node;
    else //Append to tail
        zh->ev_tail->next = new_node;
    zh->ev_tail = new_node;

    return 0;
}
