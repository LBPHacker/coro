#include "coro.h"

#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>

struct coro
{
	void *ud;
	int st;
	void **pd;
	ucontext_t ctx;
	void *(*fn)(void *);
	struct coro *yt;
};

struct tlinfo
{
	struct coro *cur;
	void *ud;
	ucontext_t ctx;
};

_Thread_local struct tlinfo tli = {
	.cur = NULL,
	.ud = NULL,
};

struct coro *coro_running()
{
	return tli.cur;
}

static ucontext_t *ctx_running()
{
	struct coro *co = coro_running();
	if (co)
	{
		return &co->ctx;
	}
	return &tli.ctx;
}

static void yield_to(struct coro *co, int st)
{
	co->st = st;
	assert(!swapcontext(&co->ctx, co->yt ? &co->yt->ctx : &tli.ctx));
}

static void fn_wrap()
{
	struct coro *co = coro_running();
	void *ret = co->fn(*(co->pd));
	*(co->pd) = ret;
	yield_to(co, CORO_DEAD);
}

int coro_create(struct coro **co, void *(*fn)(void *), size_t stack_size)
{
	if (!co)
	{
		return CORO_CREATE_ENULLCO;
	}
	if (!fn)
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
	struct coro *ret = malloc(sizeof(struct coro));
	if (!ret)
	{
		free(stack);
		return CORO_CREATE_ENOMEM;
	}
	if (getcontext(&ret->ctx))
	{
		free(ret);
		free(stack);
		return CORO_CREATE_ESYS;
	}
	ret->ud = NULL;
	ret->st = CORO_SUSPENDED;
	ret->ctx.uc_stack.ss_sp = stack;
	ret->ctx.uc_stack.ss_size = stack_size;
	ret->fn = fn;
	makecontext(&ret->ctx, fn_wrap, 0);
	*co = ret;
	return CORO_OK;
}

int coro_free(struct coro *co)
{
	if (!co)
	{
		return CORO_FREE_ENULLCO;
	}
	if (co->st != CORO_DEAD)
	{
		return CORO_FREE_ENOTDEAD;
	}
	free(co->ctx.uc_stack.ss_sp);
	free(co);
	return CORO_OK;
}

int coro_resume(struct coro *co, void **pd)
{
	if (!co)
	{
		return CORO_RESUME_ENULLCO;
	}
	if (!pd)
	{
		return CORO_RESUME_ENULLPD;
	}
	if (co->st != CORO_SUSPENDED)
	{
		return CORO_RESUME_ENOTSUSP;
	}
	co->st = CORO_RUNNING;
	co->pd = pd;
	ucontext_t *run = ctx_running();
	struct coro *yt = tli.cur;
	co->yt = yt;
	tli.cur = co;
	int swap_ok = 1;
	if (swapcontext(run, &co->ctx))
	{
		swap_ok = 0;
	}
	tli.cur = yt;
	return swap_ok ? CORO_OK : CORO_RESUME_ESYS;
}

int coro_yield(void **pd)
{
	if (!pd)
	{
		return CORO_YIELD_ENULLPD;
	}
	struct coro *co = coro_running();
	if (!co)
	{
		return CORO_YIELD_ETOPLVL;
	}
	*(co->pd) = *pd;
	yield_to(co, CORO_SUSPENDED);
	*pd = *(co->pd);
	return CORO_OK;
}

int coro_status(struct coro *co)
{
	if (!co)
	{
		return coro_running() ? CORO_SUSPENDED : CORO_RUNNING;
	}
	return co->st;
}

void coro_setudata(struct coro *co, void *udata)
{
	if (!co)
	{
		tli.ud = udata;
		return;
	}
	co->ud = udata;
}

void *coro_getudata(struct coro *co)
{
	if (!co)
	{
		return tli.ud;
	}
	return co->ud;
}
