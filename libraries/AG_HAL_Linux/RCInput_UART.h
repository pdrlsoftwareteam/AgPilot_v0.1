#pragma once

#include <AG_Common/AG_Common.h>

#include "AG_HAL_Linux.h"
#include "RCInput.h"

#define CHANNELS 8

namespace Linux {

class RCInput_UART : public RCInput
{
public:
    RCInput_UART(const char *path);
    ~RCInput_UART();

    void init() override;
    void _timer_tick(void) override;

private:
    int _fd;
    uint8_t *_pdata;
    ssize_t _remain;
    struct {
        uint16_t magic;
        uint16_t values[CHANNELS];
    } _data;
};

}
