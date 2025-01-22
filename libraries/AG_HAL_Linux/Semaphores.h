#pragma once

#include <AG_HAL/AG_HAL_Boards.h>
#include <stdint.h>
#include <AG_HAL/AG_HAL_Macros.h>
#include <AG_HAL/Semaphores.h>
#include <pthread.h>

namespace Linux {

class Semaphore : public AG_HAL::Semaphore {
public:
    Semaphore();
    bool give() override;
    bool take(uint32_t timeout_ms) override;
    bool take_nonblocking() override;
protected:
    pthread_mutex_t _lock;
};

}
