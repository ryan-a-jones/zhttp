#include "zhttp.h"
#include "zh-msg-priv.h"

#include <string.h>
#include <stdlib.h>

#define ZH_CRLF "\r\n"

/**
 * Initial size for message allocation
 */
#define ZH_MSG_INIT_SIZE 100

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

zh_msg_t * zh_msg_req(zh_method_t method, const char * url)
{
    return zh_msg_req_str(zh_method_to_str(method), url, "HTTP/1.1");
}

zh_msg_t * zh_msg_req_str(const char * method, const char * url, const char * httpv)
{
    if(method && url && httpv)
        return zh_msg_req_strn(method, strlen(method), url, strlen(url), httpv, strlen(httpv));

    return NULL;
}
zh_msg_t * zh_msg_req_strn( const char * method, size_t method_len,
                            const char * url, size_t url_len,
                            const char * httpv, size_t httpv_len)
{
    size_t new_size;
    void * data;
    zh_msg_t * msg = __msg_new(ZH_MSG_INIT_SIZE);

    if(!msg)
        goto fail;

    new_size = method_len + url_len + httpv_len + 4; //2 spaces + CRLF

    if(__msg_data_increase_by(msg, new_size))
        goto fail;

    data = msg->raw.data;

    /*Append Method*/
    msg->method.data = memcpy(data, method, method_len);
    msg->method.len = method_len;
    data += method_len;

    memcpy(data, " ", 1);
    data++;

    /*Append URL*/
    msg->url.data = memcpy(data, url, url_len);
    msg->url.len = url_len;
    data += url_len;

    memcpy(data, " ", 1);
    data++;

    /*Append URL*/
    msg->httpv.data = memcpy(data, httpv, httpv_len);
    msg->httpv.len = httpv_len;
    data += httpv_len;

    memcpy(data, ZH_CRLF, 2);
    data+=2;

    msg->header.data = memcpy(data, ZH_CRLF, 2);
    msg->header.len = 0;
    data+=2;

    msg->body.data = data;
    msg->body.len = 0;

    return msg;

fail :
    __msg_free(msg);
    return NULL;
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
    if(msg) free(msg);
}
