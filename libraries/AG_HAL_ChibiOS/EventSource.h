#pragma once
#include <AG_HAL/AG_HAL.h>

#include <stdint.h>
#include <AG_HAL/AG_HAL_Boards.h>
#include <AG_HAL/AG_HAL_Macros.h>
#include <AG_HAL/EventHandle.h>
#include "AG_HAL_ChibiOS_Namespace.h"
#include <ch.hpp>

#if CH_CFG_USE_EVENTS == TRUE
class ChibiOS::EventSource : public AG_HAL::EventSource {
    // Single event source to be shared across multiple users
    chibios_rt::EventSource ch_evt_src_;

public:
    // generate event from thread context
    void signal(uint32_t evt_mask) override;

    // generate event from interrupt context
    void signalI(uint32_t evt_mask) override;

    // Wait on an Event handle, method for internal use by EventHandle
    bool wait(uint64_t duration, AG_HAL::EventHandle* evt_handle) override;
};
#endif //#if CH_CFG_USE_EVENTS == TRUE
