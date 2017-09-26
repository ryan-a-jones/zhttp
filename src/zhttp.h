#ifndef _ZHTTP_H
#define _ZHTTP_H

#include <zmq.h>

/**
 * ZHTTP Handler
 *
 * An opaque structure for handling
 */
typedef struct zhttp zhttp_t;

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

/**
 * Create a new ZHTTP instance
 *
 * @param   socket  0MQ Socket
 *
 * @returns New ZHTTP handle or NULL on failure
 *          Free with call to zhttp_destroy()
 */
zhttp_t * zhttp_new(void * socket);

/**
 * Free ZHTTP resources
 *
 * @param zh ZHTTP handle
 */
void zhttp_destroy(zhttp_t * zh);

/**
 * ZHTTP return types
 */
typedef enum {
    ZH_EV_ERROR    = -1, /**< A fatal error has occurred*/
    ZH_EV_OK       =  0, /**< The event returned and handled request data*/
    ZH_EV_PASS     =  1, /**< The event was handled without error but did
                              not process request data*/
} zhttp_ev_ret_t;

/**
 * Event Handler
 *
 * @returns Status of event handling.
 *          If ZH_EV_ERROR is returned, the event loop exits with error.
 *          If ZH_EV_OK is returned, all other handlers are skipped for this event.
 *          If ZH_EV_PASS is returned, all other handlers are processed.
 */
typedef zhttp_ev_ret_t (* zhttp_ev_fun_t)(const zh_msg_t * msg, void * data);

/**
 * Free function handler
 *
 * @param data  Data to be freed
 */
typedef void (*zhttp_free_fun_t)(void * data);

/**
 * Register an event handler
 *
 * @param zh    ZHTTP handle
 * @param ev    Event handler
 * @param free  Function to free data when zh is destroyed.
 *              May be set to NULL if unneeded.
 * @param data  User data to be passed to handle
 */
int zhttp_ev_reg(zhttp_t * zh, zhttp_ev_fun_t ev, zhttp_free_fun_t free, void * data);

/**
 * Receive a new HTTP message
 *
 * @param zh        ZHTTP handle
 * @param zmq_flags Flags to pass to 0MQ
 *
 */
int zhttp_recv(zhttp_t * zh, int zmq_flags);

/**
 * Send an HTTP message
 *
 * @param msg HTTP Message to Send
 */
int zhttp_send(zh_msg_t * msg);

#endif
