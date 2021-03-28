#include "coro.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int hit = 0;

#define HITCHECK(k) if (hit != k) exit(k + 1); hit = k + 1;

static void f2()
{
	void *ret;
	HITCHECK(2);
	assert(!coro_yield(&ret));
}

static void *f1(void *data)
{
	void *ret;
	HITCHECK(0);
	assert(!coro_yield(&ret));
	f2();
	HITCHECK(4);
	return NULL;
}

int main(int argc, char *argv[])
{
	struct coro *co;
	void *ret;
	assert(!coro_create(&co, f1, 0x10000U));
	assert(!coro_resume(co, &ret));
	HITCHECK(1);
	assert(!coro_resume(co, &ret));
	HITCHECK(3);
	assert(!coro_resume(co, &ret));
	HITCHECK(5);
	assert(!coro_free(co));
	return 0;
}
