#include "all.h"

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
	fprintf(stderr, "> %s\n", __func__);
}

void sca_reactor_hook_read_ready(void ** data, uint16_t * data_len)
{
	fprintf(stderr, "> %s\n", __func__);
}

void sca_reactor_hook_frame_received(void * data, uint16_t frame_len)
{
	fprintf(stderr, "> %s\n", __func__);
}

void sca_reactor_hook_frame_return(void *data)
{
	fprintf(stderr, "> %s\n", __func__);
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
	return 10.0;
}
