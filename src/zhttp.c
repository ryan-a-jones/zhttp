#include "zhttp.h"
#include "zhttp-priv.h"
#include "zh-msg-priv.h"

#include <stdlib.h>

/********************************************************************************
 *                           PRIVATE UTILITIES                                  *
 *******************************************************************************/

#define _zhttp_send(sock, buf, len) zmq_send(sock, buf, len, ZMQ_SNDMORE)

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

int zhttp_recv(zhttp_t * zh, int zmq_flags)
{
    int rc = 0;

    void * data;
    size_t data_len;

    unsigned char id[ZMQ_IDENTITY_LEN];
    size_t id_len;

    zmq_msg_t msg;
    zh_msg_t * http_msg = NULL;

    struct http_ev_node * ev;

    (void) zmq_msg_init(&msg);

    /*Get the ID Frame*/
    rc = zmq_recv(zh->socket, &id, ZMQ_IDENTITY_LEN, zmq_flags);
    if(rc == -1)
        goto exit;
    id_len = rc;

    /*Get the HTTP frame*/
    rc = zmq_msg_recv(&msg, zh->socket, 0); //Ignore zmq_flags here... need to block
    if(rc == -1)
        goto exit;
    data = zmq_msg_data(&msg);
    data_len = zmq_msg_size(&msg);

    /*Convert to HTTP Message*/
    http_msg = __zh_msg_req_from_data(zh->socket, id, id_len, data, data_len);
    if(!http_msg){
        rc = -1;
        goto exit;
    }

    /*Iterate through event nodes*/
    ev_node_foreach(ev, zh->ev_head){
        rc = ev->cb(http_msg, ev->data);
        switch(rc){
            case ZH_EV_ERROR :
            case ZH_EV_OK :
                goto exit;
            case ZH_EV_PASS :
                break;
            default :
                rc = -1;
                goto exit;
        }
    }

    rc = 0;
exit :
    zmq_msg_close(&msg);
    if(http_msg)
        zh_msg_free(http_msg);
    return rc;
}

int zhttp_send(zh_msg_t * msg)
{
    int rc;

    /*Send ID first*/
    rc = _zhttp_send(msg->socket, msg->id.data, msg->id.len);
    if(rc != (int) msg->id.len)
        return -1;

    /*Send Data*/
    rc = _zhttp_send(msg->socket, msg->raw.data, msg->raw.len);
    if(rc != (int) msg->raw.len)
        return -1;

    return 0;
}
