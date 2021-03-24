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
	if (coro_status(NULL) != CORO_SUSPENDED)
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
	if (coro_running() != NULL)
	{
		exit(1);
	}
	if (coro_create(NULL, NULL, 0x0U) != CORO_CREATE_ENULLCO)
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
	if (coro_status(NULL) != CORO_RUNNING)
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
	if (coro_resume(co, NULL) != CORO_RESUME_ENULLPD)
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
	if (coro_running() != NULL)
	{
		exit(12);
	}
	if (coro_free(NULL) != CORO_FREE_ENULLCO)
	{
		exit(17);
	}
	if (coro_getudata(NULL) != NULL)
	{
		exit(18);
	}
	coro_setudata(NULL, (void *)(intptr_t)(8));
	if ((intptr_t)coro_getudata(NULL) != 8)
	{
		exit(19);
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
