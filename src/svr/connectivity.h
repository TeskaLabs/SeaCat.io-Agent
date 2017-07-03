#ifndef TLSCA_SVR__CONNECTIVITY_H_
#define TLSCA_SVR__CONNECTIVITY_H_

// This class is responsible for a connectivity strategy
struct sca_connectivity
{
	struct ev_timer timer_w;

	struct ft_subscriber ready_subscriber;
	struct ft_subscriber connected_subscriber;
	struct ft_subscriber disconnected_subscriber;

	ev_tstamp network_error_at;
};

bool sca_connectivity_init(struct sca_connectivity *);
void sca_connectivity_fini(struct sca_connectivity *);

void sca_connectivity_exit(struct sca_connectivity * this);

#endif //TLSCA_SVR__CONNECTIVITY_H_
