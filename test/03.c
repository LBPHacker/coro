#include "coro.h"
#include "test.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

static int hit = 0;

#define HITCHECK(k) do { EXPECT(hit == k); hit = k + 1; } while (0)

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
