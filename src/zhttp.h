#ifndef _ZHTTP_H
#define _ZHTTP_H

#include <zmq.h>

#include "zh-method.h" // HTTP Method Header
#include "zh-msg.h" // HTTP Message Header
#include "zh-srv.h" // HTTP Server Header
#include "zh-cli.h" // HTTP Client Header

/**
 * Initialize 0MQ Socket
 *
 * @param context 0MQ Context - See man 3 zmq_ctx_new
 *
 * @returns 0MQ Socket Suitable for HTTP transport.
 *          See :
 *              man 3 zmq_close
 *              man 3 zmq_setsockopt
 *              man 3 zmq_getsockopt
 */
static inline void * zhttp_socket(void * context)
{
    return zmq_socket(context, ZMQ_STREAM);
}

#endif
