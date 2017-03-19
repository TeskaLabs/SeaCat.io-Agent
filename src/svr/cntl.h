#ifndef TLSCA_SVR__CNTL_H_
#define TLSCA_SVR__CNTL_H_

struct sca_cntl
{
	struct ft_stream stream;
	struct sc_proto_spdy spdy;
	struct ft_subscriber exit;

};

void sca_cntl_on_remove(struct ft_list * list, struct ft_list_node * node);

extern struct ft_listener_delegate sca_cntl_listener_delegate;

#endif // TLSCA_SVR__CNTL_H_
