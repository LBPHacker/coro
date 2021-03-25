# Coro

Or: _as if we don't have enough things to worry about_

## What (is all this)?

A very simple coroutine library for C (similar to
[Lua's](https://www.lua.org/manual/5.1/manual.html#2.11), except there's no
GC involved so you have to clean up after yourself). Currently heavily depends
on the [getcontext](https://linux.die.net/man/3/getcontext) family of
functions, which seem to have been phased out of POSIX, very sad.

This library is not thread-safe, which shouldn't come as a surprise if you're
trying to use it (seeing as it's a coroutine library). Regardless, you can
configure (see below) the library to make its global state `_Thread_local`
rather than `static` if you so wish, and if your compiler supports it.

## Why (would you do this)?

I just really wanted to get the itch to write some sort of coroutine library
for C out of my system.

## When (should I use this)?

Never, unless you're sure that the rest of your program can handle the sort
of context transfers
[setcontext](https://linux.die.net/man/3/getcontext) does. Especially not when
using C++; this library will eat your exceptions and RAII for breakfast.

Also don't use this if you can't guarantee that you won't overflow the stack
allocated for your coroutines; see the code below. If you can't guarantee
this but still would like to use the library, consider building (see way below)
with `-Duse_mmap_stack=true` to have the library ask Linux for stacks that
reliably crash your program with a `SIGSEGV` when overflowed.

## Who (should I blame if my program dies)?

Yourself, see the [license](LICENSE.md).

## How (would I use this)?

The same way you'd do it in Lua. See [coro.h](include/coro.h) and
[03.c](test/03.c). But [tl;dr](deptest/deptest.c):

```c
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

```

Build and install with [Meson](https://mesonbuild.com/) and
[Ninja](https://ninja-build.org/) like so:

```sh
cd coro
meson build
cd build
ninja test
sudo ninja install
```

If you wish to make the global state thread-local, configure the build site
with `use_thread_local` set to `true`:

```sh
cd coro
meson -Duse_thread_local=true build
cd build
ninja test
sudo ninja install
```

The Meson script exports the dependency `coro_dep`. Installing also creates a
`coro.pc` [pkg-config](https://linux.die.net/man/1/pkg-config) file, which lets
you link against this dependency with other build systems, and with
`dependency('coro')` in Meson.

You can uninstall the library with:

```sh
sudo ninja uninstall
```
