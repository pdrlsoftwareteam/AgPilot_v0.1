#pragma once

#include <AG_Common/AG_Common.h>
#include <AG_HAL/I2CDevice.h>
#include "AG_BattMonitor_Backend.h"
#include <AG_Param/AG_Param.h>
#include <utility>

#if AP_BATTERY_INA2XX_ENABLED

class AG_BattMonitor_INA2XX : public AG_BattMonitor_Backend
{
public:
    /// Constructor
    AG_BattMonitor_INA2XX(AG_BattMonitor &mon,
                          AG_BattMonitor::BattMonitor_State &mon_state,
                          AG_BattMonitor_Params &params);

    bool has_cell_voltages() const override { return false; }
    bool has_temperature() const override { return false; }
    bool has_current() const override { return true; }
    bool reset_remaining(float percentage) override { return false; }
    bool get_cycle_count(uint16_t &cycles) const override { return false; }

    void init(void) override;
    void read() override;

    static const struct AG_Param::GroupInfo var_info[];

private:
    AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev;

    enum class DevType : uint8_t {
        UNKNOWN = 0,
        INA226,
        INA228,
        INA238,
    };

    static const uint8_t i2c_probe_addresses[];
    uint8_t i2c_probe_next;

    bool configure(DevType dtype);
    bool read_word16(const uint8_t reg, int16_t& data) const;
    bool read_word24(const uint8_t reg, int32_t& data) const;
    bool write_word(const uint8_t reg, const uint16_t data) const;
    void timer(void);
    bool detect_device(void);

    DevType dev_type;
    uint32_t last_detect_ms;

    AP_Int8 i2c_bus;
    AP_Int8 i2c_address;
    AP_Float max_amps;
    AP_Float rShunt;
    uint32_t failed_reads;

    struct {
        uint16_t count;
        float volt_sum;
        float current_sum;
        HAL_Semaphore sem;
    } accumulate;
    float current_LSB;
    float voltage_LSB;
};

#endif // AP_BATTERY_INA2XX_ENABLED
