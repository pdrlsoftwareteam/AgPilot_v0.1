/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 *   AG_ServoRelayEvents - handle servo and relay MAVLink events
 */


#include <AG_HAL/AG_HAL.h>
#include <AG_Common/AG_Common.h>
#include "AG_ServoRelayEvents.h"
#include <RC_Channel/RC_Channel.h>
#include <SRV_Channel/SRV_Channel.h>
#include <GCS_MAVLink/GCS.h>

extern const AG_HAL::HAL& hal;

bool AG_ServoRelayEvents::do_set_servo(uint8_t _channel, uint16_t pwm)
{
    SRV_Channel *c = SRV_Channels::srv_channel(_channel-1);
    if (c == nullptr) {
        return false;
    }
    switch(c->get_function())
    {
    case SRV_Channel::k_none:
    case SRV_Channel::k_manual:
    case SRV_Channel::k_sprayer_pump:
    case SRV_Channel::k_sprayer_spinner:
    case SRV_Channel::k_gripper:
    case SRV_Channel::k_rcin1 ... SRV_Channel::k_rcin16: // rc pass-thru
        break;
    default:
        gcs().send_text(MAV_SEVERITY_INFO, "ServoRelayEvent: Channel %d is already in use", _channel);
        return false;
    }
    if (type == EVENT_TYPE_SERVO && 
        channel == _channel) {
        // cancel previous repeat
        repeat = 0;
    }
    c->set_output_pwm(pwm);
    c->ignore_small_rcin_changes();
    return true;
}

bool AG_ServoRelayEvents::do_set_relay(uint8_t relay_num, uint8_t state)
{
    AG_Relay *relay = AP::relay();
    if (relay == nullptr) {
        return false;
    }

    if (!relay->enabled(relay_num)) {
        return false;
    }
    if (type == EVENT_TYPE_RELAY && 
        channel == relay_num) {
        // cancel previous repeat
        repeat = 0;
    }
    if (state == 1) {
        relay->on(relay_num);
    } else if (state == 0) {
        relay->off(relay_num);
    } else {
        relay->toggle(relay_num);
    }
    return true;
}

bool AG_ServoRelayEvents::do_repeat_servo(uint8_t _channel, uint16_t _servo_value, 
                                          int16_t _repeat, uint16_t _delay_ms)
{
    SRV_Channel *c = SRV_Channels::srv_channel(_channel-1);
    if (c == nullptr) {
        return false;
    }
    switch(c->get_function())
    {
    case SRV_Channel::k_none:
    case SRV_Channel::k_manual:
    case SRV_Channel::k_sprayer_pump:
    case SRV_Channel::k_sprayer_spinner:
    case SRV_Channel::k_gripper:
    case SRV_Channel::k_rcin1 ... SRV_Channel::k_rcin16: // rc pass-thru
        break;
    default:
        gcs().send_text(MAV_SEVERITY_INFO, "ServoRelayEvent: Channel %d is already in use", _channel);
        return false;
    }
    channel = _channel;
    type = EVENT_TYPE_SERVO;

    start_time_ms  = 0;
    delay_ms    = _delay_ms / 2;
    repeat      = _repeat * 2;
    servo_value = _servo_value;
    update_events();
    return true;
}

bool AG_ServoRelayEvents::do_repeat_relay(uint8_t relay_num, int16_t _repeat, uint32_t _delay_ms)
{
    AG_Relay *relay = AP::relay();
    if (relay == nullptr) {
        return false;
    }
    if (!relay->enabled(relay_num)) {
        return false;
    }
    type = EVENT_TYPE_RELAY;
    channel = relay_num;
    start_time_ms  = 0;
    delay_ms        = _delay_ms/2; // half cycle time
    repeat          = _repeat*2;  // number of full cycles
    update_events();
    return true;
}


/*
  update state for MAV_CMD_DO_REPEAT_SERVO and MAV_CMD_DO_REPEAT_RELAY
*/
void AG_ServoRelayEvents::update_events(void)
{
    if (repeat == 0 || (AG_HAL::millis() - start_time_ms) < delay_ms) {
        return;
    }

    start_time_ms = AG_HAL::millis();

    switch (type) {
    case EVENT_TYPE_SERVO: {
        SRV_Channel *c = SRV_Channels::srv_channel(channel-1);
        if (c != nullptr) {
            if (repeat & 1) {
                c->set_output_pwm(c->get_trim());
            } else {
                c->set_output_pwm(servo_value);
                c->ignore_small_rcin_changes();
            }
        }
        break;
    }

    case EVENT_TYPE_RELAY: {
        AG_Relay *relay = AP::relay();
        if (relay != nullptr) {
            relay->toggle(channel);
        }
        break;
    }
    }
    if (repeat > 0) {
        repeat--;
    } else {
        // toggle bottom bit so servos flip in value
        repeat ^= 1;
    }
}

// singleton instance
AG_ServoRelayEvents *AG_ServoRelayEvents::_singleton;

namespace AP {

AG_ServoRelayEvents *servorelayevents()
{
    return AG_ServoRelayEvents::get_singleton();
}

}
