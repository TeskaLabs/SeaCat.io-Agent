#include "all.h"

///

bool sca_app_init(struct sca_app * this)
{
	bool ok;
	ASSERT_THIS();

	ft_initialise();

	ok = ft_context_init(&this->context);
	if (!ok) return false;

	ok = sca_config_load();
	if (!ok) return false;

	return true;
}

void sca_app_fini(struct sca_app * this)
{
	ASSERT_THIS();

	ft_context_fini(&this->context);
}

///

int sca_app_run(struct sca_app * this)
{
	ASSERT_THIS();

	ft_context_run(&this->context);

	return EXIT_SUCCESS;
}
