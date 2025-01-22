/*
 * AG_SBusOut.h
 *
 *  Created on: Aug 19, 2017
 *      Author: Mark Whitehorn
 */

#pragma once

#include <AG_HAL/AG_HAL.h>
#include <AG_Param/AG_Param.h>

class AG_SBusOut {
public:
    AG_SBusOut();

    /* Do not allow copies */
    CLASS_NO_COPY(AG_SBusOut);

    static const struct AG_Param::GroupInfo var_info[];

    void update();

    // public format function for use by IOMCU
    static void sbus_format_frame(uint16_t *channels, uint8_t num_channels, uint8_t buffer[25]);

private:

    AG_HAL::UARTDriver *sbus1_uart;

    void init(void);

    uint16_t sbus_frame_interval;   // microseconds

    AP_Int16 sbus_rate;
    bool initialised;
};
