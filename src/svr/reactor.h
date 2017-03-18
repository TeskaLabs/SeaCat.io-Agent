#ifndef TLSCA_SVR__REACTOR_H_
#define TLSCA_SVR__REACTOR_H_

// SeaCat C-Core reactor
void sca_reactor_hook_write_ready(void ** data, uint16_t * data_len);
void sca_reactor_hook_read_ready(void ** data, uint16_t * data_len);
void sca_reactor_hook_frame_received(void * data, uint16_t frame_len);
void sca_reactor_hook_frame_return(void *data);
void sca_reactor_hook_worker_request(char worker);
double sca_reactor_hook_evloop_heartbeat(double now);

#endif //TLSCA_SVR__REACTOR_H_
