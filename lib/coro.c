#include "coro.h"

#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>

#if CORO_USE_MMAP_STACK
# include <sys/mman.h>
# define stackmalloc(a) mmap(NULL, a, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN | MAP_STACK, 0, 0)
# define stackfree(a) assert(!munmap(a, 1))
#else
# define stackmalloc(a) malloc(a)
# define stackfree(a) free(a)
#endif

struct coro
{
	void *udata;
	int status;
	void **ppass;
	ucontext_t context;
	coro_func_t func;
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
	.toplevel = {
		.udata = NULL,
		.status = CORO_RUNNING,
	},
	.current = NULL,
};

struct coro *coro_running(void)
{
	return gs.current ? gs.current : &gs.toplevel;
}

struct coro *coro_toplevel(void)
{
	return &gs.toplevel;
}

static void start()
{
	struct coro *co = coro_running();
	co->status = CORO_RUNNING;
	void *ret = co->func(*(co->ppass));
	*(co->ppass) = ret;
	co->status = CORO_DEAD;
	assert(!setcontext(&co->yield_to->context));
}

int coro_create(struct coro **pco, coro_func_t func, size_t stack_size)
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
	void *stack = stackmalloc(stack_size);
	if (!stack)
	{
		return CORO_CREATE_ENOMEM;
	}
	struct coro *co = malloc(sizeof(struct coro));
	if (!co)
	{
		stackfree(stack);
		return CORO_CREATE_ENOMEM;
	}
	if (getcontext(&co->context))
	{
		free(co);
		stackfree(stack);
		return CORO_CREATE_ESYS;
	}
	co->udata = NULL;
	co->status = CORO_SUSPENDED;
	co->context.uc_stack.ss_sp = stack;
	co->context.uc_stack.ss_size = stack_size;
	co->func = func;
	makecontext(&co->context, start, 0);
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
	stackfree(co->context.uc_stack.ss_sp);
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
	co->ppass = ppass;
	co->yield_to = coro_running();
	gs.current = co;
	co->yield_to->status = CORO_NORMAL;
	assert(!swapcontext(&co->yield_to->context, &co->context));
	co->yield_to->status = CORO_RUNNING;
	gs.current = co->yield_to;
	return CORO_OK;
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
	co->status = CORO_SUSPENDED;
	assert(!swapcontext(&co->context, &co->yield_to->context));
	co->status = CORO_RUNNING;
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
