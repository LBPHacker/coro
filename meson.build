project('coro', 'c')

coro_inc = include_directories('include')
coro_lib = library('coro', sources: 'lib/coro.c', include_directories: coro_inc)

coro_dep = declare_dependency(
	link_with: coro_lib,
	include_directories: coro_inc,
)

test('test01', executable('test01', sources: files('test/01.c'), dependencies: coro_dep))
test('test02', executable('test02', sources: files('test/02.c'), dependencies: coro_dep))
test('test03', executable('test03', sources: files('test/03.c'), dependencies: coro_dep))
