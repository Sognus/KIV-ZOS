#include "usefull_functions.h"

bool starts_with(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

bool str_contains(const char c, const char *str)
{
    while(*str != '\0')
    {
        if(*str == c)
        {
            return 1;
        }
        str = str+1;
    }
    return 0;
}