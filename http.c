#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "http.h"


/**
 * httprequest_parse_headers
 * 
 * Populate a `req` with the contents of `buffer`, returning the number of bytes used from `buf`.
 */
ssize_t httprequest_parse_headers(HTTPRequest *req, char *buffer, ssize_t buffer_len) {
  int bytes_read = 0;
  req->buffer = calloc(1, buffer_len+1);
  memcpy(req->buffer, buffer, buffer_len);

  char *end = strstr(req->buffer, "\r\n\r\n");
  char *token;
  token = strtok(req->buffer, " ");
  req->action = token;
  bytes_read += strlen(token) + 1;
  
  token = strtok(NULL, " ");
  req->path = token;
  bytes_read += strlen(token) + 1;
  
  token = strtok(NULL, "\r");
  req->version = token;
  bytes_read += strlen(token) + 1;

  Header *header = (Header *) malloc(sizeof(Header));
  token = strtok(NULL, ":");
  bytes_read += strlen(token) + 1;
  header->key = calloc(1, strlen(token));
  strncpy(header->key, token+1, strlen(token)-1);
  
  token = strtok(NULL, "\r");
  bytes_read += strlen(token) + 1;
  header->value = calloc(1, strlen(token));
  strncpy(header->value, token+1, strlen(token)-1);
  header->next = NULL;
  req->header = header;

  Header *curr = req->header;
  while (token + strlen(token) != end) {
    Header *header = malloc(sizeof(Header));
    header->next = NULL;
    token = strtok(NULL, ":");
    bytes_read += strlen(token) + 1;
    header->key = calloc(1, strlen(token));
    strncpy(header->key, token+1, strlen(token)-1);

    token = strtok(NULL, "\r");
    bytes_read += strlen(token) + 1;
    header->value = calloc(1, strlen(token));
    strncpy(header->value, token+1, strlen(token)-1);
    curr->next = header;
    curr = curr->next;
  }

  return bytes_read + 3;
}


/**
 * httprequest_read
 * 
 * Populate a `req` from the socket `sockfd`, returning the number of bytes read to populate `req`.
 */
ssize_t httprequest_read(HTTPRequest *req, int sockfd) {
  int BUFFER_SIZE = 1024;
  int bytes_read = 0;
  char *buffer = malloc(BUFFER_SIZE);
  bytes_read = read(sockfd, buffer, 1024);
  int total_bytes = bytes_read;
  while (bytes_read == BUFFER_SIZE) {
    bytes_read = read(sockfd, buffer, 1024);
    total_bytes += bytes_read;
    buffer = realloc(buffer, total_bytes);
  }

  ssize_t size = httprequest_parse_headers(req, buffer, total_bytes);
  const char *content_length = httprequest_get_header(req, "Content-Length");
  if (!content_length) {
    free(buffer);
    req->payload = NULL;
    return size;
  }

  int length = atoi(content_length);
  req->payload = req->buffer + size;
  free(buffer);
  return size + length;
}


/**
 * httprequest_get_action
 * 
 * Returns the HTTP action verb for a given `req`.
 */
const char *httprequest_get_action(HTTPRequest *req) {
  return req->action;
}


/**
 * httprequest_get_header
 * 
 * Returns the value of the HTTP header `key` for a given `req`.
 */
const char *httprequest_get_header(HTTPRequest *req, const char *key) {
  Header *curr = req->header;
  while (curr) {
    if (strcmp(curr->key, key) == 0) {
      return curr->value;
    }
    curr = curr->next;
  }
  return NULL;
}


/**
 * httprequest_get_path
 * 
 * Returns the requested path for a given `req`.
 */
const char *httprequest_get_path(HTTPRequest *req) {
  return req->path;
}


/**
 * httprequest_destroy
 * 
 * Destroys a `req`, freeing all associated memory.
 */
void httprequest_destroy(HTTPRequest *req) {
  free(req->buffer);
  Header *curr = req->header;
  while (curr) {
    Header *temp = curr;
    curr = curr->next;
    free(temp->key);
    free(temp->value);
    free(temp);
  }
}