
#include "util.h"



size_t deal_response(char *ptr, size_t size, size_t nmemb, void *arg)
{
    response_data_t *response_data = (response_data_t*)arg;
    int count = size*nmemb;
    memcpy(response_data->data,ptr,count);
    response_data->data_len= count;
    return count;
}
int send_curl(const char *url,const char *src, response_data_t *arg)
{
    CURL *curl =    NULL;
    CURLcode res;
    curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL,url);
    curl_easy_setopt(curl,CURLOPT_POST,1);
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,src);
    // 忽略CA 认证
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
    // 设置回调
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,deal_response);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,arg);
    curl_easy_perform(curl);
    if(res!= CURLE_OK){
        printf("curl perform error, res = %d",res);
        curl_easy_cleanup(curl);
        return -1;
    }
    curl_easy_cleanup(curl);
    return 0;
}

