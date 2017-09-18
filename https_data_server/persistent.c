#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


#include <cJSON.h>
#include "util.h"

/* This callback gets invoked when we get any http request that doesn't match
 * any other callback.  Like any evhttp server callback, it has a simple job:
 * it must eventually call evhttp_send_error() or evhttp_send_reply().
 */
void persistent_cb (struct evhttp_request *req, void *arg)
{ 
    struct evbuffer *evb = NULL;
    const char *uri = evhttp_request_get_uri (req);
    struct evhttp_uri *decoded = NULL;

    /* 判断 req 是否是GET 请求 */
    if (evhttp_request_get_command (req) == EVHTTP_REQ_GET)
    {
        struct evbuffer *buf = evbuffer_new();
        if (buf == NULL) return;
        evbuffer_add_printf(buf, "Requested: %s\n", uri);
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
        return;
    }

    /* 这里只处理Post请求, Get请求，就直接return 200 OK  */
    if (evhttp_request_get_command (req) != EVHTTP_REQ_POST)
    { 
        evhttp_send_reply (req, 200, "OK", NULL);
        return;
    }

    printf ("Got a POST request for <%s>\n", uri);

    //判断此URI是否合法
    decoded = evhttp_uri_parse (uri);
    if (! decoded)
    { 
        printf ("It's not a good URI. Sending BADREQUEST\n");
        evhttp_send_error (req, HTTP_BADREQUEST, 0);
        return;
    }

    /* Decode the payload */
    struct evbuffer *buf = evhttp_request_get_input_buffer (req);
    evbuffer_add (buf, "", 1);    /* NUL-terminate the buffer */
    char *payload = (char *) evbuffer_pullup (buf, -1);
    int post_data_len = evbuffer_get_length(buf);
    char request_data_buf[4096] = {0};
    memcpy(request_data_buf, payload, post_data_len);

    printf("[post_data][%d]=\n %s\n", post_data_len, payload);

 /*   {
        cmd: "insert",
        busi: "reg",
        table: "OBO_TABLE_USER",
        username:  "盖伦",
        password:  "ADSWADSADWQ(MD5加密之后的)",
        tel     :  "13332133313",
        email   :  "danbing_at@163.com",
        id_card :  "21040418331323",
        driver  :  "yes",
    } */
    //拿到数据
    cJSON* root = cJSON_CreateObject();
    root=cJSON_Parse(request_data_buf);
    char *cmd = cJSON_GetObjectItem(root,"cmd")->valuestring;
    char *busi = cJSON_GetObjectItem(root,"busi")->valuestring;
    char *table = cJSON_GetObjectItem(root,"table")->valuestring;
    char *username = cJSON_GetObjectItem(root,"username")->valuestring;
    char *password = cJSON_GetObjectItem(root,"password")->valuestring;
    char *tel = cJSON_GetObjectItem(root,"tel")->valuestring;
    char *id_card = cJSON_GetObjectItem(root,"id_card")->valuestring;
    char *driver = cJSON_GetObjectItem(root,"driver")->valuestring;
    
/*    {
                result: "ok",
                recode: "0"
                                            }
        //失败
        //    {
        //            result: "error",
        //                    reason: "why...."
        //                        }
        //    } */
    cJSON_Delete(root);
    cJSON *response_root = cJSON_CreateObject();
    if(1){
    cJSON_AddStringToObject(response_root,"result","ok");
    cJSON_AddStringToObject(response_root,"recode","0");
    }
    else{
        cJSON_AddStringToObject(response_root,"result","error");
        cJSON_AddStringToObject(response_root,"reason","why");
    }
    char *response_data= cJSON_Print(response_root);
    cJSON_Delete(response_root);
    /* This holds the content we're sending. */

    //HTTP header
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");

    evb = evbuffer_new ();
    evbuffer_add_printf(evb, "%s", response_data);
    //将封装好的evbuffer 发送给客户端
    evhttp_send_reply(req, HTTP_OK, "OK", evb);

    if (decoded)
        evhttp_uri_free (decoded);
    if (evb)
        evbuffer_free (evb);


    printf("[response]:\n");
    printf("%s\n", response_data);

    free(response_data);
}
