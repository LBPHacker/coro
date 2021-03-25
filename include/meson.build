coroconf_conf = configuration_data()

coroconf_conf.set('CORO_USE_THREAD_LOCAL', get_option('use_thread_local') ? 1 : 0)

configure_file(
	input: 'coroconf.template.h',
	output: 'coroconf.h',
	configuration: coroconf_conf
)
