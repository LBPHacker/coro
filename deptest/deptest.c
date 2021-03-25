#include <stdio.h>
#include <assert.h>

#include "coro.h"

struct context
{
	int input;
};

static void *cofunc(void *data)
{
	struct context *pctx = (struct context *)data;
	pctx->input *= pctx->input;
	printf("hello from cofunc\n");
	// * There first parameter of coro_yield is similar to the parameter to
	//   the second parameter of coro_resume, see below.
	assert(!coro_yield((void **)&pctx));
	pctx->input += 42;
	return data;
}

static int func(int input)
{
	struct context ctx;
	struct context *pctx = &ctx;
	pctx->input = input;
	struct coro *co;
	// * All functions barring coro_status and coro_getudata return
	//   CORO_OK = 0 on success. See coro.h for other possible return values.
	// * Tell the library to allocate 0x1000 bytes to be used as stack
	//   by the new coroutine.
	assert(!coro_create(&co, cofunc, 0x1000U));
	printf("input is now %i\n", pctx->input);
	// * Pass a pointer to a pointer-sized area as the second parameter.
	//   This is necessary because not only do we want to send data to the
	//   coroutine, we also want to receive data from it.
	assert(!coro_resume(co, (void **)&pctx));
	printf("input is now %i\n", pctx->input);
	assert(!coro_resume(co, (void **)&pctx));
	printf("input is now %i\n", pctx->input);
	assert(!coro_free(co));
	return pctx->input;
}

int main(int argc, char *argv[])
{
	printf("result is %i\n", func(argc));
	return 0;
}
