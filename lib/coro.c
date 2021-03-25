#include "coro.h"

#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>

struct coro
{
	void *udata;
	int status;
	void **ppass;
	ucontext_t context;
	void *(*func)(void *);
	struct coro *yield_to;
};

struct global_state
{
	int init;
	struct coro *current;
	struct coro toplevel;
};

#if CORO_USE_THREAD_LOCAL
_Thread_local
#else
static
#endif
struct global_state gs = {
	.init = 0,
};

static void init_global_state(void)
{
	gs.init = 1;
	gs.toplevel.udata = NULL;
	gs.toplevel.status = CORO_RUNNING;
	gs.toplevel.ppass = NULL;
	gs.toplevel.func = NULL;
	gs.toplevel.yield_to = NULL;
	gs.current = &gs.toplevel;
}

struct coro *coro_running(void)
{
	if (!gs.init)
	{
		init_global_state();
	}
	return gs.current;
}

struct coro *coro_toplevel(void)
{
	if (!gs.init)
	{
		init_global_state();
	}
	return &gs.toplevel;
}

static void transfer(struct coro *co, int status)
{
	co->status = status;
	assert(!swapcontext(&co->context, &co->yield_to->context));
}

static void wrap()
{
	struct coro *co = coro_running();
	void *ret = co->func(*(co->ppass));
	*(co->ppass) = ret;
	transfer(co, CORO_DEAD);
}

int coro_create(struct coro **pco, void *(*func)(void *), size_t stack_size)
{
	if (!pco)
	{
		return CORO_CREATE_ENULLPCO;
	}
	if (!func)
	{
		return CORO_CREATE_ENULLFN;
	}
	if (!stack_size)
	{
		return CORO_CREATE_ESTACKSZ;
	}
	void *stack = malloc(stack_size);
	if (!stack)
	{
		return CORO_CREATE_ENOMEM;
	}
	struct coro *co = malloc(sizeof(struct coro));
	if (!co)
	{
		free(stack);
		return CORO_CREATE_ENOMEM;
	}
	if (getcontext(&co->context))
	{
		free(co);
		free(stack);
		return CORO_CREATE_ESYS;
	}
	co->udata = NULL;
	co->status = CORO_SUSPENDED;
	co->context.uc_stack.ss_sp = stack;
	co->context.uc_stack.ss_size = stack_size;
	co->func = func;
	makecontext(&co->context, wrap, 0);
	*pco = co;
	return CORO_OK;
}

int coro_free(struct coro *co)
{
	if (!co)
	{
		return CORO_FREE_ENULLCO;
	}
	if (co->status != CORO_DEAD)
	{
		return CORO_FREE_ENOTDEAD;
	}
	free(co->context.uc_stack.ss_sp);
	free(co);
	return CORO_OK;
}

int coro_resume(struct coro *co, void **ppass)
{
	if (!co)
	{
		return CORO_RESUME_ENULLCO;
	}
	if (!ppass)
	{
		return CORO_RESUME_ENULLPPASS;
	}
	if (co->status != CORO_SUSPENDED)
	{
		return CORO_RESUME_ENOTSUSP;
	}
	co->status = CORO_RUNNING;
	co->ppass = ppass;
	co->yield_to = coro_running();
	gs.current = co;
	co->yield_to->status = CORO_NORMAL;
	int swap_ok = 1;
	if (swapcontext(&co->yield_to->context, &co->context))
	{
		swap_ok = 0;
	}
	co->yield_to->status = CORO_RUNNING;
	gs.current = co->yield_to;
	return swap_ok ? CORO_OK : CORO_RESUME_ESYS;
}

int coro_yield(void **ppass)
{
	if (!ppass)
	{
		return CORO_YIELD_ENULLPPASS;
	}
	struct coro *co = coro_running();
	if (co == coro_toplevel())
	{
		return CORO_YIELD_ETOPLVL;
	}
	*(co->ppass) = *ppass;
	transfer(co, CORO_SUSPENDED);
	*ppass = *(co->ppass);
	return CORO_OK;
}

int coro_status(struct coro *co)
{
	if (!co)
	{
		return CORO_STATUS_ENULLCO;
	}
	return co->status;
}

int coro_setudata(struct coro *co, void *udata)
{
	if (!co)
	{
		return CORO_SETUDATA_ENULLCO;
	}
	co->udata = udata;
	return CORO_OK;
}

int coro_getudata(struct coro *co, void **pudata)
{
	if (!co)
	{
		return CORO_GETUDATA_ENULLCO;
	}
	if (!pudata)
	{
		return CORO_GETUDATA_ENULLPUD;
	}
	*pudata = co->udata;
	return CORO_OK;
}
