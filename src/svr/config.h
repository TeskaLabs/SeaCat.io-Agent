#ifndef TLSCA_SVR__CONFIG_H_
#define TLSCA_SVR__CONFIG_H_

struct sca_config
{
	const char * config_file;
	const char * config_dir;

	const char * application_id;
	const char * application_id_suffix;

	const char * var_dir;
	const char * cntl_socket_name;

	double keepalive_interval;
	double connecting_interval;
};

extern struct sca_config sca_config;

void sca_config_environ(void);
bool sca_config_load(void);

#endif // TLSCA_SVR__CONFIG_H_
