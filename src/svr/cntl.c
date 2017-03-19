#include "all.h"

///

static struct ft_stream_delegate sca_cntl_stream_delegate;
static struct sc_proto_spdy_delegate sca_cntl_spdy_delegate;

///

static bool sca_cntl_on_accept_cb(struct ft_listener * listening_socket, int fd, const struct sockaddr * client_addr, socklen_t client_addr_len)
{
	bool ok;

	struct sca_app * app = listening_socket->base.socket.data;
	assert(app != NULL);

	struct ft_list_node * new_node = ft_list_node_new(sizeof(struct sca_cntl));
	if (new_node == NULL) return false;

	struct sca_cntl * this = (struct sca_cntl *)&new_node->data;

	// Internal sca_cntl_init continues from here
	ok = ft_stream_accept(&this->stream, &sca_cntl_stream_delegate, listening_socket, fd, client_addr, client_addr_len);
	if (!ok)
	{
		ft_list_node_del(new_node);
		return false;
	}

	ok = sc_proto_spdy_init(&this->spdy, &sca_cntl_spdy_delegate);
	if (!ok)
	{
		FT_ERROR("Cannot initialise a bus client protocol");
		ft_stream_fini(&this->stream);
		this->stream.base.socket.clazz = NULL;
		return false;
	}
	this->stream.base.socket.protocol = &this->spdy;
	this->spdy.data = this;

	ft_list_add(&app->cntl_list, new_node);

	FT_INFO("Control connection open");

	return true;
}

void sca_cntl_on_remove(struct ft_list * list, struct ft_list_node * node)
{
	struct sca_cntl * this = (struct sca_cntl *)node->data;
	ASSERT_THIS();

	FT_INFO("Control connection closed");

	// Internal sca_cntl_fini continues from here
	sc_proto_spdy_fini(&this->spdy);
	ft_stream_fini(&this->stream);
}



struct ft_listener_delegate sca_cntl_listener_delegate =
{
	.accept = sca_cntl_on_accept_cb,
};

static struct ft_stream_delegate sca_cntl_stream_delegate =
{
	.get_read_frame = sc_proto_spdy_stream_delegate_get_read_frame,
	.read = sc_proto_spdy_stream_delegate_read,
};

static struct sc_proto_spdy_delegate sca_cntl_spdy_delegate =
{

};
