#pragma once

#include <stdint.h>

#include <AG_Common/AG_Common.h>

#include "AG_HAL_Macros.h"

namespace AG_HAL {

void init();

void panic(const char *errormsg, ...) FMT_PRINTF(1, 2) NORETURN;

uint16_t micros16();
uint32_t micros();
uint32_t millis();
uint16_t millis16();
uint64_t micros64();
uint64_t millis64();

uint32_t native_micros();
uint32_t native_millis();
uint16_t native_millis16();
uint64_t native_micros64();
uint64_t native_millis64();

void dump_stack_trace();
void dump_core_file();

} // namespace AG_HAL
