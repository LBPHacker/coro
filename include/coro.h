#ifndef CORO_H_
#define CORO_H_

#include <stddef.h>

struct coro;

struct coro *coro_running();

#define CORO_OK               (     0)

#define CORO_RUNNING          (  1001)
#define CORO_SUSPENDED        (  1002)
#define CORO_DEAD             (  1003)

#define CORO_CREATE_ENULLCO   ( -2001)
#define CORO_CREATE_ENULLFN   ( -2002)
#define CORO_CREATE_ESTACKSZ  ( -2003)
#define CORO_CREATE_ENOMEM    ( -2004)
#define CORO_CREATE_ESYS      ( -2005)
int coro_create(struct coro **co, void *(*fn)(void *), size_t stack_size);

#define CORO_FREE_ENULLCO     ( -3001)
#define CORO_FREE_ENOTDEAD    ( -3002)
int coro_free(struct coro *co);

#define CORO_RESUME_ENULLPD   ( -4001)
#define CORO_RESUME_ENULLCO   ( -4002)
#define CORO_RESUME_ENOTSUSP  ( -4003)
#define CORO_RESUME_ESYS      ( -4004)
int coro_resume(struct coro *co, void **pd);

#define CORO_YIELD_ENULLPD    ( -5001)
#define CORO_YIELD_ETOPLVL    ( -5002)
int coro_yield(void **pd);

int coro_status(struct coro *co);

void coro_setudata(struct coro *co, void *udata);

void *coro_getudata(struct coro *co);

#endif
