//
// Created by xinitrix on 13.09.2026.
//

#include <stdio.h>
#include "vu_stdio.h"
#include <stdarg.h>

int vu_snprintf(char* buffer, size_t buf_size, const char* format, ...) {
    va_list args;
    va_start(args, format);

    int result = vsnprintf(buffer, buf_size, format, args);

    va_end(args);
    return result;
}