#pragma once
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


struct _Header {
  char *key;
  char *value;
  struct _Header *next;
};
typedef struct _Header Header;


struct _HTTPRequest {
  const char *action;
  const char *path;
  const char *version;
  const void *payload;
  // You may want p reqto add more to this struct (ex: to hold the headers).
  // ...however, you MUST keep the `action`, `path`, `version`, and `payload` members
  //    so our test cases can inspect the contents of them without making you write
  //    a bunch of getter functions. :)
  char *buffer;
  struct _Header *header;
};
typedef struct _HTTPRequest HTTPRequest;


ssize_t httprequest_read(HTTPRequest *req, int sockfd);
ssize_t httprequest_parse_headers(HTTPRequest *req, char *buffer, ssize_t buffer_len);
const char *httprequest_get_action(HTTPRequest *req);
const char *httprequest_get_header(HTTPRequest *req, const char *key);
const char *httprequest_get_path(HTTPRequest *req);
void httprequest_destroy(HTTPRequest *req);

#ifdef __cplusplus
}
#endif