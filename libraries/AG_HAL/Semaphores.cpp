#include "AG_HAL.h"

extern const AG_HAL::HAL &hal;

/*
  implement WithSemaphore class for WITH_SEMAPHORE() support
 */
WithSemaphore::WithSemaphore(AG_HAL::Semaphore *mtx, uint32_t line) :
    WithSemaphore(*mtx, line)
{}

WithSemaphore::WithSemaphore(AG_HAL::Semaphore &mtx, uint32_t line) :
    _mtx(mtx)
{
    bool in_main = hal.scheduler->in_main_thread();
    if (in_main) {
        hal.util->persistent_data.semaphore_line = line;
    }
    _mtx.take_blocking();
    if (in_main) {
        hal.util->persistent_data.semaphore_line = 0;
    }
}

WithSemaphore::~WithSemaphore()
{
    _mtx.give();
}
