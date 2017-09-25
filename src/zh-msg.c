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

const void * zh_msg_get_prop(zh_msg_t * msg, zh_msg_prop_t prop, size_t * prop_len)
{
    /*Message type specific switches*/
    switch(msg->type){
        case ZH_MSG_REQ :
            switch(prop){
                case ZH_MSG_REQ_METHOD :
                    *prop_len = msg->priv.req.method.len;
                    return msg->priv.req.method.data;
                case ZH_MSG_REQ_URL :
                    *prop_len = msg->priv.req.url.len;
                    return msg->priv.req.url.data;
                case ZH_MSG_REQ_HTTPV :
                    *prop_len = msg->priv.req.httpv.len;
                    return msg->priv.req.httpv.data;
                default :
                    break;
            }
            break;
        case ZH_MSG_RES :
            switch(prop){
                case ZH_MSG_RES_HTTPV :
                    *prop_len = msg->priv.res.httpv.len;
                    return msg->priv.res.httpv.data;
                case ZH_MSG_RES_STAT :
                    *prop_len = msg->priv.res.stat.len;
                    return msg->priv.res.stat.data;
                case ZH_MSG_RES_STAT_MSG :
                    *prop_len = msg->priv.res.stat_msg.len;
                    return msg->priv.res.stat_msg.data;
                default :
                    break;
            }
            break;
        default :
            break;
    }

    /*Common message properties*/
    switch(prop){
        case ZH_MSG_HEADER :
            *prop_len = msg->header.len;
            return msg->header.data;
        case ZH_MSG_BODY :
            *prop_len = msg->body.len;
            return msg->body.data;
        case ZH_MSG_RAW :
            *prop_len = msg->raw.len;
            return msg->raw.data;
        case ZH_MSG_ID :
            *prop_len = msg->id.len;
            return msg->id.data;
        default :
            break;
    }

    return NULL;
}

int zh_msg_put_body(zh_msg_t * msg, const void * data, size_t data_len)
{
    if(__msg_data_increase_by(msg, data_len))
        return -1;

    memcpy(msg->body.data + msg->body.len, data, data_len);
    msg->body.len += data_len;

    return 0;
}

int zh_msg_put_header_str(zh_msg_t * msg, const char * header, const char * value)
{
    return zh_msg_put_header_strn(msg, header, strlen(header), value, strlen(value));
}

int zh_msg_put_header_strn(zh_msg_t * msg, const void * header, size_t header_len,
                                           const void * value, size_t value_len)
{
    void * head_ref;
    size_t mv_data_size, new_header_size;

    if(!msg)
        return -1;

    head_ref = msg->header.data + msg->header.len; //End of header segment
    mv_data_size = (msg->raw.data + msg->raw.len) - head_ref; //size of data following headers
    new_header_size = header_len + value_len + 3; //size of new header

    if(__msg_data_increase_by(msg, new_header_size))
        return -1;

    /*Move following data segment so new header can be inserted*/
    memmove(head_ref+new_header_size, head_ref, mv_data_size);

    /*Add header key*/
    memcpy(head_ref, header, header_len);
    head_ref += header_len;

    /*Add colon*/
    *((char*) head_ref++) = ':';

    /*Add header value*/
    memcpy(head_ref, value, value_len);
    head_ref += value_len;

    /*Add CRLF*/
    memcpy(head_ref, ZH_CRLF, ZH_CRLF_LEN);

    msg->header.len += new_header_size;

    msg->body.data += new_header_size;

    return 0;
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

zh_msg_t * zh_msg_res(zh_msg_t * req, int status)
{
    return zh_msg_res_str(req, status, __zh_stat_to_str(status));
}

zh_msg_t * zh_msg_res_str(zh_msg_t * req, int stat, const char * stat_msg)
{
    return zh_msg_res_strn(req, stat, stat_msg, strlen(stat_msg));
}

zh_msg_t * zh_msg_res_strn(zh_msg_t * req, int stat, const char * stat_msg, size_t stat_msg_len)
{
    zh_msg_t * res = NULL;

    size_t msg_size;

    (void) stat;
    (void) stat_msg;
    (void) stat_msg_len;

    if(!req)
        goto fail;

    if(!(res = __msg_new(ZH_MSG_INIT_SIZE)))
        goto fail;

    /*Copy message ID*/
    memcpy(res->id.data, req->id.data, req->id.len);
    res->id.len = req->id.len;

    res->type = ZH_MSG_RES;

    msg_size = stat_msg_len + sizeof(ZH_HTTP) + 8;
    if(__msg_data_increase_by(res, msg_size))
        goto fail;

    /*Set HTTP Version*/
    res->priv.res.httpv.data = memcpy(res->raw.data, ZH_HTTP, sizeof(ZH_HTTP)-1);
    res->priv.res.httpv.len = sizeof(ZH_HTTP)-1;

    /*Add white space*/
    res->priv.res.stat.data = res->raw.data + res->priv.res.httpv.len;
    *((char *)res->priv.res.stat.data) = ' ';
    res->priv.res.stat.data++;

    /*Add status code*/
    sprintf(res->priv.res.stat.data, "%.*u ", 3, (unsigned int) stat);
    res->priv.res.stat.len = 3;

    /*Add status message*/
    res->priv.res.stat_msg.data = res->priv.res.stat.data + 4;
    memcpy(res->priv.res.stat_msg.data, stat_msg, stat_msg_len);
    res->priv.res.stat_msg.len = stat_msg_len;

    /*Add CRLF*/
    memcpy(res->priv.res.stat_msg.data + stat_msg_len, ZH_CRLF ZH_CRLF, 2 * ZH_CRLF_LEN);

    /*Init header*/
    res->header.data = res->priv.res.stat_msg.data + stat_msg_len + ZH_CRLF_LEN;

    /*Init body*/
    res->body.data = res->header.data + ZH_CRLF_LEN;

    return res;

fail :
    __msg_free(res);
    return NULL;
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

    msg->type = ZH_MSG_REQ;

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

    /*Get Body Data*/
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

#define MIN_CHUNK_SIZE 5

void * __zh_msg_proc_chunk(const void * data, size_t * data_len)
{
    const void * size_begin,
               * size_end,
               * data_begin,
               * data_end;

    if(!(data && data_len && (*data_len>=MIN_CHUNK_SIZE)))
        goto fail;

    /*Find the size segment*/
    size_begin = data;
    size_end = __zh_memmem(size_begin, *data_len, ZH_CRLF, ZH_CRLF_LEN);
    if(!size_end)
        goto fail;

    /*Find data segment*/
    data_begin = size_end + ZH_CRLF_LEN;
    if(data_begin >= (data + *data_len))
        goto fail;
    data_end = data + (*data_len - ZH_CRLF_LEN);
    if(memcmp(data_end, ZH_CRLF, ZH_CRLF_LEN))
        goto fail;

    *data_len = (data_end - data_begin);

    /*TODO :We really should verify that the data length*/
    /*is correct here...*/

    return (void *) data_begin;

fail :
    *data_len = 0;
    return NULL;
}

const char * __zh_stat_to_str(int status)
{
    (void) status;
    return "OK";
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

