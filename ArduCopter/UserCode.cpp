#include "Copter.h"

#ifdef USERHOOK_INIT
void Copter::userhook_init()
{
    // put your initialisation code here
    // this will be called once at start-up
}
#endif

#ifdef USERHOOK_FASTLOOP
void Copter::userhook_FastLoop()
{
    // put your 100Hz code here
}
#endif

#ifdef USERHOOK_50HZLOOP
void Copter::userhook_50Hz()
{
    // put your 50Hz code here
}
#endif

#ifdef USERHOOK_MEDIUMLOOP
void Copter::userhook_MediumLoop()
{
    // put your 10Hz code here
}
#endif

#ifdef USERHOOK_SLOWLOOP
void Copter::userhook_SlowLoop()
{
    // put your 3.3Hz code here
}
#endif

#ifdef USERHOOK_SUPERSLOWLOOP
void Copter::userhook_SuperSlowLoop()
{
    // put your 1Hz code here
	// @Values: 0:Undefined, 1:Quad, 2:Hexa, 3:Octa, 4:OctaQuad, 5:Y6, 6:Heli, 7:Tri, 8:SingleCopter, 9:CoaxCopter, 10:BiCopter, 11:Heli_Dual, 12:DodecaHexa, 13:HeliQuad, 14:Deca, 15:Scripting Matrix, 16:6DoF Scripting, 17:Dynamic Scripting Matrix
	// @Values: 0:Plus, 1:X, 2:V, 3:H, 4:V-Tail, 5:A-Tail, 10:Y6B, 11:Y6F, 12:BetaFlightX, 13:DJIX, 14:ClockwiseX, 15: I, 18: BetaFlightXReversed, 19:Y4
	//Quad - + (1,0)	//Quad - X (1,1)
	//Hexa - + (2,0)	//Hexa - X (2,1)
	//Octa - + (3,0)	//Octa - X (3,1)
	bool changed = false;
	bool init = true;
	static int prevframe_class = (AP_Motors::motor_frame_class)g2.frame_class.get();
	static int prevframe_type = (AP_Motors::motor_frame_type)g.frame_type.get();
	int m1 = SRV_Channels::get_function(0);
	int m2 = SRV_Channels::get_function(1);
	int m3 = SRV_Channels::get_function(2);
	int m4 = SRV_Channels::get_function(3);
	int m5 = SRV_Channels::get_function(4);
	int m6 = SRV_Channels::get_function(5);
	int m7 = SRV_Channels::get_function(6);
	int m8 = SRV_Channels::get_function(7);

	if(prevframe_class == 1 && prevframe_type == 0)
	{
		if(m1 == SRV_Channel::k_motor3 && m2 == SRV_Channel::k_motor2 && m3 == SRV_Channel::k_motor4 && m4 == SRV_Channel::k_motor1 &&
				m5 == SRV_Channel::k_none && m6 == SRV_Channel::k_none && m7 == SRV_Channel::k_none && m8 == SRV_Channel::k_none)
		{
			init = false;
		}
	}
	else if(prevframe_class == 1 && prevframe_type == 1)
	{
		if(m1 == SRV_Channel::k_motor1 && m2 == SRV_Channel::k_motor3 && m3 == SRV_Channel::k_motor2 && m4 == SRV_Channel::k_motor4 &&
				m5 == SRV_Channel::k_none && m6 == SRV_Channel::k_none && m7 == SRV_Channel::k_none && m8 == SRV_Channel::k_none)
		{
			init = false;
		}
	}
	else if(prevframe_class == 2 && prevframe_type == 0)
	{
		if(m1 == SRV_Channel::k_motor1 && m2 == SRV_Channel::k_motor5 && m3 == SRV_Channel::k_motor3 && m4 == SRV_Channel::k_motor2 &&
				m5 == SRV_Channel::k_motor6 && m6 == SRV_Channel::k_motor4 && m7 == SRV_Channel::k_none && m8 == SRV_Channel::k_none)
		{
			init = false;
		}
	}
	else if(prevframe_class == 2 && prevframe_type == 1)
	{
		if(m1 == SRV_Channel::k_motor5 && m2 == SRV_Channel::k_motor3 && m3 == SRV_Channel::k_motor2 && m4 == SRV_Channel::k_motor6 &&
				m5 == SRV_Channel::k_motor4 && m6 == SRV_Channel::k_motor1 && m7 == SRV_Channel::k_none && m8 == SRV_Channel::k_none)
		{
			init = false;
		}
	}

	if(prevframe_class != (AP_Motors::motor_frame_class)g2.frame_class.get() ||
			prevframe_type != (AP_Motors::motor_frame_type)g.frame_type.get())
	{
		gcs().send_text(MAV_SEVERITY_WARNING,"Frame Changed");
		changed = true;
		init = true;
		prevframe_class = (AP_Motors::motor_frame_class)g2.frame_class.get();
		prevframe_type = (AP_Motors::motor_frame_type)g.frame_type.get();
	}

	if(init || changed)
	{
		if(prevframe_class == 1 && prevframe_type == 0)
		{
			SRV_Channels::set_function(0, SRV_Channel::k_motor3);
			SRV_Channels::set_function(1, SRV_Channel::k_motor2);
			SRV_Channels::set_function(2, SRV_Channel::k_motor4);
			SRV_Channels::set_function(3, SRV_Channel::k_motor1);
			for(int i=4;i<8;i++)
			{
				SRV_Channels::set_function(i, SRV_Channel::k_none);
			}
		}
		else if(prevframe_class == 1 && prevframe_type == 1)
		{
			SRV_Channels::set_function(0, SRV_Channel::k_motor1);
			SRV_Channels::set_function(1, SRV_Channel::k_motor3);
			SRV_Channels::set_function(2, SRV_Channel::k_motor2);
			SRV_Channels::set_function(3, SRV_Channel::k_motor4);
			for(int i=4;i<8;i++)
			{
				SRV_Channels::set_function(i, SRV_Channel::k_none);
			}
		}
		else if(prevframe_class == 2 && prevframe_type == 0)
		{
			SRV_Channels::set_function(0, SRV_Channel::k_motor1);
			SRV_Channels::set_function(1, SRV_Channel::k_motor5);
			SRV_Channels::set_function(2, SRV_Channel::k_motor3);
			SRV_Channels::set_function(3, SRV_Channel::k_motor2);
			SRV_Channels::set_function(4, SRV_Channel::k_motor6);
			SRV_Channels::set_function(5, SRV_Channel::k_motor4);
			for(int i=6;i<8;i++)
			{
				SRV_Channels::set_function(i, SRV_Channel::k_none);
			}
		}
		else if(prevframe_class == 2 && prevframe_type == 1)
		{
			SRV_Channels::set_function(0, SRV_Channel::k_motor5);
			SRV_Channels::set_function(1, SRV_Channel::k_motor3);
			SRV_Channels::set_function(2, SRV_Channel::k_motor2);
			SRV_Channels::set_function(3, SRV_Channel::k_motor6);
			SRV_Channels::set_function(4, SRV_Channel::k_motor4);
			SRV_Channels::set_function(5, SRV_Channel::k_motor1);
			for(int i=6;i<8;i++)
			{
				SRV_Channels::set_function(i, SRV_Channel::k_none);
			}
		}
		else if(prevframe_class == 3 && prevframe_type == 0)
		{

		}
		else if(prevframe_class == 3 && prevframe_type == 1)
		{

		}


		if(changed)
		{
			changed = false;
			printf("Changed: Motor set\n");
			gcs().send_text(MAV_SEVERITY_WARNING,"Changed: Motors Set, Required Reboot");
		}
		else if(init)
		{
			init = true;
			printf("init: Motor set\n");
			gcs().send_text(MAV_SEVERITY_WARNING,"Init: Motors Set, Required Reboot");
		}

	}
}
#endif

#ifdef USERHOOK_AUXSWITCH
void Copter::userhook_auxSwitch1(const RC_Channel::AuxSwitchPos ch_flag)
{
    // put your aux switch #1 handler here (CHx_OPT = 47)
}

void Copter::userhook_auxSwitch2(const RC_Channel::AuxSwitchPos ch_flag)
{
    // put your aux switch #2 handler here (CHx_OPT = 48)
}

void Copter::userhook_auxSwitch3(const RC_Channel::AuxSwitchPos ch_flag)
{
    // put your aux switch #3 handler here (CHx_OPT = 49)
}
#endif
