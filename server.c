#include "http.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

void *client_thread(void *vptr) {
  int fd = *((int *)vptr);
  HTTPRequest *req = (HTTPRequest *) malloc(sizeof(HTTPRequest));
  httprequest_read(req, fd);

  FILE *file;
  if (strcmp(req->path, "/") == 0) {
    char *file_name = "static/index.html";
    file = fopen(file_name, "r");
  } else {
    char *file_name = calloc(1,  strlen("static/") + strlen(req->path) + 1);
    sprintf(file_name, "static/%s", req->path + 1);
    file = fopen(file_name, "r");
    free(file_name);
  }

  if (!file) {
    char *response = "HTTP/1.1 404 Not Found\r\n\r\n";
    write(fd, response, strlen(response));
    close(fd);
    fclose(file);
    httprequest_destroy(req);
    free(req);
    return (void*) response;
  }

  int file_size;
  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  rewind(file);

  char *buffer = calloc(1, file_size);
  fread(buffer, file_size, 1, file);
  fclose(file);

  char header[1024];
  char *dot = strrchr(req->path, '.');
  if (strcmp(req->path, "/") == 0 || (dot && strcmp(dot, ".html") == 0)) {
    int bytes_written = sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", file_size);
    header[bytes_written] = 0;
  } else if (dot && strcmp(dot, ".png") == 0) {
    int bytes_written = sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: %d\r\n\r\n", file_size);
    header[bytes_written] = 0;
  }
  
  char *response = malloc(strlen(header) + file_size);
  strcpy(response, header);
  memcpy(response + strlen(header), buffer, file_size);
  write(fd, response, strlen(header) + file_size);
  
  close(fd);
  httprequest_destroy(req);
  free(buffer);
  free(req);
  return (void*) response;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  printf("Binding to port %d. Visit http://localhost:%d/ to interact with your server!\n", port, port);

  // socket:
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // bind:
  struct sockaddr_in server_addr, client_address;
  memset(&server_addr, 0x00, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);  
  bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

  // listen:
  listen(sockfd, 10);

  // accept:
  socklen_t client_addr_len;
  while (1) {
    int *fd = malloc(sizeof(int));
    *fd = accept(sockfd, (struct sockaddr *)&client_address, &client_addr_len);
    printf("Client connected (fd=%d)\n", *fd);

    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, fd);
    pthread_detach(tid);
  }

  return 0;
}