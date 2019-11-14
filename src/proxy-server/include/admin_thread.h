#ifndef ADMIN_THREAD_H
#define ADMIN_THREAD_H

void resolve_admin_fd_in_thread(int admin_fd, struct sockaddr_in *admin_addr, socklen_t * admin_addr_len, settings_t settings, metrics_t metrics);

#endif