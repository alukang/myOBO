#ifndef __OBO_BUSI_H__
#define __OBO_BUSI_H__

#define MYHTTPD_SIGNATURE   "MoCarHttpd v0.1"

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <curl/curl.h>
#include <uuid/uuid.h>

#define RESPONSE_LEN 4096
#define WEB_SERVER_IP "172.17.3.251"
#define WEB_SERVER_PORT "8080"

typedef struct response_data{
    char data[RESPONSE_LEN];
    int data_len;
}response_data_t;

int send_curl(const char *url,const char*src,response_data_t *arg);

void persistent_cb (struct evhttp_request *req, void *arg);
void cache_cb (struct evhttp_request *req, void *arg);





#endif
