/*
 * xrdp_module.h
 *
 *  Created on: 16 janv. 2013
 *      Author: david
 */

#ifndef XRDP_MODULE_H_
#define XRDP_MODULE_H_


struct xrdp_user_channel
{
  tbus handle;
  tbus wm;
  tbus self_term_event;

  bool (*init)(struct xrdp_user_channel*);
  int (*exit)(struct xrdp_user_channel*);

  // module functions
  int (*get_data_descriptor)(struct xrdp_user_channel*, tbus* robjs, int* rc, tbus* wobjs, int* wc, int* timeout);
  struct list* (*get_data)(struct xrdp_user_channel*);
  int (*disconnect)(struct xrdp_user_channel*);
  int (*end)(struct xrdp_user_channel*);
  bool (*connect)(struct xrdp_user_channel* user_channel, int session_id, struct xrdp_session* session);
  int (*callback)(long id, int msg, long param1, long param2, long param3, long param4);
  int (*get_login_mode)(struct xrdp_user_channel*);
  void (*set_network_stat)(struct xrdp_user_channel* user_channel, long bandwidth, int rtt);
  void (*set_static_framerate)(struct xrdp_user_channel* user_channel, int framerate);

  // xrdp function
  int (*is_term)();
};


#endif /* XRDP_MODULE_H_ */
