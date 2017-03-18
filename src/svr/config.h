#ifndef APP_SOCKS__CONFIG_H_
#define APP_SOCKS__CONFIG_H_

struct sca_config
{
	const char * config_file;

	const char * application_id;
	const char * application_id_suffix;

	const char * var_dir;
};

extern struct sca_config sca_config;

void sca_config_environ(void);
bool sca_config_load(void);

#endif // APP_SOCKS__CONFIG_H_
