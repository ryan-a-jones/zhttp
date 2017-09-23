#include "zhttp.h"
#include "zh-msg-priv.h"

#include <string.h>
#include <stdlib.h>

/********************************************************************************
 *                      PRIVATE PROTOTYPE FUNCTIONS                             *
 *******************************************************************************/
/**
 * Get a new message reference
 *
 * @param init_size Initial data block size
 * @returns New zh_msg_t reference or NULL on memory failure
 */
zh_msg_t * __msg_new(size_t init_size);

/**
 * Increase the data size of the message.
 *
 * If the size is larger than the data already allocated,
 * data will be reallocated (by a factor of 2) in order
 * to accommodate the larger payload.
 *
 * @param msg message reference
 * @param inc the amount to grow the data by
 */

int __msg_data_increase_by(zh_msg_t * msg, size_t inc);

/**
 * Free a message reference
 *
 * @param msg message reference
 */
void __msg_free(zh_msg_t * msg);

/********************************************************************************
 *                          PUBLIC API FUNCTIONS                                *
 *******************************************************************************/

zh_msg_type_t zh_msg_get_type(const zh_msg_t * msg)
{
    return (msg)? msg->type : ZH_MSG_UNKNOWN;
}

zh_msg_t * zh_msg_req(void * sock, zh_method_t method, const char * url)
{
    return zh_msg_req_str(sock, zh_method_to_str(method), url, ZH_HTTP);
}

zh_msg_t * zh_msg_req_str(void * sock, const char * method, const char * url, const char * httpv)
{
    if(method && url && httpv)
        return zh_msg_req_strn(sock, method, strlen(method), url, strlen(url), httpv, strlen(httpv));

    return NULL;
}
zh_msg_t * zh_msg_req_strn( void * sock,
                            const char * method, size_t method_len,
                            const char * url, size_t url_len,
                            const char * httpv, size_t httpv_len)
{
    size_t new_size;
    void * data;
    zh_msg_t * msg = NULL;

    if(!(msg = __msg_new(ZH_MSG_INIT_SIZE)))
        goto fail;

    /*Get socket options*/
    msg->id.len = ZMQ_IDENTITY_LEN;
    if(zmq_getsockopt(sock, ZMQ_IDENTITY, msg->id.data, &msg->id.len))
        goto fail;

    msg->type = ZH_MSG_REQ;

    new_size = method_len + url_len + httpv_len + 4; //2 spaces + CRLF

    if(__msg_data_increase_by(msg, new_size))
        goto fail;

    data = msg->raw.data;

    /*Append Method*/
    msg->priv.req.method.data = memcpy(data, method, method_len);
    msg->priv.req.method.len = method_len;
    data += method_len;

    memcpy(data, " ", 1);
    data++;

    /*Append URL*/
    msg->priv.req.url.data = memcpy(data, url, url_len);
    msg->priv.req.url.len = url_len;
    data += url_len;

    memcpy(data, " ", 1);
    data++;

    /*Append HTTP Version*/
    msg->priv.req.httpv.data = memcpy(data, httpv, httpv_len);
    msg->priv.req.httpv.len = httpv_len;
    data += httpv_len;

    memcpy(data, ZH_CRLF, ZH_CRLF_LEN);
    data+=ZH_CRLF_LEN;

    /*Empty Header*/
    msg->priv.req.header.data = memcpy(data, ZH_CRLF, ZH_CRLF_LEN);
    msg->priv.req.header.len = 0;
    data+=ZH_CRLF_LEN;

    /*Empty Body*/
    msg->body.data = data;
    msg->body.len = 0;

    return msg;

fail :
    __msg_free(msg);
    return NULL;
}

zh_method_t zh_msg_req_get_method(const zh_msg_t * msg)
{
    if(!msg || (msg->type != ZH_MSG_REQ))
        return ZH_UNKNOWN_METHOD;

    return zh_method_from_strn((const char *) msg->priv.req.method.data, msg->priv.req.method.len);
}

void zh_msg_free(zh_msg_t * msg)
{
    __msg_free(msg);
}

/********************************************************************************
 *                      PRIVATE PROTOTYPE FUNCTIONS                             *
 *******************************************************************************/

zh_msg_t * __msg_new(size_t init_size)
{
    zh_msg_t * msg;

    if(!(msg = calloc(1, sizeof(zh_msg_t))))
        return NULL;

    if(!(msg->raw.data = malloc(init_size))){
        free(msg);
        return NULL;
    }

    msg->raw.total_len = init_size;

    return msg;
}

int __msg_data_increase_by(zh_msg_t * msg, size_t inc)
{
    void * redata;
    msg->raw.len += inc;

    while(msg->raw.total_len < msg->raw.len){
        //Double the storage size
        msg->raw.total_len <<= 1;
        redata = realloc(msg->raw.data, msg->raw.total_len);
        if(!redata){
            msg->raw.total_len >>= 1;
            return -1;
        }
        msg->raw.data = redata;
    }

    return 0;
}

void __msg_free(zh_msg_t * msg)
{
    if(msg){
        if(msg->raw.data)
            free(msg->raw.data);
        free(msg);
    }
}
