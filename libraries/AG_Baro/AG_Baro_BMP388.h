#pragma once

#include "AG_Baro_Backend.h"

#if AP_BARO_BMP388_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/Device.h>
#include <AG_HAL/utility/OwnPtr.h>

#ifndef HAL_BARO_BMP388_I2C_ADDR
 #define HAL_BARO_BMP388_I2C_ADDR  (0x76)
#endif
#ifndef HAL_BARO_BMP388_I2C_ADDR2
 #define HAL_BARO_BMP388_I2C_ADDR2 (0x77)
#endif

class AG_Baro_BMP388 : public AG_Baro_Backend
{
public:
    AG_Baro_BMP388(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> _dev);

    /* AG_Baro public interface: */
    void update() override;

    static AG_Baro_Backend *probe(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> _dev);

private:

    bool init(void);
    void timer(void);
    void update_temperature(uint32_t);
    void update_pressure(uint32_t);

    AG_HAL::OwnPtr<AG_HAL::Device> dev;

    uint8_t instance;
    float pressure_sum;
    uint32_t pressure_count;
    float temperature;

    // Internal calibration registers
    struct PACKED {
        int16_t nvm_par_p1; // at 0x36
        int16_t nvm_par_p2;
        int8_t nvm_par_p3;
        int8_t nvm_par_p4;
        int16_t nvm_par_p5;
        int16_t nvm_par_p6;
        int8_t nvm_par_p7;
        int8_t nvm_par_p8;
        int16_t nvm_par_p9;
        int8_t nvm_par_p10;
        int8_t nvm_par_p11;
    } calib_p;

    struct PACKED {
        uint16_t nvm_par_t1; // at 0x31
        uint16_t nvm_par_t2;
        int8_t nvm_par_t3;
    } calib_t;

    // scaled calibration data
    struct {
        float par_t1;
        float par_t2;
        float par_t3;
        float par_p1;
        float par_p2;
        float par_p3;
        float par_p4;
        float par_p5;
        float par_p6;
        float par_p7;
        float par_p8;
        float par_p9;
        float par_p10;
        float par_p11;
        float t_lin;
    } calib;

    void scale_calibration_data(void);
    bool read_registers(uint8_t reg, uint8_t *data, uint8_t len);
};

#endif  // AP_BARO_BMP388_ENABLED
