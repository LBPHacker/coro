#include "coro.h"

#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>

#if CORO_USE_MMAP_STACK
# include <sys/mman.h>

static void *stackmalloc(size_t size)
{
	void *mapped = mmap(NULL, a, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if (mapped == MAP_FAILED)
	{
		return NULL;
	}
	return mapped;
}

static void stackfree(void *a)
{
	assert(!munmap(a, 1));
}
#else
static void *stackmalloc(size_t size)
{
	return malloc(size);
}

static void stackfree(void *a)
{
	free(a);
}
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
	// * Can't just initialize current with &toplevel because gs might be
	//   thread-local, in which case its address is not a compile-time constant.
	return gs.current ? gs.current : &gs.toplevel;
}

struct coro *coro_toplevel(void)
{
	return &gs.toplevel;
}

static void start()
{
	struct coro *co = coro_running();
	void *pass = *(co->ppass);
	co->status = CORO_RUNNING;
	pass = co->func(pass);
	co->status = CORO_DEAD;
	*(co->ppass) = pass;
	assert(!setcontext(&co->yield_to->context));
}

int coro_create(struct coro **pco, coro_func_t func, size_t stack_size)
{
	if (pco == NULL)
	{
		return CORO_CREATE_ENULLPCO;
	}
	if (func == NULL)
	{
		return CORO_CREATE_ENULLFN;
	}
	if (!stack_size)
	{
		return CORO_CREATE_ESTACKSZ;
	}
	void *stack = stackmalloc(stack_size);
	if (stack == NULL)
	{
		return CORO_CREATE_ENOMEM;
	}
	struct coro *co = malloc(sizeof(struct coro));
	if (co == NULL)
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
	if (co == NULL)
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
	if (co == NULL)
	{
		return CORO_RESUME_ENULLCO;
	}
	if (ppass == NULL)
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
	if (ppass == NULL)
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
	if (co == NULL)
	{
		return CORO_STATUS_ENULLCO;
	}
	return co->status;
}

int coro_setudata(struct coro *co, void *udata)
{
	if (co == NULL)
	{
		return CORO_SETUDATA_ENULLCO;
	}
	co->udata = udata;
	return CORO_OK;
}

int coro_getudata(struct coro *co, void **pudata)
{
	if (co == NULL)
	{
		return CORO_GETUDATA_ENULLCO;
	}
	if (pudata == NULL)
	{
		return CORO_GETUDATA_ENULLPUD;
	}
	*pudata = co->udata;
	return CORO_OK;
}
