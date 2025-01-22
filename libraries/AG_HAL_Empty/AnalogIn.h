#pragma once

#include "AG_HAL_Empty.h"

class Empty::AnalogSource : public AG_HAL::AnalogSource {
public:
    AnalogSource(float v);
    float read_average() override;
    float read_latest() override;
    bool set_pin(uint8_t p) override;
    float voltage_average() override;
    float voltage_latest() override;
    float voltage_average_ratiometric() override { return voltage_average(); }
private:
    float _v;
};

class Empty::AnalogIn : public AG_HAL::AnalogIn {
public:
    AnalogIn();
    void init() override;
    AG_HAL::AnalogSource* channel(int16_t n) override;
    float board_voltage(void) override;
};
