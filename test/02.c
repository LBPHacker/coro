#include "coro.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

struct coro *co1;
struct coro *co2;

static void *inc_thrice(void *data)
{
	if (coro_running() != co2)
	{
		exit(3);
	}
	intptr_t val = (intptr_t)data;
	val += 1;
	assert(!coro_yield((void **)&val));
	if (coro_running() != co2)
	{
		exit(10);
	}
	val += 1;
	if ((intptr_t)coro_getudata(co2) != 7)
	{
		exit(11);
	}
	assert(!coro_yield((void **)&val));
	if (coro_running() != co2)
	{
		exit(15);
	}
	val += 1;
	return (void *)val;
}

static void *inc_twice(void *data)
{
	if (coro_running() != co1)
	{
		exit(2);
	}
	intptr_t val = (intptr_t)data;
	val += 1;
	assert(!coro_create(&co2, inc_thrice, 0x1000U));
	assert(!coro_resume(co2, (void **)&val));
	if (coro_running() != co1)
	{
		exit(4);
	}
	if (val != 10)
	{
		exit(5);
	}
	if (coro_yield(NULL) != CORO_YIELD_ENULLPD)
	{
		exit(6);
	}
	assert(!coro_yield((void **)&val));
	if (coro_running() != co1)
	{
		exit(14);
	}
	assert(!coro_resume(co2, (void **)&val));
	if (coro_running() != co1)
	{
		exit(16);
	}
	if (val != 12)
	{
		exit(17);
	}
	val += 1;
	assert(!coro_free(co2));
	return (void *)val;
}

int main(int argc, const char *argv[])
{
	if (coro_running() != NULL)
	{
		exit(1);
	}
	assert(!coro_create(&co1, inc_twice, 0x1000U));
	intptr_t val = 8;
	assert(!coro_resume(co1, (void **)&val));
	if (coro_running() != NULL)
	{
		exit(7);
	}
	if (val != 10)
	{
		exit(8);
	}
	if (coro_getudata(co2) != NULL)
	{
		exit(9);
	}
	coro_setudata(co2, (void *)(intptr_t)7);
	assert(!coro_resume(co2, (void **)&val));
	if (coro_running() != NULL)
	{
		exit(12);
	}
	if (val != 11)
	{
		exit(13);
	}
	assert(!coro_resume(co1, (void **)&val));
	if (coro_running() != NULL)
	{
		exit(18);
	}
	if (val != 13)
	{
		exit(19);
	}
	assert(!coro_free(co1));
	if (coro_yield((void **)&val) != CORO_YIELD_ETOPLVL)
	{
		exit(20);
	}
	return 0;
}