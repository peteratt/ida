#ifdef __cplusplus
extern "C" {
#endif

  int ffs_recvfile_c(const char *proto, const char *remote_ip, const char *server_port, const char *remote_filename, const char *local_filename);
  int ffs_sendfile_c(const char *proto, const char *remote_ip, const char *server_port, const char *local_filename, const char *remote_filename);

#ifdef __cplusplus
}
#endif
