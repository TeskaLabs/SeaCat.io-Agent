#ifndef TLSCA_SVR__REACTOR_H_
#define TLSCA_SVR__REACTOR_H_

extern const char * SCA_PUBSUB_TOPIC_SEACATCC_STATE_CHANGED;
extern const char * SCA_PUBSUB_TOPIC_SEACATCC_IS_READY;
extern const char * SCA_PUBSUB_TOPIC_SEACATCC_IS_NOT_READY;
extern const char * SCA_PUBSUB_TOPIC_SEACATCC_CONNECTED;
extern const char * SCA_PUBSUB_TOPIC_SEACATCC_DISCONNECTED;

void sca_reactor_init(void);
void sca_reactor_send(struct ft_frame * frame);

#endif //TLSCA_SVR__REACTOR_H_
