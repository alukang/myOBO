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
void reg_cb (struct evhttp_request *req, void *arg)
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
    
    cJSON* root = cJSON_CreateObject();
    root = cJSON_Parse(request_data_buf);
    
    /*        username: "gailun",
              password: "123123",
            driver:   "yes/no",
            tel:      "13331333333",
            email:    "danbing_at@163.cn",
             id_card:  "2104041222121211122"  */
    char * username = cJSON_GetObjectItem(root,"username")->valuestring;
    char * password = cJSON_GetObjectItem(root,"password")->valuestring;
    char * driver = cJSON_GetObjectItem(root,"driver")->valuestring;
    char * tel = cJSON_GetObjectItem(root,"password")->valuestring;
    char * email = cJSON_GetObjectItem(root,"email")->valuestring;
    char * id_card = cJSON_GetObjectItem(root,"id_card")->valuestring;
/*    printf("username = %s\n",username);
    printf("password = %s\n",password);
    printf("driver = %s\n",driver);
    printf("tel = %s\n",password);
    printf("email = %s\n",email);
    printf("id_card = %s\n",id_card);
*/    // 查询 mysql数据库
/*
    {
                cmd: "insert",
              busi: "reg",
          table: "OBO_TABLE_USER",
          username:  "盖伦",
          password:  "ADSWADSADWQ(MD5加密之后的)",                  
          tel     :  "13332133313",                                          
        email   :  "danbing_at@163.com",                                                                        
        id_card :  "21040418331323",
        driver  :  "yes",                                                          
    }
  */  
    cJSON* request_root = cJSON_CreateObject();
    cJSON_AddStringToObject(request_root,"cmd","insert");
    cJSON_AddStringToObject(request_root,"busi","reg");
    cJSON_AddStringToObject(request_root,"table",OBO_TABLE_USER);
    cJSON_AddStringToObject(request_root,"username",username);
    cJSON_AddStringToObject(request_root,"password",password);
    cJSON_AddStringToObject(request_root,"tel",tel);
    cJSON_AddStringToObject(request_root,"email",email);
    cJSON_AddStringToObject(request_root,"id_card",id_card);
    cJSON_AddStringToObject(request_root,"driver",driver);
    char *request_data = cJSON_Print(request_root);
// 删除 root, request_root;
    cJSON_Delete(root);
    cJSON_Delete(request_root);
    // 发送到data_server端
    char url[64];
    sprintf(url, "https://%s:%s/persistent",DATA_SERVER_IP,DATA_SERVER_PORT);
    response_data_t response_data;
    memset(&response_data,0,sizeof(response_data));
    if(send_curl(url,request_data,&response_data)== -1){
        printf("conn https_data_server fail");
    }
    //成功
    //    {
    // result: "ok",
    //  recode: "0"
    //  }
    //                            //失败
    //  {
    //       result: "error",
    //      reason: "why...."}
    //                                }
    //    }
// 解析 https_data_server 返回数据 
    cJSON* sql_response = cJSON_CreateObject();
    sql_response = cJSON_Parse(response_data.data);
    cJSON *result = cJSON_GetObjectItem(sql_response,"result");
    
    if(result&&strcmp(result->valuestring, "ok")== 0){
        // 插入数据库成功
        printf("插入数据成功");    
    }
    else{
        cJSON* reason = cJSON_GetObjectItem(sql_response,"reason");
        if(reason){
            printf("insert fail ; reason =%s",reason->valuestring);
        }
        else{

        }
    }
    
/*{
                result: "ok",
                recode: "0",
                sessionid: "online-driver-xxxx-xxx-xxx-xxxx",
                orderid:"NONE",
                status:"idle"                                                                    
}
        //失败
        //        {
        //          result: "error",
        //          reason: "why...."
        //      }*/
    cJSON* web_response_root=cJSON_CreateObject();
    if(1){
        cJSON_AddStringToObject(web_response_root,"result","ok");
        cJSON_AddStringToObject(web_response_root,"recode","0");
        cJSON_AddStringToObject(web_response_root,"sessionid","");
        cJSON_AddStringToObject(web_response_root,"orderid","NONE");
        cJSON_AddStringToObject(web_response_root,"status","idle");
    }
    else{
        cJSON_AddStringToObject(web_response_root,"result","error");
        cJSON_AddStringToObject(web_response_root,"reason","why...");

    }
    
    char*web_response_data = cJSON_Print(web_response_root);
    cJSON_Delete(web_response_root);

    //HTTP header
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");

    evb = evbuffer_new ();
    evbuffer_add_printf(evb, "%s", web_response_data);
    //将封装好的evbuffer 发送给客户端
    evhttp_send_reply(req, HTTP_OK, "OK", evb);

    if (decoded)
        evhttp_uri_free (decoded);
    if (evb)
        evbuffer_free (evb);


    printf("[response]:\n");
    printf("%s\n", response_data.data);

    free(web_response_data);
}
