#ifndef _ZHTTP_MSG_H
#define _ZHTTP_MSG_H

/**
 * ZHTTP Message Handler
 *
 * This is an opaque handler and, accordingly,
 * members should never be addressed directly.
 */
typedef struct zhttp_msg zhttp_msg_t;

zhttp_msg_t * zhttp_msg_req_str(const char * method, const char * url, const char * httpv);

/**
 * Free a ZHTTP Request or Response
 */
void zhttp_msg_free(zhttp_msg_t * msg);

#endif
