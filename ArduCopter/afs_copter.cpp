/*
  copter specific AG_AdvancedFailsafe class
 */

#include "Copter.h"

#if ADVANCED_FAILSAFE == ENABLED

/*
  setup radio_out values for all channels to termination values
 */
void AG_AdvancedFailsafe_Copter::terminate_vehicle(void)
{
    if (_terminate_action == TERMINATE_ACTION_LAND) {
        copter.set_mode(Mode::Number::LAND, ModeReason::TERMINATE);
    } else {
        // stop motors
        copter.motors->set_desired_spool_state(AG_Motors::DesiredSpoolState::SHUT_DOWN);
        copter.motors->output();

        // disarm as well
        copter.arming.disarm(AG_Arming::Method::AFS);
    
        // and set all aux channels
        SRV_Channels::set_output_limit(SRV_Channel::k_heli_rsc, SRV_Channel::Limit::TRIM);
        SRV_Channels::set_output_limit(SRV_Channel::k_heli_tail_rsc, SRV_Channel::Limit::TRIM);
        SRV_Channels::set_output_limit(SRV_Channel::k_engine_run_enable, SRV_Channel::Limit::TRIM);
        SRV_Channels::set_output_limit(SRV_Channel::k_ignition, SRV_Channel::Limit::TRIM);
        SRV_Channels::set_output_limit(SRV_Channel::k_none, SRV_Channel::Limit::TRIM);
        SRV_Channels::set_output_limit(SRV_Channel::k_manual, SRV_Channel::Limit::TRIM);
    }

    SRV_Channels::output_ch_all();
}

void AG_AdvancedFailsafe_Copter::setup_IO_failsafe(void)
{
    // setup failsafe for all aux channels
    SRV_Channels::set_failsafe_limit(SRV_Channel::k_heli_rsc, SRV_Channel::Limit::TRIM);
    SRV_Channels::set_failsafe_limit(SRV_Channel::k_heli_tail_rsc, SRV_Channel::Limit::TRIM);
    SRV_Channels::set_failsafe_limit(SRV_Channel::k_engine_run_enable, SRV_Channel::Limit::TRIM);
    SRV_Channels::set_failsafe_limit(SRV_Channel::k_ignition, SRV_Channel::Limit::TRIM);
    SRV_Channels::set_failsafe_limit(SRV_Channel::k_none, SRV_Channel::Limit::TRIM);
    SRV_Channels::set_failsafe_limit(SRV_Channel::k_manual, SRV_Channel::Limit::TRIM);

    // setup AG_Motors outputs for failsafe
    uint32_t mask = copter.motors->get_motor_mask();
    hal.rcout->set_failsafe_pwm(mask, copter.motors->get_pwm_output_min());
}

/*
  return an AFS_MODE for current control mode
 */
AG_AdvancedFailsafe::control_mode AG_AdvancedFailsafe_Copter::afs_mode(void)
{
    switch (copter.flightmode->mode_number()) {
    case Mode::Number::AUTO:
    case Mode::Number::AUTO_RTL:
    case Mode::Number::GUIDED:
    case Mode::Number::RTL:
    case Mode::Number::LAND:
        return AG_AdvancedFailsafe::AFS_AUTO;
    default:
        break;
    }
    return AG_AdvancedFailsafe::AFS_STABILIZED;
}

#endif // ADVANCED_FAILSAFE
