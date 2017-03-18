#ifndef TLSCA_SVR__APP_H_
#define TLSCA_SVR__APP_H_

struct sca_app
{
	struct ft_context context;

	struct ft_subscriber exit_subscriber;

	// C-Core related components
	pthread_t seacatcc_thread; // SeaCat C-Core thread
	int seacatcc_thread_rc;
	struct ev_async seacatcc_async_w;
	struct ev_check seacatcc_check_w;

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

#endif //TLSCA_SVR__APP_H_
