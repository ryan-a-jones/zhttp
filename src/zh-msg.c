#include "zhttp.h"
#include "zh-msg-priv.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/********************************************************************************
 *                      PRIVATE PROTOTYPE FUNCTIONS                             *
 *******************************************************************************/
/**
 * Get a new message reference
 *
 * @param init_size Initial data block size
 * @returns New zh_msg_t reference or NULL on memory failure
 */
static zh_msg_t * __msg_new(size_t init_size);

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

static int __msg_data_increase_by(zh_msg_t * msg, size_t inc);

/**
 * Free a message reference
 *
 * @param msg message reference
 */
static void __msg_free(zh_msg_t * msg);

/********************************************************************************
 *                          PUBLIC API FUNCTIONS                                *
 *******************************************************************************/

zh_msg_type_t zh_msg_get_type(const zh_msg_t * msg)
{
    return (msg)? msg->type : ZH_MSG_UNKNOWN;
}

void zh_msg_free(zh_msg_t * msg)
{
    __msg_free(msg);
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

	msg->socket = sock;

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
    msg->header.data = memcpy(data, ZH_CRLF, ZH_CRLF_LEN);
    msg->header.len = 0;
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

/********************************************************************************
 *                         PROTECTED FUNCTIONS                                  *
 *******************************************************************************/

zh_msg_t * __zh_msg_req_from_data(void * socket, const void * id, size_t id_len, const void * data, size_t data_len)
{
    zh_msg_t * msg;

    void * data_end;
    void * seg_start, * seg_end;

    if(!(msg = __msg_new(data_len)))
        goto fail;

    msg->socket = socket;

    /*Copy ID*/
    memcpy(msg->id.data, id, id_len);
    msg->id.len = id_len;

    /*Copy Data*/
    memcpy(msg->raw.data, data, data_len);
    msg->raw.len = data_len;
    data_end = msg->raw.data + data_len;

    seg_end = __zh_memmem(msg->raw.data, msg->raw.len, ZH_CRLF, ZH_CRLF_LEN);
    if(!seg_end)
        goto fail;
    seg_start = msg->raw.data;

    /*Get the method*/
    msg->priv.req.method.data = seg_start;
    seg_start = memchr(seg_start, ' ', seg_end - seg_start);
    if(!seg_start)
        goto fail;
    msg->priv.req.method.len = seg_start - msg->priv.req.method.data;

    /*Eat up white space*/
    while(isspace(*((unsigned char *)seg_start))){
        if(seg_start == seg_end)
            goto fail;
        seg_start++;
    }

    /*Get URL*/
    msg->priv.req.url.data = seg_start;
    seg_start = memchr(seg_start, ' ', seg_end - seg_start);
    if(!seg_start)
        goto fail;
    msg->priv.req.url.len = seg_start - msg->priv.req.url.data;

    /*Eat up white space*/
    while(isspace(*((unsigned char *)seg_start))){
        if(seg_start == seg_end)
            goto fail;
        seg_start++;
    }

    /*Get HTTP*/
    msg->priv.req.httpv.data = seg_start;
    msg->priv.req.httpv.len = seg_end - msg->priv.req.httpv.data;

    /*Get Header Data*/
    seg_start = seg_end + ZH_CRLF_LEN;
    seg_end = __zh_memmem(seg_start, data_end-seg_start, ZH_CRLF ZH_CRLF, ZH_CRLF_LEN*2);
    if(!seg_end)
        goto fail;
    seg_end += ZH_CRLF_LEN;
    msg->header.data = seg_start;
    msg->header.len = seg_end - seg_start;

    seg_start = seg_end + ZH_CRLF_LEN;
    msg->body.data = seg_start;
    msg->body.len = data_end - seg_start;

    return msg;
fail :
    __msg_free(msg);
    return NULL;
}

void * __zh_memmem(const void * hay, size_t hay_len, const void * need, size_t need_len)
{
    /*This probably isn't super efficient. Should check memmem*/
    /*source for a better idea of how to do it.               */

    while(hay_len >= need_len){
        if(!memcmp(hay, need, need_len))
            return (void *) hay;
        hay++;
        hay_len--;
    }

    return NULL;
}

/********************************************************************************
 *                      PRIVATE PROTOTYPE FUNCTIONS                             *
 *******************************************************************************/

static zh_msg_t * __msg_new(size_t init_size)
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

static int __msg_data_increase_by(zh_msg_t * msg, size_t inc)
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

static void __msg_free(zh_msg_t * msg)
{
    if(msg){
        if(msg->raw.data)
            free(msg->raw.data);
        free(msg);
    }
}

