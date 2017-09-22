#ifndef _ZH_METHOD_H
#define _ZH_METHOD_H

/**
 * Enumerated HTTP Methods
 */
typedef enum {
    ZH_UNKNOWN_METHOD = -1, /**< Unknown method*/
    ZH_GET,     /**< "GET" method*/
    ZH_POST,    /**< "POST" method*/
    ZH_HEAD,    /**< "HEAD" method*/
    ZH_PUT,     /**< "PUT" method*/
    ZH_DELETE,  /**< "DELETE" method*/
    ZH_CONNECT, /**< "CONNECT" method*/
    ZH_OPTIONS, /**< "OPTIONS" method*/
    ZH_TRACE,   /**< "TRACE" method*/
    ZH_PATCH,   /**< "PATCH" method*/
} zh_method_t;

/**
 * Get enumerated value of string method
 *
 * @param method    String HTTP Method.
 *                  MUST be NULL-terminated. If the
 *                  string may not be NULL-terminated,
 *                  use zh_method_from_strn().
 * @returns The corresponding enumerated method or
 *          ZH_UNKNOWN_METHOD of the method is unknown.
 */
zh_method_t zh_method_from_str(const char * method);

/**
 * Get enumerated value of string method
 *
 * @param method        String HTTP Method.
 * @param method_len    The length of the string to be evaluated.
 * @returns The corresponding enumerated method or
 *          ZH_METHOD_UNKNOWN of the method is unknown.
 */
zh_method_t zh_method_from_strn(const char * method, size_t method_len);

/**
 * Get string value of enumerated method
 *
 * @param method        String HTTP Method.
 * @param method_len    The length of the string to be evaluated.
 * @returns The corresponding enumerated method or
 *          ZH_METHOD_UNKNOWN of the method is unknown.
 */
const char * zh_method_to_str(zh_method_t method);

#endif
