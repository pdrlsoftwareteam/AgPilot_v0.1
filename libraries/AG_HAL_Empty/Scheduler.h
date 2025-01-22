#pragma once

#include "AG_HAL_Empty.h"

class Empty::Scheduler : public AG_HAL::Scheduler {
public:
    Scheduler() {}
    void     init() override {}
    void     delay(uint16_t ms) override {}
    void     delay_microseconds(uint16_t us) override {}
    void     register_timer_process(AG_HAL::MemberProc) override {}
    void     register_io_process(AG_HAL::MemberProc) override {}

    void     register_timer_failsafe(AG_HAL::Proc, uint32_t period_us) override {}

    void     set_system_initialized() override {}
    bool     is_system_initialized() override { return true; }

    void     reboot(bool hold_in_bootloader) override { for (;;); }

};
