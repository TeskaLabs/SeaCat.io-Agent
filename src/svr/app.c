#include "all.h"

///

static void sca_on_exit(struct ft_subscriber *, struct ft_pubsub * pubsub, const char * topic, void * data);

static void * sca_app_seacatcc_thread(void * data);
static void sca_app_on_seacatcc_check(struct ev_loop * loop, ev_check *w, int revents);
static void sca_app_on_seacatcc_async(struct ev_loop * loop, ev_async * w, int revents);

///

bool sca_app_init(struct sca_app * this)
{
	bool ok;
	int rc;

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
	void * p;
	int rc;

	ASSERT_THIS();

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

///

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
}

