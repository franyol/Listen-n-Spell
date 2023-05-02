#include <stdlib.h>

#ifndef _SPEECH_TO_TEXT_
#define _SPEECH_TO_TEXT_

    // Constant definitions

    // Type definitions

    struct string {
        char *ptr;
        size_t len;
    };

    // Global variables

    extern char* _oai_api_key;

    // Function prototypes

    void init_string(struct string *s);
    /**
     * @brief Inits string class which allocating
     *        a 0 size string ending with \0
     * 
     */

    size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
    /**
     * @brief Curl callback to save speech to text response into
     *        a string.
     * 
     */

    char *audio_file_to_text (char *filename);
    /**
     * @brief Gets a str with the API response
     * 
     */

    void set_api_key (char *key);
    /**
     * @brief Saves the key in s global variable
     * 
     */

#endif