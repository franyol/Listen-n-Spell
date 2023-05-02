#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "speech_to_text.h"

// Global variables definitions

char* _oai_api_key;

// Functions

void init_string(struct string *s)
    /**
     * @brief Inits string class which allocating
     *        a 0 size string ending with \0
     * 
     */
{
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
    /**
     * @brief Curl callback to save speech to text response into
     *        a string.
     * 
     */
{
    size_t new_len = s->len + size*(nmemb-11);
    s->ptr = realloc(s->ptr, new_len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr+s->len, ptr+9, size*(nmemb-11));
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

void set_api_key (char *key)
    /**
     * @brief Saves the key in s global variable
     * 
     */
{
    _oai_api_key = key;
}

char *audio_file_to_text (char *filename)
    /**
     * @brief Gets a str with the API response
     * 
     */
{
    struct curl_slist *headers = NULL;
    CURLcode res;
    CURL *curl = curl_easy_init();
    char auth_header[120] = "Authorization: Bearer ";
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    struct string api_response;
    init_string(&api_response);

    curl_easy_setopt (curl, CURLOPT_URL, "https://api.openai.com/v1/audio/transcriptions");
    strcat (auth_header, _oai_api_key);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
    curl_formadd(&post, &last, CURLFORM_COPYNAME, "file",
                CURLFORM_FILE, filename, CURLFORM_END);
    curl_formadd(&post, &last, CURLFORM_COPYNAME, "model",
                CURLFORM_COPYCONTENTS, "whisper-1", CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &api_response);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    return api_response.ptr;
}