/*
 * AP_BattMonitor_CAN_BMS_NEXUS.h
 *
 * Created on: Aug 4, 2025
 * Author: pdrl
 */

#pragma once

#include "AP_BattMonitor_Backend.h"
#include "AP_BattMonitor.h"
#include <AP_CANManager/AP_CANSensor.h>

class AP_BattMonitor_CAN_BMS_NEXUS : public AP_BattMonitor_Backend, public CANSensor {
public:
    // Constructor
    AP_BattMonitor_CAN_BMS_NEXUS(AP_BattMonitor &mon,
                            AP_BattMonitor::BattMonitor_State &mon_state,
                            AP_BattMonitor_Params &params);

    // Called at 10 Hz to read battery status
    void read() override;

    // Frame handler
    void handle_frame(AP_HAL::CANFrame &frame) override;

    // Optional read_frame method
    void read_frame();

    // Parse incoming CAN frame data
    void parse_frame(uint32_t id, uint8_t* byte);


    bool get_unique_id(const uint8_t* &uniq_id) const override;
    bool get_firmware_info(uint16_t &firm_info) const override;
    bool get_state_of_health(uint16_t &state_of_health) const override;
    bool get_state_of_charge(uint16_t &state_of_charge) const override;
    bool get_capacity(uint32_t &cap) const override;
    uint8_t get_cell_count(uint8_t &count_cell) const override;
    bool get_temperature2(int16_t &temp) const override;
    bool get_remaining_capacity(uint32_t &rem_cap) const override;
    bool get_battery_info(uint16_t &firm_info,const uint8_t* &uniq_id) const override;

    bool get_VI_readings(uint16_t &SOC, uint16_t &SOH, uint32_t &capacity,
                         float &batt_volt, float &batt_curr, float &batt_chr_volt) const override;

    bool get_min_max_cellVolt(float &max_cell_volt, uint8_t &max_cell_volt_cell_loc, uint8_t &max_cell_volt_cell_ctr,
                              float &min_cell_volt, uint8_t &min_cell_volt_cell_loc, uint8_t &min_cell_volt_cell_ctr) const override;

    bool get_min_max_temperature(int8_t &max_temp, uint8_t &max_temp_ntc_loc_cell, uint8_t &max_temp_ntc_loc_ctr,
                                 int8_t &min_temp, uint8_t &min_temp_ntc_loc_cell, uint8_t &min_temp_ntc_loc_ctr) const override;

    bool get_bms_relay_state(uint8_t &bms_state, bool &relay_charge, bool &relay_precharge,
                             bool &relay_negative, bool &relay_positive) const override;

    bool get_cell_balancing_status(uint16_t &balancing_status_cc1, uint16_t &balancing_status_cc2,
                                   uint16_t &balancing_status_cc3, uint16_t &balancing_status_cc4) const override;

    bool get_faults_and_warnings(uint32_t &fault_flags, uint32_t &warning_flags) const override;

    bool get_temp_ntc_cell_count_and_voltages(const int8_t* &temperatures_ntc, uint8_t &cell_count_series,
					      const uint16_t* &cell_voltages) const override;

    bool get_cycle_count(uint16_t &cycles) const override;

    bool has_temperature() const override;
    void send_gcs_bms_status();

    // Battery capabilities
    bool has_current() const override { return _has_current; }
    bool has_consumed_energy() const override { return has_current(); }
    bool has_cell_voltages() const override { return _has_cell_voltages; }


    // Optional init if needed
    void init() override {}

    typedef struct __attribute__((packed)) {
            // ID 0x12C - Firmware Info and 16-byte Unique ID
            uint16_t firmware_info;       // divide by 100

            // ID 0x12D and 0x12E
            uint8_t uid_str[33];

            // ID 0x130 - SoC, SoH, Capacity
            uint16_t SOC;                 // divide by 10
            uint16_t SOH;                 // divide by 10
            uint32_t capacity;            // divide by 1000 (mAh)
            uint32_t remaining_capacity;  // divide by 1000 (mAh)

            // ID 0x131 - Battery Voltage, Current, Charger Voltage
            float batt_volt;              // divide by 1000 (mV)
            float batt_curr;              // divide by 1000 then subtract 1500 (mA)
            float batt_chr_volt;          // divide by 100 (mV)

            // ID 0x132 - Cell Voltage Extremes and Locations
            float max_cell_volt;          // divide by 1000 (mV)
            uint8_t max_cell_volt_cell_loc;
            uint8_t max_cell_volt_cell_ctr;
            float min_cell_volt;          // divide by 1000 (mV)
            uint8_t min_cell_volt_cell_loc;
            uint8_t min_cell_volt_cell_ctr;

            // ID 0x133 - Temperature Extremes and Locations
            int8_t max_temp;              // offset -128
            uint8_t max_temp_ntc_loc_cell;
            uint8_t max_temp_ntc_loc_ctr;
            int8_t min_temp;              // offset -128
            uint8_t min_temp_ntc_loc_cell;
            uint8_t min_temp_ntc_loc_ctr;

            // ID 0x134 - BMS and Relay States
            uint8_t bms_state;
            bool relay_precharge;
            bool relay_charge;
            bool relay_negative;
            bool relay_positive;

            // ID 0x135 - Cell Balancing Status
            uint16_t balancing_status_cc1;
            uint16_t balancing_status_cc2;
            uint16_t balancing_status_cc3;
            uint16_t balancing_status_cc4;

            // ID 0x136 - Faults and Warnings
            uint32_t fault_flags;
            uint32_t warning_flags;

            // ID 0x137+i - Temperatures from each NTC (offset -128)
            int8_t temperatures_ntc[7];   // NTC1 - NTC7

            // ID 0x150+i and 0x151+i - Cell Voltages
            uint16_t cell_voltages[14];   // 14 per controller, max 2 controllers (use 12 if 12S only)
            uint8_t cell_count_series;

        } BMS_Status_t;

        BMS_Status_t Battery_info= {};

    uint8_t unique_id[16];
    // For Internal Use
    bool _has_current;
    bool _has_temperature;
    bool _has_cell_voltages;
    uint16_t cycle_count;

};


class TEST_CAN_NEXUS : public CANSensor {
public:
    // Singleton access
    static TEST_CAN_NEXUS* getInstance() {
        if (instance == nullptr) {
            instance = new TEST_CAN_NEXUS();
        }
        return instance;
    }

    // Constructor
    TEST_CAN_NEXUS();

    // Set reference to battery monitor
    void set_monitor(AP_BattMonitor_CAN_BMS_NEXUS* mon) { monitor = mon; }

    // Handle incoming CAN frames
    void handle_frame(AP_HAL::CANFrame &frame) override;
    bool write_frame(AP_HAL::CANFrame &out_frame, const uint64_t timeout_us);



private:
    static TEST_CAN_NEXUS* instance;
    AP_BattMonitor_CAN_BMS_NEXUS* monitor = nullptr;
};
