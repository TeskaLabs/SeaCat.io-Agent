#include "all.h"

//TODO: Consider auto-adding an 'bad connection' detector - it means that if there is e.g. NAT that disconnect the agent frequently, shorten the keepalive interval accordingly

static void sca_connectivity_on_keepalive(struct ev_loop * loop, ev_timer * w, int revents);
static void sca_connectivity_on_connecting(struct ev_loop * loop, ev_timer * w, int revents);
static void sca_connectivity_on_ready(struct ft_subscriber * sub, struct ft_pubsub * pubsub, const char * topic, void * data);
static void sca_connectivity_on_connected(struct ft_subscriber * sub, struct ft_pubsub * pubsub, const char * topic, void * data);
static void sca_connectivity_on_disconnected(struct ft_subscriber * sub, struct ft_pubsub * pubsub, const char * topic, void * data);

///

bool sca_connectivity_init(struct sca_connectivity * this)
{
	ASSERT_THIS();

	this->network_error_at = 0.0; // This means not observed

	ev_init(&this->timer_w, NULL);
	this->timer_w.data = this;

	ft_subscriber_init(&this->ready_subscriber, sca_connectivity_on_ready);
	ft_subscriber_subscribe(&this->ready_subscriber, NULL, SCA_PUBSUB_TOPIC_SEACATCC_IS_READY);
	this->ready_subscriber.data = this;

	ft_subscriber_init(&this->connected_subscriber, sca_connectivity_on_connected);
	ft_subscriber_subscribe(&this->connected_subscriber, NULL, SCA_PUBSUB_TOPIC_SEACATCC_CONNECTED);
	this->connected_subscriber.data = this;

	ft_subscriber_init(&this->disconnected_subscriber, sca_connectivity_on_disconnected);
	ft_subscriber_subscribe(&this->disconnected_subscriber, NULL, SCA_PUBSUB_TOPIC_SEACATCC_DISCONNECTED);
	this->disconnected_subscriber.data = this;

	return true;
}

void sca_connectivity_fini(struct sca_connectivity * this)
{
	ASSERT_THIS();

	ft_subscriber_fini(&this->disconnected_subscriber);
	ft_subscriber_fini(&this->connected_subscriber);
	ft_subscriber_fini(&this->ready_subscriber);
}

void sca_connectivity_exit(struct sca_connectivity * this)
{
	seacatcc_yield('d');
	ev_timer_stop(sca_app.context.ev_loop, &this->timer_w);
}
///

void sca_connectivity_on_ready(struct ft_subscriber * sub, struct ft_pubsub * pubsub, const char * topic, void * data)
{
	assert(sub != NULL);
	struct sca_connectivity * this = sub->data;
	ASSERT_THIS();

	FT_INFO("Ready");

	// Start the connection procedure (ASAP)
	ev_timer_stop(sca_app.context.ev_loop, &this->timer_w);
	ev_set_cb(&this->timer_w, sca_connectivity_on_connecting);
	this->timer_w.repeat = sca_config.connecting_interval;
	ev_timer_start(sca_app.context.ev_loop, &this->timer_w);
	ev_invoke(sca_app.context.ev_loop, &this->timer_w, 0);
}


void sca_connectivity_on_connected(struct ft_subscriber * sub, struct ft_pubsub * pubsub, const char * topic, void * data)
{
	assert(sub != NULL);
	struct sca_connectivity * this = sub->data;
	ASSERT_THIS();

	this->network_error_at = 0.0; // This means not observed

	FT_INFO("Connected");

	// Reconfigure to keep-alive procedure
	ev_timer_stop(sca_app.context.ev_loop, &this->timer_w);
	ev_set_cb(&this->timer_w, sca_connectivity_on_keepalive);
	this->timer_w.repeat = sca_config.keepalive_interval;
	ev_timer_start(sca_app.context.ev_loop, &this->timer_w);
}


void sca_connectivity_on_disconnected(struct ft_subscriber * sub, struct ft_pubsub * pubsub, const char * topic, void * data)
{
	assert(sub != NULL);
	struct sca_connectivity * this = sub->data;
	ASSERT_THIS();

	if (sca_app.state == 'e')
	{
		int rc;

		FT_INFO("Disconnected, ready to exit");

		// Shutdown SeaCat CCore
		rc = seacatcc_shutdown();
		if (rc != SEACATCC_RC_OK)
		{
			FT_WARN("Error in seacatcc_shutdown: %d", rc);
		}

		return;
	}

	FT_INFO("Disconnected");

	// Reconfigure to connecting procedure
	ev_timer_stop(sca_app.context.ev_loop, &this->timer_w);
	ev_set_cb(&this->timer_w, sca_connectivity_on_connecting);
	this->timer_w.repeat = sca_config.connecting_interval;
	ev_timer_start(sca_app.context.ev_loop, &this->timer_w);
}


void sca_connectivity_on_connecting(struct ev_loop * loop, ev_timer * w, int revents)
{
	assert(w != NULL);
	struct sca_connectivity * this = w->data;
	ASSERT_THIS();

	if (sca_app.seacatcc_state[0] == 'n')
	{
		if ((ev_now(loop) - this->network_error_at) < 60.0) return;
		FT_INFO("Trying to recover from a network connectivity loss");
		seacatcc_yield(81);
	}

	FT_INFO("Connecting ...");
	seacatcc_yield('c');
}

void sca_connectivity_on_keepalive(struct ev_loop * loop, ev_timer * w, int revents)
{
	assert(w != NULL);
	struct sca_connectivity * this = w->data;
	ASSERT_THIS();

	FT_DEBUG("Sending keep-alive ping");

	struct ft_frame * frame = ft_pool_borrow(&sca_app.context.frame_pool, (SPDY_CNTL_FRAME_VERSION_SPD3 << 16) | SPDY_CNTL_TYPE_PING);
	if (frame == NULL) return;

	static uint32_t ping_id = 1;

	ft_frame_format_empty(frame);
	sc_proto_spdy_ping_build(frame, ping_id);
	ping_id += 2;

	ft_frame_flip(frame);
	sca_reactor_send(frame);
}
