#include "all.h"

///

static void sca_on_exit(struct ft_subscriber *, struct ft_pubsub * pubsub, const char * topic, void * data);

static void * sca_app_seacatcc_thread(void * data);
static void sca_app_on_seacatcc_check(struct ev_loop * loop, ev_check *w, int revents);
static void sca_app_on_seacatcc_async(struct ev_loop * loop, ev_async * w, int revents);

static void sca_app_on_prepare(struct ev_loop * loop, ev_prepare * w, int revents);

static void sca_app_loop_release(struct ev_loop * loop);
static void sca_app_loop_acquire(struct ev_loop * loop);

///

bool sca_app_init(struct sca_app * this)
{
	bool ok;
	int rc;

	ASSERT_THIS();

	this->state = 'i';

	this->seacatcc_write_buffer = NULL;
	this->seacatcc_read_buffer = NULL;
	this->seacatcc_write_queue = NULL;
	this->seacatcc_write_queue_last = &this->seacatcc_write_queue;

	if (ft_config.log_verbose)
	{
//		ft_config.log_trace_mask |= FT_TRACE_ID_MEMPOOL;
	}

	ft_initialise();

	FT_INFO("SeaCat.io Agent %s started", SEACAT_VERSION);

	ok = ft_context_init(&this->context);
	if (!ok) return false;

	ok = sca_config_load();
	if (!ok) return false;

	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&this->seacatcc_loop_lock, &mutex_attr);
	ev_set_loop_release_cb(this->context.ev_loop, sca_app_loop_release, sca_app_loop_acquire);

	// Prepare listening control socket(s)
	ft_listener_list_init(&this->cntl_listeners_list);

	rc = ft_listener_list_extend(&this->cntl_listeners_list, &sca_cntl_listener_delegate, &this->context, AF_UNIX, SOCK_STREAM, sca_config.cntl_socket_name, "");
	if (rc < 0) exit(EXIT_FAILURE);

	FT_LIST_FOR(&this->cntl_listeners_list, node)
	{
		struct ft_listener * l = (struct ft_listener *)&node->data;
		l->base.socket.data = this;
	}

	ft_list_init(&this->cntl_list, sca_cntl_on_remove);

	if (ft_config.log_verbose)
	{
		union seacatcc_log_mask_u mask = {.value = 0 };
		mask.DEBUG_GENERIC = true;
		seacatcc_log_set_mask(mask);
	}

	sca_reactor_init();

	this->seacatcc_thread_rc = -1;

	ev_async_init(&this->seacatcc_async_w, sca_app_on_seacatcc_async);
	ev_async_start(this->context.ev_loop, &this->seacatcc_async_w);
	this->seacatcc_async_w.data = this;
	ev_unref(this->context.ev_loop);

	ev_check_init(&this->seacatcc_check_w, sca_app_on_seacatcc_check);
	ev_check_start(this->context.ev_loop, &this->seacatcc_check_w);
	this->seacatcc_check_w.data = this;

	ev_prepare_init(&this->prepare_w, sca_app_on_prepare);
	ev_prepare_start(this->context.ev_loop, &this->prepare_w);
	this->prepare_w.data = this;
	ev_unref(this->context.ev_loop);

	ft_subscriber_init(&this->exit_subscriber, sca_on_exit);
	ft_subscriber_subscribe(&this->exit_subscriber, NULL, FT_PUBSUB_TOPIC_EXIT);
	this->exit_subscriber.data = this;

	ok = sca_connectivity_init(&this->connectivity);
	if (!ok) exit(EXIT_FAILURE);

	// Start listening
	ft_listener_list_cntl(&this->cntl_listeners_list, FT_LISTENER_START);

	// Configure port forwarding
	rc = seacatcc_socket_configure_worker(22, AF_INET, SOCK_STREAM, 0, "127.0.0.1", "22");
	if (rc != SEACATCC_RC_OK)
	{
		FT_ERROR("seacatcc_socket_configure_worker failed: %d", rc);
	}

	return true;
}

void sca_app_fini(struct sca_app * this)
{
	ASSERT_THIS();

	sca_connectivity_fini(&this->connectivity);
	ft_subscriber_fini(&this->exit_subscriber);

	ft_list_fini(&this->cntl_listeners_list);
	ft_list_fini(&this->cntl_list);

	pthread_mutex_destroy(&this->seacatcc_loop_lock);

	ft_context_fini(&this->context);
}

///

int sca_app_run(struct sca_app * this)
{
	void * p;
	int rc;

	ASSERT_THIS();

	// Start keep-alive ping
	// if (this->keepalive_w.repeat > 0.0)
	// {
	// 	ev_timer_again(this->context.ev_loop, &this->keepalive_w);
	// 	ev_invoke(this->context.ev_loop, &this->keepalive_w, 0);
	// }
	this->state = 'r';

	pthread_create(&this->seacatcc_thread, NULL, sca_app_seacatcc_thread, this);

	ft_context_run(&this->context);

	rc = pthread_join(this->seacatcc_thread, &p);
	if (rc != 0)
	{
		FT_ERROR_ERRNO(rc, "pthread_join()");
	}

	return EXIT_SUCCESS;
}

///

void * sca_app_seacatcc_thread(void * data)
{
	struct sca_app * this = data;
	ASSERT_THIS();

	this->seacatcc_thread_rc = seacatcc_run();
	if (this->seacatcc_thread_rc != SEACATCC_RC_OK)
	{
		fprintf(stderr, "Error in seacatcc_run:%d\n", this->seacatcc_thread_rc);
	}

	// Push main event loop
	ev_async_send(this->context.ev_loop, &this->seacatcc_async_w);
	return NULL;
}


static void sca_app_on_seacatcc_async(struct ev_loop * loop, ev_async * w, int revents)
{
	assert(w != NULL);
	struct sca_app * this = w->data;
	ASSERT_THIS();

	// Basically do nothing ...
}


void sca_app_on_seacatcc_check(struct ev_loop * loop, ev_check * w, int revents)
{
	assert(w != NULL);
	struct sca_app * this = w->data;
	ASSERT_THIS();

	if (this->seacatcc_thread_rc != -1)
	{
		ev_check_stop(this->context.ev_loop, w);
	}

}

static void sca_app_loop_release(struct ev_loop * loop)
{
	sca_loop_lock_release();
}

static void sca_app_loop_acquire(struct ev_loop * loop)
{
	sca_loop_lock_acquire();
}

///

void sca_app_on_prepare(struct ev_loop * loop, ev_prepare * w, int revents)
{
	assert(w != NULL);
	struct sca_app * this = w->data;
	ASSERT_THIS();

	// Remove control sockets that are closed
restart:
	FT_LIST_FOR(&this->cntl_list, node)
	{
		struct sca_cntl * cntl = (struct sca_cntl *)node->data;
		if (ft_stream_is_shutdown(&cntl->stream))
		{
			ft_list_remove(&this->cntl_list, node);
			goto restart; // List has been changed during iteration
		}
	}

}

// We have been asked to exit, inform SeaCat C-Core about that ...
void sca_on_exit(struct ft_subscriber * sub, struct ft_pubsub * pubsub, const char * topic, void * data)
{
	int rc;
	assert(sub != NULL);
	struct sca_app * this = sub->data;
	ASSERT_THIS();

	// Shutdown SeaCat CCore
	rc = seacatcc_shutdown();
	if (rc != SEACATCC_RC_OK)
	{
		FT_WARN("Error in seacatcc_shutdown: %d", rc);
	}
	this->state = 'e';

	// Stop listening on control sockets
	ft_listener_list_cntl(&this->cntl_listeners_list, FT_LISTENER_STOP);

	sca_connectivity_exit(&this->connectivity);
}
