#ifndef TLSCA_SVR__APP_H_
#define TLSCA_SVR__APP_H_

struct sca_app
{
	struct ft_context context;

	struct ft_subscriber exit_subscriber;
	struct ev_prepare prepare_w;
	struct ev_timer killer_timer_w;

	char state; // 'i' - init, 'r' - running , 'e' - exiting

	// C-Core related components
	pthread_t seacatcc_thread; // SeaCat C-Core thread
	int seacatcc_thread_rc;
	struct ev_async seacatcc_async_w;
	struct ev_check seacatcc_check_w;
	pthread_mutex_t seacatcc_loop_lock;
	struct ft_frame * seacatcc_write_buffer;
	struct ft_frame * seacatcc_read_buffer;
	char seacatcc_state[SEACATCC_STATE_BUF_SIZE];
	struct ft_frame * seacatcc_write_queue;
	struct ft_frame ** seacatcc_write_queue_last;

	// Control socket
	struct ft_list cntl_listeners_list;
	struct ft_list cntl_list;

	// Connectivity
	struct sca_connectivity connectivity;

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

static inline void sca_loop_lock_acquire(void)
{
	pthread_mutex_lock(&sca_app.seacatcc_loop_lock);
	ev_now_update(sca_app.context.ev_loop);
}

static inline void sca_loop_lock_release(void)
{
	pthread_mutex_unlock(&sca_app.seacatcc_loop_lock);
}

#endif //TLSCA_SVR__APP_H_
