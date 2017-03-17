#ifndef APP_SOCKS__APP_H_
#define APP_SOCKS__APP_H_

struct sca_app
{
	struct ft_context context;

	struct
	{
	} stats;

	struct
	{
	} flags;

};

extern struct sca_app sca_app;

bool sca_app_init(struct sca_app *);
void sca_app_fini(struct sca_app *);

int sca_app_run(struct sca_app *);

#endif //APP_SOCKS__APP_H_
