#include "all.h"

///

// SeaCat C-Core reactor
static void sca_reactor_hook_write_ready(void ** data, uint16_t * data_len);
static void sca_reactor_hook_read_ready(void ** data, uint16_t * data_len);
static void sca_reactor_hook_frame_received(void * data, uint16_t frame_len);
static void sca_reactor_hook_frame_return(void *data);
static void sca_reactor_hook_worker_request(char worker);
static double sca_reactor_hook_evloop_heartbeat(double now);

static void sca_reactor_hook_client_state_changed(void);
static void sca_reactor_hook_connected(void);
static void sca_reactor_hook_disconnected(void);

static void sca_reactor_log(char level, const char * message);

///

void sca_reactor_init()
{
	int rc;

	seacatcc_set_discover_domain("\01s\06seacat\02io");
	seacatcc_log_setfnct(sca_reactor_log);

	memset(sca_app.seacatcc_state, '?', SEACATCC_STATE_BUF_SIZE);
	sca_app.seacatcc_state[SEACATCC_STATE_BUF_SIZE-1] = '\0';

	rc = seacatcc_init(
		sca_config.application_id,
		sca_config.application_id_suffix,
		OS_NAME,
		sca_config.var_dir,
		sca_reactor_hook_write_ready,
		sca_reactor_hook_read_ready,
		sca_reactor_hook_frame_received,
		sca_reactor_hook_frame_return,
		sca_reactor_hook_worker_request,
		sca_reactor_hook_evloop_heartbeat
	);
	if (rc != SEACATCC_RC_OK)
	{
		FT_FATAL("SeaCat C-Core failed to initialise: %d", rc);
		exit(EXIT_FAILURE);
	}

	// Registed state_changed event
	rc = seacatcc_hook_register('S', sca_reactor_hook_client_state_changed);
	if (rc != SEACATCC_RC_OK)
	{
		FT_FATAL_P("SeaCat C-Core failed to initialise: %d", rc);
		exit(EXIT_FAILURE);
	}

	// Registed gateway_connected event
	rc = seacatcc_hook_register('c', sca_reactor_hook_connected);
	if (rc != SEACATCC_RC_OK)
	{
		FT_FATAL_P("SeaCat C-Core failed to initialise: %d", rc);
		exit(EXIT_FAILURE);
	}

	// Registed gateway_connected event
	rc = seacatcc_hook_register('R', sca_reactor_hook_disconnected);
	if (rc != SEACATCC_RC_OK)
	{
		FT_FATAL_P("SeaCat C-Core failed to initialise: %d", rc);
		exit(EXIT_FAILURE);
	}

	const char * capabilities[] = {
		NULL, NULL
	};

	seacatcc_capabilities_store(capabilities);

}


void sca_reactor_send(struct ft_frame * frame)
{
	assert(frame != NULL);
	assert(*sca_app.seacatcc_write_queue_last == NULL);

	seacatcc_yield('W');

	*sca_app.seacatcc_write_queue_last = frame;
	frame->next = NULL;
	sca_app.seacatcc_write_queue_last = &frame->next;
}

///

static void * sca_reactor_worker_ppkgen(void * p)
{
	seacatcc_ppkgen_worker();
	return NULL;
}

static void * sca_reactor_worker_csrgen(void * p)
{
	//TODO: Consider reading this from a config file
	const char * entries[] = {
		NULL, NULL
	};
	seacatcc_csrgen_worker(entries);
	return NULL;
}

///

void sca_reactor_hook_write_ready(void ** data, uint16_t * data_len)
{
	if (sca_app.seacatcc_write_queue == NULL)
	{
		*data = NULL;
		*data_len = 0;
		return;
	}

	sca_app.seacatcc_write_buffer = sca_app.seacatcc_write_queue;
	sca_app.seacatcc_write_queue = sca_app.seacatcc_write_buffer->next;
	if (sca_app.seacatcc_write_queue == NULL)
	{
		sca_app.seacatcc_write_queue_last = &sca_app.seacatcc_write_queue;
	}

	struct ft_vec * vec = ft_frame_get_vec(sca_app.seacatcc_write_buffer);
	assert(vec != NULL);
	*data = ft_vec_ptr(vec);
	*data_len = vec->capacity;
}

void sca_reactor_hook_read_ready(void ** data, uint16_t * data_len)
{
	sca_loop_lock_acquire();
	assert(sca_app.seacatcc_read_buffer == NULL);

	sca_app.seacatcc_read_buffer = ft_pool_borrow(&sca_app.context.frame_pool, FT_FRAME_TYPE_SEACATCC_READ);
	if (sca_app.seacatcc_read_buffer == NULL)
	{
		FT_ERROR("Failed to allocate frame for seacatcc read buffer");
		goto exit;
	}

	ft_frame_format_simple(sca_app.seacatcc_read_buffer);
	struct ft_vec * vec = ft_frame_get_vec(sca_app.seacatcc_read_buffer);
	assert(vec != NULL);
	*data = ft_vec_ptr(vec);
	*data_len = ft_vec_len(vec);

exit:
	sca_loop_lock_release();
}

void sca_reactor_hook_frame_received(void * data, uint16_t frame_len)
{
	if ((sca_app.seacatcc_read_buffer == NULL) || (sca_app.seacatcc_read_buffer->data != data))
	{
		FT_ERROR_P("Received unexpected data, other read frame");
	}

	struct ft_frame * frame = sca_app.seacatcc_read_buffer;
	sca_app.seacatcc_read_buffer = NULL;

	ft_frame_return(frame);
}

void sca_reactor_hook_frame_return(void * data)
{
	sca_loop_lock_acquire();

	if ((sca_app.seacatcc_read_buffer != NULL) && (sca_app.seacatcc_read_buffer->data == data))
	{
		ft_frame_return(sca_app.seacatcc_read_buffer);
		sca_app.seacatcc_read_buffer = NULL;
		goto exit;
	}


	if ((sca_app.seacatcc_write_buffer != NULL) && (sca_app.seacatcc_write_buffer->data == data))
	{
		ft_frame_return(sca_app.seacatcc_write_buffer);
		sca_app.seacatcc_write_buffer = NULL;
		goto exit;
	}

	FT_WARN_P("Unidentified data frame returned: %p", data);

exit:
	sca_loop_lock_release();
}

void sca_reactor_hook_worker_request(char worker)
{
	int rc;
	pthread_t thread;

	switch (worker)
	{
		case 'P':
			rc = pthread_create(&thread, NULL, sca_reactor_worker_ppkgen, NULL);
			if (rc != 0)
			{
				fprintf(stderr, "%s pthread_create():%d\n", __func__, rc);
				return;
			}
			pthread_detach(thread);
			break;

		case 'C':
			rc = pthread_create(&thread, NULL, sca_reactor_worker_csrgen, NULL);
			if (rc != 0)
			{
				fprintf(stderr, "%s pthread_create():%d\n", __func__, rc);
				return;
			}
			pthread_detach(thread);
			break;

		default:
			fprintf(stderr, "> %s w:'%c'\n", __func__, worker);
	}
}

double sca_reactor_hook_evloop_heartbeat(double now)
{
	return 120.0;
}


void sca_reactor_log(char level, const char * message)
{
	sca_loop_lock_acquire();
	_ft_log(level, NULL, "%s", message);
	sca_loop_lock_release();
}

///

bool sca_is_ready(void)
{
	return (sca_app.seacatcc_state[3] == 'Y') && (sca_app.seacatcc_state[4] == 'N') && (sca_app.seacatcc_state[0] != 'f');
}

const char * SCA_PUBSUB_TOPIC_SEACATCC_STATE_CHANGED = "sca/seacatcc/state-changed";
const char * SCA_PUBSUB_TOPIC_SEACATCC_IS_READY = "sca/seacatcc/ready";
const char * SCA_PUBSUB_TOPIC_SEACATCC_IS_NOT_READY = "sca/seacatcc/notready";
const char * SCA_PUBSUB_TOPIC_SEACATCC_CONNECTED = "sca/seacatcc/connected";
const char * SCA_PUBSUB_TOPIC_SEACATCC_DISCONNECTED = "sca/seacatcc/disconnected";

void sca_reactor_hook_client_state_changed(void)
{
	sca_loop_lock_acquire();
	bool old_is_ready = sca_is_ready();

	seacatcc_state(sca_app.seacatcc_state);
	FT_DEBUG("C-Core state: %s", sca_app.seacatcc_state);

	ft_pubsub_publish(NULL , SCA_PUBSUB_TOPIC_SEACATCC_STATE_CHANGED, sca_app.seacatcc_state);

	bool new_is_ready = sca_is_ready();
	if ((old_is_ready == false) && (new_is_ready == true))
	{
		ft_pubsub_publish(NULL, SCA_PUBSUB_TOPIC_SEACATCC_IS_READY, sca_app.seacatcc_state);
	}
	else if ((old_is_ready == true) && (new_is_ready == false))
	{
		ft_pubsub_publish(NULL, SCA_PUBSUB_TOPIC_SEACATCC_IS_NOT_READY, sca_app.seacatcc_state);
	}

	sca_loop_lock_release();
}

void sca_reactor_hook_connected()
{
	sca_loop_lock_acquire();
	ft_pubsub_publish(NULL, SCA_PUBSUB_TOPIC_SEACATCC_CONNECTED, NULL);
	sca_loop_lock_release();
}

void sca_reactor_hook_disconnected()
{
	sca_loop_lock_acquire();
	ft_pubsub_publish(NULL, SCA_PUBSUB_TOPIC_SEACATCC_DISCONNECTED, NULL);
	sca_loop_lock_release();
}
