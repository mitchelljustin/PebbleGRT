#include <pebble.h>

void log_four_words(void *ptr)
{
    unsigned int *ptr_raw = ptr;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "%p = %08X %08X %08X %08X", ptr, ptr_raw[0], ptr_raw[1], ptr_raw[2], ptr_raw[3]);
}

void split_semi_delimited(char *src, const char **parts)
{
    while (*src != '\0') {
        *parts = src;
        parts++;
        for (;;) {
            if (*src == ';') {
                *src = '\0';
                src++;
                break;
            }
            if (*src == '\0') {
                return;
            }
            src++;
        }
    }
}