#include <stdlib.h>

typedef struct _token {
    const char* name;
} token;

typedef struct _token_stream {
    token* token;
    struct _token_stream* next;
} token_stream;

token_stream* lex(const char* text);
