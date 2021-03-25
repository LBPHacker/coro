project('coro', 'c', version: '0.1')

coro_inc = include_directories('include')
coro_lib = library('coro', sources: 'lib/coro.c', include_directories: coro_inc, install: true)

subdir('include')

coro_dep = declare_dependency(
	link_with: coro_lib,
	include_directories: coro_inc,
)

test('test01', executable('test01', sources: files('test/01.c'), dependencies: coro_dep))
test('test02', executable('test02', sources: files('test/02.c'), dependencies: coro_dep))
test('test03', executable('test03', sources: files('test/03.c'), dependencies: coro_dep))

import('pkgconfig').generate(
	libraries: coro_lib,
	subdirs: '.',
	name: 'libcoro',
	filebase: 'coro',
	description: 'A very simple coroutine library for C based on getcontext',
)
