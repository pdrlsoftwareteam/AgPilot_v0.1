/*
* AP_BattMonitor_CAN_BMS.cpp
*
*  Created on: Aug 4, 2025
*      Author: pdrl
*/

#include "AP_BattMonitor_config.h"
#include <AP_HAL/AP_HAL.h>
#include <AP_Common/AP_Common.h>
#include <AP_Math/AP_Math.h>
#include "AP_BattMonitor_CAN_BMS.h"
#include "GCS_MAVLink/GCS.h"
#include <AP_CANManager/AP_CANSensor.h>
#include <stdio.h>
extern const AP_HAL::HAL& hal;

//const AP_Param::GroupInfo AP_BattMonitor_CAN_BMS::var_info[] = {
//
//    AP_GROUPEND
//};

AP_BattMonitor_CAN_BMS::AP_BattMonitor_CAN_BMS(AP_BattMonitor &mon,
						 AP_BattMonitor::BattMonitor_State &mon_state,
						 AP_BattMonitor_Params &params) :
			AP_BattMonitor_Backend(mon, mon_state, params),
			CANSensor("USD1")
{
  //  AP_Param::setup_object_defaults(this, var_info);
  //  _state.var_info = var_info;
  TEST_CAN::getInstance();
  TEST_CAN::getInstance()->set_monitor(this);
}

void AP_BattMonitor_CAN_BMS::read(){
  const uint32_t tnow_us = AP_HAL::micros();

  // timeout after 5 seconds
  if ((tnow_us - _state.last_time_micros) > 10000000) {
      _state.healthy = false;
  }
}

void AP_BattMonitor_CAN_BMS::read_frame(){

}

//CAN data distributer
TEST_CAN *TEST_CAN::instance = nullptr;
TEST_CAN::TEST_CAN():CANSensor("CAN_BMS")
{
  register_driver(AP_CANManager::Driver_Type_CAN_BMS);

}

void TEST_CAN::handle_frame(AP_HAL::CANFrame &frame)
{
  int32_t raw_id = frame.id_signed();
//  gcs().send_text(MAV_SEVERITY_INFO, "Prajwal Tayade");
  monitor->parse_frame(raw_id,frame.data);
}

void AP_BattMonitor_CAN_BMS::parse_frame(uint32_t id, uint8_t* byte) {
  switch (id) {
    case 0x12C: // Firmware version
      {
	Battery_info.firmware_info = ((byte[1] << 8) | byte[0]) / 100;
	//	gcs().send_text(MAV_SEVERITY_INFO, "FW: %u ", firmware_info);
      }
      break;

    case 0x12D: // UID High
      {
//	gcs().send_text(MAV_SEVERITY_INFO, "lower: %X %X %X %X %X %X %X %X ",byte[0],byte[1],byte[2],byte[3],byte[4],byte[5],byte[6],byte[7]);
	memcpy(&unique_id[0], byte, 8);
      }
      break;

    case 0x12E: // UID Low
      {
//	gcs().send_text(MAV_SEVERITY_INFO, "higher: %X %X %X %X %X %X %X %X ",byte[0],byte[1],byte[2],byte[3],byte[4],byte[5],byte[6],byte[7]);
	memcpy(&unique_id[8], byte, 8);
	for (uint8_t i = 0; i < 16; i++) {
	    char hex[4];
	    snprintf(hex, sizeof(hex), "%02X", unique_id[i]);
	    strncat((char*)Battery_info.uid_str, hex, sizeof(Battery_info.uid_str) - strlen((const char*)Battery_info.uid_str) - 1);
	}
	Battery_info.uid_str[32] = '\0';
//		gcs().send_text(MAV_SEVERITY_INFO, "%s", Battery_info.uid_str);
      }
      break;

    case 0x130: // SOC, SOH, Capacity
      {
	Battery_info.SOC = ((byte[1] << 8) | byte[0]) / 10;
	Battery_info.SOH = ((byte[3] << 8) | byte[2]) / 10;
	Battery_info.capacity = ((byte[6] << 16) | (byte[5] << 8) | byte[4]);
	//	gcs().send_text(MAV_SEVERITY_INFO, "SOC: %u  SOH: %u  Capacity: %lu mAh", SOC, SOH, capacity);
	gcs().send_text(MAV_SEVERITY_INFO, "SOC: %u  SOH: %u  Capacity: %lu mAh", Battery_info.SOC, Battery_info.SOH, Battery_info.capacity);
	Battery_info.remaining_capacity = _params._pack_capacity - _state.consumed_mah;
	if (_params._pack_capacity > 0) {

	    // If remaining is greater than total, treat as invalid and skip update
	    if (Battery_info.remaining_capacity > (uint32_t)_params._pack_capacity) {
		// Optionally log or clamp values
		Battery_info.remaining_capacity = _params._pack_capacity;
	    }
	}
	else
	  {
	    Battery_info.remaining_capacity = 0;
	  }
      }
      break;

    case 0x131: // Volt, Curr, Charger Volt
      {
	Battery_info.batt_volt = ((byte[2] << 16) | (byte[1] << 8) | byte[0]) / 1000;
	Battery_info.batt_curr = (((byte[5] << 16) | (byte[4] << 8) | byte[3]) / 1000) - 1500;
	Battery_info.batt_chr_volt = ((byte[7] << 8) | byte[6]) / 100;
	_state.voltage = Battery_info.batt_volt;
	_state.current_amps = Battery_info.batt_curr;
	_has_current = true;
	_state.healthy = true;
	//	gcs().send_text(MAV_SEVERITY_INFO, "batt Volt: %fV  batt curr: %fAmp  Charger Volt: %fV",batt_volt, batt_curr, batt_chr_volt);
	gcs().send_text(MAV_SEVERITY_INFO, "batt Volt: %fV  batt curr: %fAmp  Charger Volt: %fV",Battery_info.batt_volt, Battery_info.batt_curr, Battery_info.batt_chr_volt);
      }
      break;

    case 0x132: // Max/Min cell volt info
      {
	Battery_info.max_cell_volt = ((byte[1] << 8) | byte[0]) / 1000;
	Battery_info.max_cell_volt_cell_loc = byte[2] >> 4;
	Battery_info.max_cell_volt_cell_ctr = byte[2] & 0x0F;
	Battery_info.min_cell_volt = ((byte[4] << 8) | byte[3]) / 1000;
	Battery_info.min_cell_volt_cell_loc = byte[5] >> 4;
	Battery_info.min_cell_volt_cell_ctr = byte[5] & 0x0F;
	//	gcs().send_text(MAV_SEVERITY_INFO, "MaxCell: %f V [Loc:%u Ctr:%u]", max_cell_volt, max_cell_volt_cell_loc, max_cell_volt_cell_ctr);
	//	gcs().send_text(MAV_SEVERITY_INFO, "MinCell: %f V [Loc:%u Ctr:%u]", min_cell_volt, min_cell_volt_cell_loc, min_cell_volt_cell_ctr);
	//	gcs().send_text(MAV_SEVERITY_INFO, "minmaxcellvolt: %X %X %X %X %X %X %X %X ",byte[0],byte[1],byte[2],byte[3],byte[4],byte[5],byte[6],byte[7]);

      }
      break;

    case 0x133: // Max/Min temp info
      {
	Battery_info.max_temp = byte[0] - 128;
	Battery_info.max_temp_ntc_loc_cell = byte[1] >> 4;
	Battery_info.max_temp_ntc_loc_ctr = byte[1] & 0x0F;
	Battery_info.min_temp = byte[2] - 128;
	Battery_info.min_temp_ntc_loc_cell = byte[3] >> 4;
	Battery_info.min_temp_ntc_loc_ctr = byte[3] & 0x0F;
	//	gcs().send_text(MAV_SEVERITY_INFO, "MaxT: %d°C [Cell:%u Ctr:%u]", max_temp, max_temp_ntc_loc_cell, max_temp_ntc_loc_ctr);
	//	gcs().send_text(MAV_SEVERITY_INFO, "MinT: %d°C [Cell:%u Ctr:%u]", min_temp, min_temp_ntc_loc_cell, min_temp_ntc_loc_ctr);
	//	gcs().send_text(MAV_SEVERITY_INFO, "minmaxtemp: %X %X %X %X %X %X %X %X ",byte[0],byte[1],byte[2],byte[3],byte[4],byte[5],byte[6],byte[7]);

      }
      break;

    case 0x134: // Relay states
      {
	Battery_info.bms_state = byte[0];
	Battery_info.relay_precharge = byte[1] >> 4;
	Battery_info.relay_charge = byte[1] & 0x0F;
	Battery_info.relay_negative = byte[2] >> 4;
	Battery_info.relay_positive = byte[2] & 0x0F;
	//	gcs().send_text(MAV_SEVERITY_INFO, "Relay state :%d  Pre:%d  Chg:%d  Neg:%d  Pos:%d",bms_state, relay_precharge, relay_charge, relay_negative, relay_positive);

      }
      break;

    case 0x135: // Cell balancing
      {
	Battery_info.balancing_status_cc1 = (byte[1] << 8) | byte[0];
	Battery_info.balancing_status_cc2 = (byte[3] << 8) | byte[2];
	Battery_info.balancing_status_cc3 = (byte[5] << 8) | byte[4];
	Battery_info.balancing_status_cc4 = (byte[7] << 8) | byte[6];
	//	gcs().send_text(MAV_SEVERITY_INFO, "Balancing: CC1=0x%04X CC2=0x%04X CC3=0x%04X CC4=0x%04X",
	//			balancing_status_cc1, balancing_status_cc2,
	//			balancing_status_cc3, balancing_status_cc4);
	//	gcs().send_text(MAV_SEVERITY_INFO, "cellblnc: %X %X %X %X %X %X %X %X ",byte[0],byte[1],byte[2],byte[3],byte[4],byte[5],byte[6],byte[7]);

      }
      break;

    case 0x136: // Fault/Warning
      {
	Battery_info.fault_flags = (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | byte[0];
	Battery_info.warning_flags = (byte[7] << 24) | (byte[6] << 16) | (byte[5] << 8) | byte[4];
	//	gcs().send_text(MAV_SEVERITY_INFO, "Faults: 0x%X  Warnings: 0x%X", (unsigned)fault_flags, (unsigned)warning_flags);
      }
      break;

    case 0x137: // NTC 1-7 temperatures
      {
	for (int i = 0; i < 7; i++) {
	    Battery_info.temperatures_ntc[i] = byte[i] - 128;
	}
	_state.temperature = Battery_info.temperatures_ntc[0];
	_has_temperature = true;
	// NTC Temperatures
	//	for (uint8_t i = 0; i < 7; i++) {
	//	    gcs().send_text(MAV_SEVERITY_INFO, "NTC[%u]: %d°C", i, temperatures_ntc[i]);
	//	}
      }
      break;
    case 0x150:
	{
	  _cell_index = 0;

	  for (uint8_t i = 0; i < 8; i++) {
		  if (byte[i] == 0) continue;  // skip zero bytes anywhere in frame
		  if (_cell_index >= 14) break; // safety guard
		  _state.cell_voltages.cells[_cell_index] = (uint16_t)byte[i] * 10;
		  Battery_info.cell_voltages[_cell_index] = (uint16_t)byte[i] * 10;
		  _cell_index++;
	  }
	}
	break;

	case 0x151:
	{
	  for (uint8_t i = 0; i < 8; i++) {
		  if (byte[i] == 0) continue;  // skip zero bytes
		  if (_cell_index >= 14) break; // safety guard
		  _state.cell_voltages.cells[_cell_index] = (uint16_t)byte[i] * 10;
		  Battery_info.cell_voltages[_cell_index] = (uint16_t)byte[i] * 10;
		  _cell_index++;
	  }

	  Battery_info.cell_count_series = _cell_index;
	  _has_cell_voltages = true;

	}
	break;

    default:
      break;
  }
  const uint32_t tnow_us = AP_HAL::micros();
  const uint32_t dt_us = tnow_us - _state.last_time_micros;
  if (_state.healthy && has_current()) {
      update_consumed(_state, dt_us);
      _state.last_time_micros = tnow_us;
  }
  //  send_gcs_bms_status();
}

void AP_BattMonitor_CAN_BMS::handle_frame(AP_HAL::CANFrame &frame)
{

}

bool AP_BattMonitor_CAN_BMS::get_unique_id(const uint8_t* &uniq_id) const
{
  uniq_id = Battery_info.uid_str;
  return true;
}

bool AP_BattMonitor_CAN_BMS::get_firmware_info(uint16_t &firm_info) const
{
  firm_info = Battery_info.firmware_info;
  return true;
}

bool AP_BattMonitor_CAN_BMS::get_state_of_health(uint16_t &state_of_health) const
{
  state_of_health = Battery_info.SOH;
  return true;
}

bool AP_BattMonitor_CAN_BMS::get_state_of_charge(uint16_t &state_of_charge) const
{
  state_of_charge = Battery_info.SOC;
  return true;
}
bool AP_BattMonitor_CAN_BMS::get_capacity(uint32_t &cap) const
{
  cap = Battery_info.capacity;
  return true;
}

bool AP_BattMonitor_CAN_BMS::get_remaining_capacity(uint32_t &rem_cap) const
{
  rem_cap = Battery_info.remaining_capacity;
  return true;
}

uint8_t AP_BattMonitor_CAN_BMS::get_cell_count(uint8_t &count_cell) const
{
  count_cell = Battery_info.cell_count_series;
  return count_cell;
}
bool AP_BattMonitor_CAN_BMS::get_temperature2(int16_t &temp) const
{
  temp = Battery_info.temperatures_ntc[1];
  return true;
}
bool AP_BattMonitor_CAN_BMS::has_temperature() const
{
  return _has_temperature;
}

bool AP_BattMonitor_CAN_BMS::get_battery_info(uint16_t &firm_info,const uint8_t* &uniq_id) const
{
  firm_info = Battery_info.firmware_info;
  uniq_id = Battery_info.uid_str;
  return true;
}

bool AP_BattMonitor_CAN_BMS::get_VI_readings(uint16_t &SOC, uint16_t &SOH, uint32_t &capacity,
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

bool AP_BattMonitor_CAN_BMS::get_min_max_cellVolt(float &max_cell_volt, uint8_t &max_cell_volt_cell_loc, uint8_t &max_cell_volt_cell_ctr,
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

bool AP_BattMonitor_CAN_BMS::get_min_max_temperature(int8_t &max_temp, uint8_t &max_temp_ntc_loc_cell, uint8_t &max_temp_ntc_loc_ctr,
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

bool AP_BattMonitor_CAN_BMS::get_bms_relay_state(uint8_t &bms_state, bool &relay_charge, bool &relay_precharge,
                         bool &relay_negative, bool &relay_positive) const
{
  bms_state = Battery_info.bms_state;
  relay_charge = Battery_info.relay_charge;
  relay_precharge = Battery_info.relay_precharge;
  relay_negative = Battery_info.relay_negative;
  relay_positive = Battery_info.relay_positive;
  return true;
}

bool AP_BattMonitor_CAN_BMS::get_cell_balancing_status(uint16_t &balancing_status_cc1, uint16_t &balancing_status_cc2,
                               uint16_t &balancing_status_cc3, uint16_t &balancing_status_cc4) const
{
    balancing_status_cc1 = Battery_info.balancing_status_cc1;
    balancing_status_cc2 = Battery_info.balancing_status_cc2;
    balancing_status_cc3 = Battery_info.balancing_status_cc3;
    balancing_status_cc4 = Battery_info.balancing_status_cc4;
    return true;
}

bool AP_BattMonitor_CAN_BMS::get_faults_and_warnings(uint32_t &fault_flags, uint32_t &warning_flags) const
{
    fault_flags = Battery_info.fault_flags;
    warning_flags = Battery_info.warning_flags;
    return true;
}

bool AP_BattMonitor_CAN_BMS::get_temp_ntc_cell_count_and_voltages(const int8_t* &temperatures_ntc, uint8_t &cell_count_series,
                                          const uint16_t* &cell_voltages) const
{
    temperatures_ntc = Battery_info.temperatures_ntc;
    cell_count_series = Battery_info.cell_count_series;
    cell_voltages = Battery_info.cell_voltages;
    return true;
}

void AP_BattMonitor_CAN_BMS::send_gcs_bms_status()
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
