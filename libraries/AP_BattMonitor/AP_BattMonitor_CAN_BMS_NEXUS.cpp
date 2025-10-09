/*
 * AP_BattMonitor_CAN_BMS_NEXUS.cpp
 *
 *  Created on: Aug 4, 2025
 *      Author: pdrl
 */

#include "AP_BattMonitor_config.h"
#include <AP_HAL/AP_HAL.h>
#include <AP_Common/AP_Common.h>
#include <AP_Math/AP_Math.h>
#include "AP_BattMonitor_CAN_BMS_NEXUS.h"
#include "GCS_MAVLink/GCS.h"
#include <AP_CANManager/AP_CANSensor.h>
#include <stdio.h>
extern const AP_HAL::HAL& hal;

//const AP_Param::GroupInfo AP_BattMonitor_CAN_BMS_NEXUS::var_info[] = {
//
//    AP_GROUPEND
//};

AP_BattMonitor_CAN_BMS_NEXUS::AP_BattMonitor_CAN_BMS_NEXUS(AP_BattMonitor &mon,
						 AP_BattMonitor::BattMonitor_State &mon_state,
						 AP_BattMonitor_Params &params) :
			AP_BattMonitor_Backend(mon, mon_state, params),
			CANSensor("USD1")
{
  //  AP_Param::setup_object_defaults(this, var_info);
  //  _state.var_info = var_info;
  TEST_CAN_NEXUS::getInstance();
  TEST_CAN_NEXUS::getInstance()->set_monitor(this);
}

void AP_BattMonitor_CAN_BMS_NEXUS::read(){
  static uint32_t test = AP_HAL::millis();
    if(!_state.healthy && (AP_HAL::millis() - test > 2000))
      {
        AP_HAL::CANFrame frame1;
        frame1.id = 0x18EFD007 | AP_HAL::CANFrame::FlagEFF;
        frame1.dlc = 8;
        frame1.data[0] = 0x02;
        frame1.data[1] = 0x06;
        frame1.data[2] = 0x9D;
        frame1.data[3] = 0x00;
        frame1.data[4] = 0x00;
        frame1.data[5] = 0x00;
        frame1.data[6] = 0x00;
        frame1.data[7] = 0x00;

        if (TEST_CAN_NEXUS::getInstance()->write_frame(frame1, 5000)) {
//  	                  gcs().send_text(MAV_SEVERITY_INFO, "Frame1 sent");
        } else {
//  	                  gcs().send_text(MAV_SEVERITY_INFO, "Frame1 failed");
        }
        test = AP_HAL::millis();
      }

    const uint32_t tnow_us = AP_HAL::micros();

    // timeout after 5 seconds
    if ((tnow_us - _state.last_time_micros) > 5000000) {
        _state.healthy = false;
    }
}

void AP_BattMonitor_CAN_BMS_NEXUS::read_frame(){

}

//CAN data distributer
TEST_CAN_NEXUS *TEST_CAN_NEXUS::instance = nullptr;
TEST_CAN_NEXUS::TEST_CAN_NEXUS():CANSensor("CAN_BMS")
{
  register_driver(AP_CANManager::Driver_Type_CAN_BMS);

}

void TEST_CAN_NEXUS::handle_frame(AP_HAL::CANFrame &frame)
{
  int32_t raw_id = frame.id_signed();
  monitor->parse_frame(raw_id,frame.data);
}

void AP_BattMonitor_CAN_BMS_NEXUS::parse_frame(uint32_t id, uint8_t* byte) {
  switch (id) {
    case 0x18FF48D0:
      {
	Battery_info.firmware_info = byte[1];

	snprintf((char*)Battery_info.uid_str, sizeof(Battery_info.uid_str), "%u", byte[0]);

//	gcs().send_text(MAV_SEVERITY_INFO,"info: %f %f",byte[0]*0.1, byte[1]*0.1);

      }
      break;
    case 0x18FF05D0:
      {
	Battery_info.SOC = (byte[0]<<8 | byte[1]) *0.01;
//	gcs().send_text(MAV_SEVERITY_INFO,"SOC: %u",Battery_info.SOC);

	Battery_info.SOH = byte[2];
//	gcs().send_text(MAV_SEVERITY_INFO,"SOH: %u",Battery_info.SOH);
      }
      break;
    case 0x18FF3FDC:
      {
	Battery_info.capacity = byte[3] * 1000;
//	gcs().send_text(MAV_SEVERITY_INFO,"Capacity: %lu",Battery_info.capacity);
	cycle_count = (byte[7] << 8) | byte[6];
//	gcs().send_text(MAV_SEVERITY_INFO,"cycle_count: %u",cycle_count);
      }
      break;
    case 0x18FF33D0:
      {

//	gcs().send_text(MAV_SEVERITY_INFO,"Remaining_Capacity: %u %u %u",(byte[0]<<8 | byte[1]), byte[0] , byte[1]);
//	gcs().send_text(MAV_SEVERITY_INFO,"Usable_Capacity: %u Ah",byte[7]);
	Battery_info.remaining_capacity = byte[7] * 1000;

	// Check if total and remaining values are valid
	if (_params._pack_capacity > 0) {

	    // If remaining is greater than total, treat as invalid and skip update
	    if (Battery_info.remaining_capacity > _params._pack_capacity) {
		// Optionally log or clamp values
		Battery_info.remaining_capacity = _params._pack_capacity;
	    }

	    // Calculate consumed mAh directly
	    _state.consumed_mah = _params._pack_capacity - Battery_info.remaining_capacity;
	}
	else
	  {
	    _state.consumed_mah = 0;
	  }
      }
      break;
    case 0x18FF8284:
      {
	Battery_info.batt_curr = (byte[7]<<8 | byte[6]) *0.1;
//	gcs().send_text(MAV_SEVERITY_INFO,"battery_current: %f",Battery_info.batt_curr);
	_state.current_amps = Battery_info.batt_curr;
	_has_current = true;

      }
      break;
    case 0x18FF01D0:
      {
	Battery_info.batt_volt = (byte[0]<<24 | byte[1]<<16 | byte[2]<<8 | byte[3]) * 0.01;
//	gcs().send_text(MAV_SEVERITY_INFO,"voltage: %f",Battery_info.batt_volt);
	_state.voltage = Battery_info.batt_volt;
	_state.healthy = true;

      }
      break;
    case 0x18EFD107:
      {
	Battery_info.cell_count_series = 0;
	Battery_info.cell_voltages[0] = (byte[0]<<8 | byte[1]);
	Battery_info.cell_voltages[1] = (byte[2]<<8 | byte[3]);
	Battery_info.cell_voltages[2] = (byte[4]<<8 | byte[5]);
	Battery_info.cell_voltages[3] = (byte[6]<<8 | byte[7]);
	_state.cell_voltages.cells[0] = Battery_info.cell_voltages[0];
	_state.cell_voltages.cells[1] = Battery_info.cell_voltages[1];
	_state.cell_voltages.cells[2] = Battery_info.cell_voltages[2];
	_state.cell_voltages.cells[3] = Battery_info.cell_voltages[3];
	Battery_info.cell_count_series+=4;
//	gcs().send_text(MAV_SEVERITY_INFO,"cell voltage: 0:%u\t1:%u\t2:%u\t3:%u",
//			Battery_info.cell_voltages[0],
//			Battery_info.cell_voltages[1],
//			Battery_info.cell_voltages[2],
//			Battery_info.cell_voltages[3]);
	_has_cell_voltages = true;
      }
      break;
    case 0x18EFD207:
      {
	Battery_info.cell_voltages[4] = (byte[0]<<8 | byte[1]);
	Battery_info.cell_voltages[5] = (byte[2]<<8 | byte[3]);
	Battery_info.cell_voltages[6] = (byte[4]<<8 | byte[5]);
	Battery_info.cell_voltages[7] = (byte[6]<<8 | byte[7]);
	_state.cell_voltages.cells[4] = Battery_info.cell_voltages[4];
	_state.cell_voltages.cells[5] = Battery_info.cell_voltages[5];
	_state.cell_voltages.cells[6] = Battery_info.cell_voltages[6];
	_state.cell_voltages.cells[7] = Battery_info.cell_voltages[7];
	Battery_info.cell_count_series+=4;
//	gcs().send_text(MAV_SEVERITY_INFO,"cell voltage: 4:%u\t5:%u\t6:%u\t7:%u",
//			Battery_info.cell_voltages[4],
//			Battery_info.cell_voltages[5],
//			Battery_info.cell_voltages[6],
//			Battery_info.cell_voltages[7]);
	_has_cell_voltages = true;
	}
      break;
    case 0x18EFD307:
      {
	Battery_info.cell_voltages[8] = (byte[0]<<8 | byte[1]);
	Battery_info.cell_voltages[9] = (byte[2]<<8 | byte[3]);
	Battery_info.cell_voltages[10] = (byte[4]<<8 | byte[5]);
	Battery_info.cell_voltages[11] = (byte[6]<<8 | byte[7]);
	_state.cell_voltages.cells[8] = Battery_info.cell_voltages[8];
	_state.cell_voltages.cells[9] = Battery_info.cell_voltages[9];
	_state.cell_voltages.cells[10] = Battery_info.cell_voltages[10];
	_state.cell_voltages.cells[11] = Battery_info.cell_voltages[11];
	Battery_info.cell_count_series+=4;
//	gcs().send_text(MAV_SEVERITY_INFO,"cell voltage: 8:%u\t9:%u\t10:%u\t11:%u",
//			Battery_info.cell_voltages[8],
//			Battery_info.cell_voltages[9],
//			Battery_info.cell_voltages[10],
//			Battery_info.cell_voltages[11]);
	_has_cell_voltages = true;
      }
      break;
    case 0x18FF46D0:
      {
	_state.temperature = (int8_t)((byte[0]<<8 | byte[1])*0.1);
	Battery_info.min_temp = (int8_t)((byte[4]<<8 | byte[5])*0.1);
	Battery_info.max_temp = (int8_t)((byte[2]<<8 | byte[3])*0.1);
	_has_temperature = true;
//	gcs().send_text(MAV_SEVERITY_INFO,"PDU_Temp: %f",(byte[0]<<8 | byte[1])*0.1);
//	gcs().send_text(MAV_SEVERITY_INFO,"Max_temp: %f",(byte[2]<<8 | byte[3])*0.1);
//	gcs().send_text(MAV_SEVERITY_INFO,"Min_temp: %f",(byte[4]<<8 | byte[5])*0.1);

      }
      break;
    case 0x18FF31D0:
      {
	Battery_info.temperatures_ntc[0] = (int8_t)((byte[2]<<8 | byte[3])*0.1);
	Battery_info.temperatures_ntc[1] = (int8_t)((byte[4]<<8 | byte[5])*0.1);
	Battery_info.temperatures_ntc[2] = (int8_t)((byte[6]<<8 | byte[7])*0.1);
	Battery_info.temperatures_ntc[3] = (int8_t)((byte[0]<<8 | byte[1])*0.1);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_ntc_0: %d",Battery_info.temperatures_ntc[0]);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_ntc_1: %d",Battery_info.temperatures_ntc[1]);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_ntc_2: %d",Battery_info.temperatures_ntc[2]);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_ntc_3: %d",Battery_info.temperatures_ntc[3]);
      }
      break;
    case 0x18FF31D1:
      {
	Battery_info.temperatures_ntc[4] = (int8_t)((byte[0]<<8 | byte[1])*0.1);
	Battery_info.temperatures_ntc[5] = (int8_t)((byte[2]<<8 | byte[3])*0.1);
	Battery_info.temperatures_ntc[6] = (int8_t)((byte[4]<<8 | byte[5])*0.1);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_ntc_4: %d",Battery_info.temperatures_ntc[4]);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_ntc_5: %d",Battery_info.temperatures_ntc[5]);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_ntc_6: %d",Battery_info.temperatures_ntc[6]);
      }
      break;
    case 0x18FF03D0:
      {
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_cell_min: %d",Battery_info.min_temp);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_cell_max: %d",Battery_info.max_temp);
//	gcs().send_text(MAV_SEVERITY_INFO,"temp_cell_mean: %f",(byte[4]<<8 | byte[5])*0.1);
	_has_temperature = true;
      }
      break;
    default:
      {
//	 gcs().send_text(MAV_SEVERITY_INFO,"defId:%X 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d",
//	    						(unsigned int)id&0x1FFFFFFF,byte[0],byte[1],byte[2],byte[3],byte[4],byte[5],byte[6],byte[7]);

      }
      break;
  }
  const uint32_t tnow_us = AP_HAL::micros();
//  const uint32_t dt_us = tnow_us - _state.last_time_micros;
  if (_state.healthy && has_current()) {
      update_consumed_from_remaining(_state);
      _state.last_time_micros = tnow_us;
  }
  //  send_gcs_bms_status();
}

void AP_BattMonitor_CAN_BMS_NEXUS::handle_frame(AP_HAL::CANFrame &frame)
{

}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_unique_id(const uint8_t* &uniq_id) const
{
  uniq_id = Battery_info.uid_str;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_firmware_info(uint16_t &firm_info) const
{
  firm_info = Battery_info.firmware_info;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_state_of_health(uint16_t &state_of_health) const
{
  state_of_health = Battery_info.SOH;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_state_of_charge(uint16_t &state_of_charge) const
{
  state_of_charge = Battery_info.SOC;
  return true;
}
bool AP_BattMonitor_CAN_BMS_NEXUS::get_capacity(uint32_t &cap) const
{
  cap = Battery_info.capacity;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_remaining_capacity(uint32_t &rem_cap) const
{
  rem_cap = Battery_info.remaining_capacity;
  return true;
}

uint8_t AP_BattMonitor_CAN_BMS_NEXUS::get_cell_count(uint8_t &count_cell) const
{
  count_cell = Battery_info.cell_count_series;
  return count_cell;
}
bool AP_BattMonitor_CAN_BMS_NEXUS::get_temperature2(int16_t &temp) const
{
  temp = Battery_info.temperatures_ntc[1];
  return true;
}
bool AP_BattMonitor_CAN_BMS_NEXUS::has_temperature() const
{
  return _has_temperature;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_battery_info(uint16_t &firm_info,const uint8_t* &uniq_id) const
{
  firm_info = Battery_info.firmware_info;
  uniq_id = Battery_info.uid_str;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_VI_readings(uint16_t &SOC, uint16_t &SOH, uint32_t &capacity,
                     float &batt_volt, float &batt_curr, float &batt_chr_volt) const
{
  SOC = Battery_info.SOC;
  SOH = Battery_info.SOH;
  capacity = Battery_info.capacity;
  batt_volt = Battery_info.batt_volt;
  batt_curr = Battery_info.batt_curr;
  batt_chr_volt = Battery_info.batt_chr_volt;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_min_max_cellVolt(float &max_cell_volt, uint8_t &max_cell_volt_cell_loc, uint8_t &max_cell_volt_cell_ctr,
                          float &min_cell_volt, uint8_t &min_cell_volt_cell_loc, uint8_t &min_cell_volt_cell_ctr) const
{
  max_cell_volt = Battery_info.max_cell_volt;
  max_cell_volt_cell_loc = Battery_info.max_cell_volt_cell_loc;
  max_cell_volt_cell_ctr = Battery_info.max_cell_volt_cell_ctr;
  min_cell_volt = Battery_info.min_cell_volt;
  min_cell_volt_cell_loc = Battery_info.min_cell_volt_cell_loc;
  min_cell_volt_cell_ctr = Battery_info.min_cell_volt_cell_ctr;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_min_max_temperature(int8_t &max_temp, uint8_t &max_temp_ntc_loc_cell, uint8_t &max_temp_ntc_loc_ctr,
                             int8_t &min_temp, uint8_t &min_temp_ntc_loc_cell, uint8_t &min_temp_ntc_loc_ctr) const
{
  max_temp = Battery_info.max_temp;
  max_temp_ntc_loc_cell = Battery_info.max_temp_ntc_loc_cell;
  max_temp_ntc_loc_ctr = Battery_info.max_temp_ntc_loc_ctr;
  min_temp = Battery_info.min_temp;
  min_temp_ntc_loc_cell = Battery_info.min_temp_ntc_loc_cell;
  min_temp_ntc_loc_ctr = Battery_info.min_temp_ntc_loc_ctr;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_bms_relay_state(uint8_t &bms_state, bool &relay_charge, bool &relay_precharge,
                         bool &relay_negative, bool &relay_positive) const
{
  bms_state = Battery_info.bms_state;
  relay_charge = Battery_info.relay_charge;
  relay_precharge = Battery_info.relay_precharge;
  relay_negative = Battery_info.relay_negative;
  relay_positive = Battery_info.relay_positive;
  return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_cell_balancing_status(uint16_t &balancing_status_cc1, uint16_t &balancing_status_cc2,
                               uint16_t &balancing_status_cc3, uint16_t &balancing_status_cc4) const
{
    balancing_status_cc1 = Battery_info.balancing_status_cc1;
    balancing_status_cc2 = Battery_info.balancing_status_cc2;
    balancing_status_cc3 = Battery_info.balancing_status_cc3;
    balancing_status_cc4 = Battery_info.balancing_status_cc4;
    return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_faults_and_warnings(uint32_t &fault_flags, uint32_t &warning_flags) const
{
    fault_flags = Battery_info.fault_flags;
    warning_flags = Battery_info.warning_flags;
    return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_temp_ntc_cell_count_and_voltages(const int8_t* &temperatures_ntc, uint8_t &cell_count_series,
                                          const uint16_t* &cell_voltages) const
{
    temperatures_ntc = Battery_info.temperatures_ntc;
    cell_count_series = Battery_info.cell_count_series;
    cell_voltages = Battery_info.cell_voltages;
    return true;
}

bool AP_BattMonitor_CAN_BMS_NEXUS::get_cycle_count(uint16_t &cycles) const
{
    cycles = cycle_count;
    return true;
}

bool TEST_CAN_NEXUS::write_frame(AP_HAL::CANFrame &out_frame, const uint64_t timeout_us)
{
    // send via underlying CAN driver
    // Example: use CANSensor's send API (assuming it exists)
    return CANSensor::write_frame(out_frame, timeout_us);
}

void AP_BattMonitor_CAN_BMS_NEXUS::send_gcs_bms_status()
{
//  static uint32_t t = AP_HAL::millis();
//  if(AP_HAL::millis() - t >2000)
//    {
//      t = AP_HAL::millis();
//      // Unique ID
//      char uid_str[40] = "UID: ";
//      for (uint8_t i = 0; i < 16; i++) {
//	  char hex[4];
//	  snprintf(hex, sizeof(hex), "%02X", unique_id[i]);
//	  strncat(uid_str, hex, sizeof(uid_str) - strlen(uid_str) - 1);
//      }
//      gcs().send_text(MAV_SEVERITY_INFO, "%s", uid_str);
//
//      // Firmware + State
//      gcs().send_text(MAV_SEVERITY_INFO, "FW: %u | BMS State: %u", firmware_info, bms_state);
//
//      // SOC, SOH, Capacity
//      gcs().send_text(MAV_SEVERITY_INFO, "SOC: %u%% | SOH: %u%% | Capacity: %lu mAh", SOC, SOH, capacity);
//
//      // Voltage & Current
//      gcs().send_text(MAV_SEVERITY_INFO, "V: %f V | I: %f A | Charge I: %f V",
//		      batt_volt, batt_curr, batt_chr_volt);
//
//      // Cell Extremes
//      gcs().send_text(MAV_SEVERITY_INFO, "MaxCell: %f V [Loc:%u Ctr:%u]",
//		      max_cell_volt, max_cell_volt_cell_loc, max_cell_volt_cell_ctr);
//      gcs().send_text(MAV_SEVERITY_INFO, "MinCell: %f V [Loc:%u Ctr:%u]",
//		      min_cell_volt, min_cell_volt_cell_loc, min_cell_volt_cell_ctr);
//
//      // Temperature
//      gcs().send_text(MAV_SEVERITY_INFO, "Temp1: %lf°C", _state.temperature);
//      gcs().send_text(MAV_SEVERITY_INFO, "MaxT: %d°C [Cell:%u Ctr:%u]",
//		      max_temp, max_temp_ntc_loc_cell, max_temp_ntc_loc_ctr);
//      gcs().send_text(MAV_SEVERITY_INFO, "MinT: %d°C [Cell:%u Ctr:%u]",
//		      min_temp, min_temp_ntc_loc_cell, min_temp_ntc_loc_ctr);
//
//      // Relay Status
//      gcs().send_text(MAV_SEVERITY_INFO, "Relays: Pre:%d Chg:%d Neg:%d Pos:%d",
//		      relay_precharge, relay_charge, relay_negative, relay_positive);
//
//      // Cell Balancing
//      gcs().send_text(MAV_SEVERITY_INFO, "Balancing: CC1=0x%04X CC2=0x%04X CC3=0x%04X CC4=0x%04X",
//		      balancing_status_cc1, balancing_status_cc2,
//		      balancing_status_cc3, balancing_status_cc4);
//
//      // Faults and Warnings
//      gcs().send_text(MAV_SEVERITY_INFO, "Faults: 0x%08lX | Warnings: 0x%08lX",
//		      fault_flags, warning_flags);
//
//      // NTC Temperatures
//      for (uint8_t i = 0; i < 7; i++) {
//	  gcs().send_text(MAV_SEVERITY_INFO, "NTC[%u]: %d°C", i+1, temperatures_ntc[i]);
//      }
//
//      // Cell Voltages
//      for (uint8_t i = 0; i < cell_count_series; i++) {
//	  gcs().send_text(MAV_SEVERITY_INFO, "Cell[%u]: %u mV", i, _state.cell_voltages.cells[i]);
//      }
//    }
}

