#include "lex.h"

token_stream* lex(const char* text)
{
    token_stream* ret_stream = (token_stream*) malloc(sizeof(token_stream));
    if (!ret_stream)
        return NULL;
    token* ret_token = (token*) malloc(sizeof(token));
    ret_token->name = "start";
    ret_stream->token = ret_token;
    return ret_stream;
}
