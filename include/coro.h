#ifndef CORO_H_
#define CORO_H_

#include "coroconf.h"

#include <stddef.h>

struct coro;

#define CORO_OK                 (     0)

#define CORO_RUNNING            (  1001)
#define CORO_NORMAL             (  1002)
#define CORO_SUSPENDED          (  1003)
#define CORO_DEAD               (  1004)

#define CORO_CREATE_ENULLPCO    ( -2001)
#define CORO_CREATE_ENULLFN     ( -2002)
#define CORO_CREATE_ESTACKSZ    ( -2003)
#define CORO_CREATE_ENOMEM      ( -2004)
#define CORO_CREATE_ESYS        ( -2005)
int coro_create(struct coro **pco, void *(*func)(void *), size_t stack_size);

#define CORO_FREE_ENULLCO       ( -3001)
#define CORO_FREE_ENOTDEAD      ( -3002)
int coro_free(struct coro *co);

#define CORO_RESUME_ENULLPPASS  ( -4001)
#define CORO_RESUME_ENULLCO     ( -4002)
#define CORO_RESUME_ENOTSUSP    ( -4003)
#define CORO_RESUME_ESYS        ( -4004)
int coro_resume(struct coro *co, void **ppass);

#define CORO_YIELD_ENULLPPASS   ( -5001)
#define CORO_YIELD_ETOPLVL      ( -5002)
int coro_yield(void **ppass);

#define CORO_STATUS_ENULLCO     ( -6001)
int coro_status(struct coro *co);

#define CORO_SETUDATA_ENULLCO   ( -7001)
int coro_setudata(struct coro *co, void *udata);

#define CORO_GETUDATA_ENULLPUD  ( -8001)
#define CORO_GETUDATA_ENULLCO   ( -8002)
int coro_getudata(struct coro *co, void **pudata);

struct coro *coro_running(void);

struct coro *coro_toplevel(void);

#endif
