#pragma once

#include <stdarg.h>

#include <AG_HAL/AG_HAL.h>

void print_vprintf(AG_HAL::BetterStream *s, const char *fmt, va_list ap);
