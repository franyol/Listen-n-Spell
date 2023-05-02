#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <speech_to_text.h>
#include <string.h>

extern char** environ;

/***** START OF Function prototypes *****/

char *read_password (char *message);
char* get_env (char* var_key);

/****** END OF Function prototypes ******/

int main (int argc, char* argv[])
{
    char *oai_api_key, *api_response;

    oai_api_key = get_env ("OAI_API_KEY");

    if (oai_api_key == NULL)
        oai_api_key = read_password("Paste your OAI api key and press enter: ");

    set_api_key(oai_api_key);
    api_response = audio_file_to_text(argv[1]);

    printf ("RESPONSE FROM OAI WHISPER:\n*\n%s\n", api_response);

    return 0;
}

/* Function definitions */

char *read_password (char *message) {
    struct termios term;
    char temp[100] = "";
    char *passwd;
    size_t pas_len;
    FILE* tty;

    /* Hide stdin */
    tcgetattr(fileno(stdin), &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), 0, &term);
    
    printf ("%s", message);

    /* Read password from terminal */
    tty = fopen ("/dev/tty", "r");
    fscanf (tty, "%s", temp);
    printf ("\n");
    fclose (tty);

    /* Show stdin */
    term.c_lflag |= ECHO;
    tcsetattr (fileno(stdin), 0, &term);

    /* Save in an exact-sized str */
    pas_len = strlen(temp+1);
    passwd = (char*) malloc (pas_len);
    strcpy (passwd, temp);

    return passwd;
}

char* get_env (char* var_key) {
    char** var;
    char* eq_key;
    int i, is_exact;
    size_t key_length, env_length = strlen((var_key));

    for (var = environ; *var != NULL; ++var) {
        is_exact = 1;
        eq_key = strchr(*var, '=');
        key_length = eq_key - *var;
        if (key_length != env_length)
            continue;
        for (i = 0; i<key_length; i++) {
            if (*(*var+i) != *(var_key+i)) {
                is_exact = 0;
                break;
            }
        }
        if (is_exact) {
            return eq_key+1;
        }
    }
    return NULL;
}