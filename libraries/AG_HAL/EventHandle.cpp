#include "EventHandle.h"
#include <AG_HAL/AG_HAL.h>


bool AG_HAL::EventHandle::register_event(uint32_t evt_mask)
{
    WITH_SEMAPHORE(sem);
    evt_mask_ |= evt_mask;
    return true;
}

bool AG_HAL::EventHandle::unregister_event(uint32_t evt_mask)
{
    WITH_SEMAPHORE(sem);
    evt_mask_ &= ~evt_mask;
    return true;
}

bool AG_HAL::EventHandle::wait(uint64_t duration)
{
    if (evt_src_ == nullptr) {
        return false;
    }
    return evt_src_->wait(duration, this);
}

bool AG_HAL::EventHandle::set_source(AG_HAL::EventSource* src)
{
    WITH_SEMAPHORE(sem);
    evt_src_ = src;
    evt_mask_ = 0;
    return true;
}
