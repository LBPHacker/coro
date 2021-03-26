coroconf_conf = configuration_data()

coroconf_conf.set('CORO_USE_THREAD_LOCAL', get_option('use_thread_local') ? 1 : 0)
coroconf_conf.set('CORO_USE_MMAP_STACK',   get_option('use_mmap_stack')   ? 1 : 0)

coroconf_h = configure_file(
	input: 'coroconf.template.h',
	output: 'coroconf.h',
	configuration: coroconf_conf
)

install_headers([ 'coro.h', coroconf_h ])
