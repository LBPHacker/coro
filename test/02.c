#include "coro.h"
#include "test.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

static struct coro *co1;
static struct coro *co2;

static void *inc_thrice(void *data)
{
	EXPECT(coro_running() == co2);
	intptr_t val = (intptr_t)data;
	val += 1;
	assert(!coro_yield((void **)&val));
	EXPECT(coro_running() == co2);
	val += 1;
	intptr_t udata;
	assert(!coro_getudata(co2, (void **)&udata));
	EXPECT(udata == 7);
	assert(!coro_yield((void **)&val));
	EXPECT(coro_running() == co2);
	val += 1;
	return (void *)val;
}

static void *inc_twice(void *data)
{
	EXPECT(coro_running() == co1);
	intptr_t val = (intptr_t)data;
	val += 1;
	assert(!coro_create(&co2, inc_thrice, 0x10000U));
	assert(!coro_resume(co2, (void **)&val));
	EXPECT(coro_running() == co1);
	EXPECT(val == 10);
	EXPECT(coro_yield(NULL) == CORO_YIELD_ENULLPPASS);
	assert(!coro_yield((void **)&val));
	EXPECT(coro_running() == co1);
	assert(!coro_resume(co2, (void **)&val));
	EXPECT(coro_running() == co1);
	EXPECT(val == 12);
	val += 1;
	assert(!coro_free(co2));
	return (void *)val;
}

int main(int argc, const char *argv[])
{
	EXPECT(coro_running() == coro_toplevel());
	assert(!coro_create(&co1, inc_twice, 0x10000U));
	intptr_t val = 8;
	assert(!coro_resume(co1, (void **)&val));
	EXPECT(coro_running() == coro_toplevel());
	EXPECT(val == 10);
	void *udata;
	EXPECT(coro_getudata(co2, &udata) == CORO_OK);
	EXPECT(udata == NULL);
	coro_setudata(co2, (void *)(intptr_t)7);
	assert(!coro_resume(co2, (void **)&val));
	EXPECT(coro_running() == coro_toplevel());
	EXPECT(val == 11);
	assert(!coro_resume(co1, (void **)&val));
	EXPECT(coro_running() == coro_toplevel());
	EXPECT(val == 13);
	assert(!coro_free(co1));
	EXPECT(coro_yield((void **)&val) == CORO_YIELD_ETOPLVL);
	EXPECT(coro_status(NULL) == CORO_STATUS_ENULLCO);
	return 0;
}
