#ifndef CORO_H_
#define CORO_H_

#include "coroconf.h"

#include <stddef.h>

struct coro;

typedef void *(*coro_func_t)(void *);

enum coro_result
{
	CORO_OK                 =     0,
	CORO_RUNNING            =  1001,
	CORO_NORMAL             =  1002,
	CORO_SUSPENDED          =  1003,
	CORO_DEAD               =  1004,
	CORO_CREATE_ENULLPCO    = -2001,
	CORO_CREATE_ENULLFN     = -2002,
	CORO_CREATE_ESTACKSZ    = -2003,
	CORO_CREATE_ENOMEM      = -2004,
	CORO_CREATE_ESYS        = -2005,
	CORO_FREE_ENULLCO       = -3001,
	CORO_FREE_ENOTDEAD      = -3002,
	CORO_RESUME_ENULLPPASS  = -4001,
	CORO_RESUME_ENULLCO     = -4002,
	CORO_RESUME_ENOTSUSP    = -4003,
	CORO_YIELD_ENULLPPASS   = -5001,
	CORO_YIELD_ETOPLVL      = -5002,
	CORO_STATUS_ENULLCO     = -6001,
	CORO_SETUDATA_ENULLCO   = -7001,
	CORO_GETUDATA_ENULLPUD  = -8001,
	CORO_GETUDATA_ENULLCO   = -8002,
};

int coro_create(struct coro **pco, coro_func_t func, size_t stack_size);
int coro_free(struct coro *co);
int coro_resume(struct coro *co, void **ppass);
int coro_yield(void **ppass);
int coro_status(struct coro *co);
int coro_setudata(struct coro *co, void *udata);
int coro_getudata(struct coro *co, void **pudata);
struct coro *coro_running(void);
struct coro *coro_toplevel(void);

#endif
