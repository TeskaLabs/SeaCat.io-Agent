#include "all.h"


struct sca_config sca_config = 
{
	.config_file = LIBTLSCA_CONFIG_FILE,
};

///

static int sca_config_parse_handler(void * user, const char * section, const char * name, const char * value)
{
	if (false) {}


	return 0;  // unknown section/name, error
}

///

void sca_config_environ()
{
	char * c;

	c = getenv("SEACAT_CONFIG");
	if (c != NULL) sca_config.config_file = c;
}


bool sca_config_load()
{
	if (sca_config.config_file == NULL) return true;

	FT_DEBUG("Loading configuration from '%s'", sca_config.config_file);
	if (ini_parse(sca_config.config_file, sca_config_parse_handler, &sca_config) < 0)
	{
		FT_ERROR("Can't load '%s'", sca_config.config_file);
		return false;
	}

	return true;
}
