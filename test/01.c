#include "coro.h"
#include "test.h"

#include <stdint.h>
#include <stdlib.h>

static struct coro *co;

static void *nothing(void *data)
{
	EXPECT(coro_running() == co);
	void *ret;
	EXPECT(coro_resume(co, &ret) == CORO_RESUME_ENOTSUSP);
	EXPECT(coro_status(coro_toplevel()) == CORO_NORMAL);
	EXPECT(coro_status(co) == CORO_RUNNING);
	return NULL;
}

int main(int argc, const char *argv[])
{
	EXPECT(coro_running() == coro_toplevel());
	EXPECT(coro_create(NULL, NULL, 0x0U) == CORO_CREATE_ENULLPCO);
	EXPECT(coro_create(&co, NULL, 0x0U) == CORO_CREATE_ENULLFN);
	EXPECT(coro_create(&co, nothing, 0x0U) == CORO_CREATE_ESTACKSZ);
	EXPECT(coro_create(&co, nothing, 0x10000U) == CORO_OK);
	EXPECT(coro_status(coro_toplevel()) == CORO_RUNNING);
	EXPECT(coro_status(co) == CORO_SUSPENDED);
	EXPECT(coro_resume(NULL, NULL) == CORO_RESUME_ENULLCO);
	EXPECT(coro_resume(co, NULL) == CORO_RESUME_ENULLPPASS);
	void *ret;
	EXPECT(coro_free(co) == CORO_FREE_ENOTDEAD);
	EXPECT(coro_resume(co, &ret) == CORO_OK);
	EXPECT(coro_running() == coro_toplevel());
	EXPECT(coro_free(NULL) == CORO_FREE_ENULLCO);
	EXPECT(coro_getudata(NULL, NULL) == CORO_GETUDATA_ENULLCO);
	EXPECT(coro_getudata(coro_toplevel(), NULL) == CORO_GETUDATA_ENULLPUD);
	intptr_t udata;
	EXPECT(coro_getudata(coro_toplevel(), (void **)&udata) == CORO_OK);
	EXPECT(udata == 0);
	EXPECT(coro_setudata(NULL, (void *)udata) == CORO_SETUDATA_ENULLCO);
	udata = 8;
	EXPECT(coro_setudata(coro_toplevel(), (void *)udata) == CORO_OK);
	EXPECT(coro_getudata(coro_toplevel(), (void **)&udata) == CORO_OK);
	EXPECT(udata == 8);
	EXPECT(coro_status(co) == CORO_DEAD);
	EXPECT(coro_free(co) == CORO_OK);
	return 0;
}
