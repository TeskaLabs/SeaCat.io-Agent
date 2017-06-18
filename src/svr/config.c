#include "all.h"


struct sca_config sca_config = 
{
	.config_file = SEACATIO_CONFIG_FILE,

	.application_id = "io.seacat.agent",
	.application_id_suffix = NULL, //TODO: Allow to configure this one

	.var_dir = SEACATIO_PREFIX "/var", //TODO: This is from configuration
	.cntl_socket_name = SEACATIO_CNTL_SOCKET_NAME,

	//DOCU: [seacatio] keepalive_interval
	.keepalive_interval = 120.0, //Configure by [seacatio] keepalive_interval
	//DOCU: [seacatio] keepalive_interval
	.connecting_interval = 30.0, //Configure by [seacatio] connecting_interval
};

///

static int sca_config_parse_handler(void * user, const char * section, const char * name, const char * value)
{
	if (false) {}

	else if (FT_INI_MATCH("seacatio", "suffix"))
	{
		sca_config.application_id_suffix = strdup(value);
		return 1;
	}

	else if (FT_INI_MATCH("seacatio", "keepalive_interval"))
	{
		sca_config.keepalive_interval = atof(value);
	}

	else if (FT_INI_MATCH("seacatio", "connecting_interval"))
	{
		sca_config.connecting_interval = atof(value);
	}

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
