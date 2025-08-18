#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_HAL/I2CDevice.h>
#include "AP_BattMonitor_Backend.h"
#include <utility>

#if AP_BATTERY_LTC2946_ENABLED

class AP_BattMonitor_LTC2946 : public AP_BattMonitor_Backend
{
public:
    // inherit constructor
    using AP_BattMonitor_Backend::AP_BattMonitor_Backend;

    bool has_cell_voltages() const override { return false; }
    bool has_temperature() const override { return false; }
    bool has_current() const override { return true; }
    bool reset_remaining(float percentage) override { return false; }
    bool get_cycle_count(uint16_t &cycles) const override { return false; }
    bool get_state_of_health(uint16_t &state_of_health) const override { return false; }
    uint8_t get_cell_count(uint8_t &cell_count) const override { return 0; }
    uint16_t get_temp_kelvin(uint16_t &temp_kelvin) const override { return 0; }
    bool get_unique_id(const uint8_t* &unique_id) const override{ return false; }
    bool get_firmware_info(uint16_t &firm_info) const override { return false; }
    bool get_state_of_charge(uint16_t &state_of_charge) const override { return false; }
    bool get_capacity(uint32_t &cap) const override { return false; }
    bool get_temperature2(int16_t &temp) const override { return false; }

    bool get_battery_info(uint16_t &firm_info, const uint8_t* &unique_id) const override { return false; }

    bool get_VI_readings(uint16_t &SOC, uint16_t &SOH, uint32_t &capacity,
                         float &batt_volt, float &batt_curr, float &batt_chr_volt) const override { return false; }

    bool get_min_max_cellVolt(float &max_cell_volt, uint8_t &max_cell_volt_cell_loc, uint8_t &max_cell_volt_cell_ctr,
                              float &min_cell_volt, uint8_t &min_cell_volt_cell_loc, uint8_t &min_cell_volt_cell_ctr) const override { return false; }

    bool get_min_max_temperature(int8_t &max_temp, uint8_t &max_temp_ntc_loc_cell, uint8_t &max_temp_ntc_loc_ctr,
                                 int8_t &min_temp, uint8_t &min_temp_ntc_loc_cell, uint8_t &min_temp_ntc_loc_ctr) const override { return false; }

    bool get_bms_relay_state(uint8_t &bms_state, bool &relay_charge, bool &relay_precharge,
                             bool &relay_negative, bool &relay_positive) const override { return false; }

    bool get_cell_balancing_status(uint16_t &balancing_status_cc1, uint16_t &balancing_status_cc2,
                                   uint16_t &balancing_status_cc3, uint16_t &balancing_status_cc4) const override { return false; }

    bool get_faults_and_warnings(uint32_t &fault_flags, uint32_t &warning_flags) const override { return false; }

    bool get_temp_ntc_cell_count_and_voltages(const int8_t* &temperatures_ntc, uint8_t &cell_count_series,
					      const uint16_t* &cell_voltages) const override { return false; }


    virtual void init(void) override;
    virtual void read() override;
    
private:
    AP_HAL::OwnPtr<AP_HAL::I2CDevice> dev;

    bool read_word(const uint8_t reg, uint16_t& data) const;
    void timer(void);

    struct {
        uint16_t count;
        float volt_sum;
        float current_sum;
        HAL_Semaphore sem;
    } accumulate;
    float current_LSB;
    float voltage_LSB;
};

#endif // AP_BATTERY_LTC2946_ENABLED
