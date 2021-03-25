#include "coro.h"

#include <stdint.h>
#include <stdlib.h>

struct coro *co;

static void *nothing(void *data)
{
	if (coro_running() != co)
	{
		exit(13);
	}
	void *ret;
	if (coro_resume(co, &ret) != CORO_RESUME_ENOTSUSP)
	{
		exit(14);
	}
	if (coro_status(coro_toplevel()) != CORO_NORMAL)
	{
		exit(15);
	}
	if (coro_status(co) != CORO_RUNNING)
	{
		exit(16);
	}
	return NULL;
}

int main(int argc, const char *argv[])
{
	if (coro_running() != coro_toplevel())
	{
		exit(1);
	}
	if (coro_create(NULL, NULL, 0x0U) != CORO_CREATE_ENULLPCO)
	{
		exit(2);
	}
	if (coro_create(&co, NULL, 0x0U) != CORO_CREATE_ENULLFN)
	{
		exit(3);
	}
	if (coro_create(&co, nothing, 0x0U) != CORO_CREATE_ESTACKSZ)
	{
		exit(4);
	}
	if (coro_create(&co, nothing, 0x1000U) != CORO_OK)
	{
		exit(5);
	}
	if (coro_status(coro_toplevel()) != CORO_RUNNING)
	{
		exit(6);
	}
	if (coro_status(co) != CORO_SUSPENDED)
	{
		exit(7);
	}
	if (coro_resume(NULL, NULL) != CORO_RESUME_ENULLCO)
	{
		exit(8);
	}
	if (coro_resume(co, NULL) != CORO_RESUME_ENULLPPASS)
	{
		exit(9);
	}
	void *ret;
	if (coro_free(co) != CORO_FREE_ENOTDEAD)
	{
		exit(10);
	}
	if (coro_resume(co, &ret) != CORO_OK)
	{
		exit(11);
	}
	if (coro_running() != coro_toplevel())
	{
		exit(12);
	}
	if (coro_free(NULL) != CORO_FREE_ENULLCO)
	{
		exit(17);
	}
	if (coro_getudata(NULL, NULL) != CORO_GETUDATA_ENULLCO)
	{
		exit(18);
	}
	if (coro_getudata(coro_toplevel(), NULL) != CORO_GETUDATA_ENULLPUD)
	{
		exit(22);
	}
	intptr_t udata;
	if (coro_getudata(coro_toplevel(), (void **)&udata) != CORO_OK)
	{
		exit(23);
	}
	if (udata != 0)
	{
		exit(24);
	}
	if (coro_setudata(NULL, (void *)udata) != CORO_SETUDATA_ENULLCO)
	{
		exit(27);
	}
	udata = 8;
	if (coro_setudata(coro_toplevel(), (void *)udata) != CORO_OK)
	{
		exit(26);
	}
	if (coro_getudata(coro_toplevel(), (void **)&udata) != CORO_OK)
	{
		exit(19);
	}
	if (udata != 8)
	{
		exit(25);
	}
	if (coro_status(co) != CORO_DEAD)
	{
		exit(20);
	}
	if (coro_free(co) != CORO_OK)
	{
		exit(21);
	}
	return 0;
}
