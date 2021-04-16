#include "Copter.h"

#if MODE_SPORT_ENABLED == ENABLED

#define MAX_INPUT 100.0f
#define MID_INPUT 50.0f
#define DEADBAND 0.01f //0 to 1


/*
 * Init and run calls for sport flight mode
 */

// sport_init - initialise sport controller
bool ModeSport::init(bool ignore_checks)
{
    counter = 0;
    lateralSequenceArmed = 0;
    lateralSequenceStart = AP_HAL::micros();
    return true;
}

// sport_run - runs the sport controller
// should be called at 100hz or more
void ModeSport::run()
{
    float lateral_input, pitch_input, yaw_input, thrust_input;
    float lateral_target = 0.0f;

    // We use a NED frame as per the UAV standard
    // Roll, pitch, yaw channel are between -1 and 1
    // lateral = 1 -> move to the right
    // Pitch = 1 -> pitch backward
    // Yaw = 1 -> turn clockwise
    // Thrust is between 0 and 1

    lateral_input = (float(channel_roll->percent_input()) - MID_INPUT) / MID_INPUT; // Exemple: channel=0.3 range -1 to 1 so 1.3/2=65% 65-50/50=0.3
    pitch_input = -(float(channel_pitch->percent_input()) - MID_INPUT) / MID_INPUT;
    yaw_input = (float(channel_yaw->percent_input()) - MID_INPUT) / MID_INPUT;
    thrust_input = float(channel_throttle->percent_input()) / MAX_INPUT;

    //Add a deadband to inputs
    lateral_input = abs(lateral_input)<DEADBAND ? 0.0f : lateral_input;
    pitch_input = abs(pitch_input)<DEADBAND ? 0.0f : pitch_input;
    yaw_input = abs(yaw_input)<DEADBAND ? 0.0f : yaw_input;

    if(lateralSequenceArmed && lateral_input<-0.8)
    {
        lateral_target = 0.0f;
        lateralSequenceArmed = false;
        lateralSequenceStart = AP_HAL::millis();
    }
    else if(!lateralSequenceArmed && lateral_input>0.8)
    {
        lateral_target = 0.0f;
        lateralSequenceArmed = true;
        lateralSequenceStart = AP_HAL::millis();
    }
    else if(lateralSequenceArmed)
    {
        uint32_t now = AP_HAL::millis();
        if(now-lateralSequenceStart<5000)
        {
            lateral_target = 0.0f;
        }
        // else if(now-lateralSequenceStart<15000)
        // {
        //     lateral_target = (L1+L2)*sinf(0.2f);
        // }
        // else if(now-lateralSequenceStart<25000)
        // {
        //     lateral_target = (L1+L2)*sinf(-0.2f);
        // }
        // else if(now-lateralSequenceStart<35000)
        // {
        //     lateral_target = (L1+L2)*sinf(-0.25f);
        // }
        else
        {
            lateral_target = -(L1+L2)*sinf(0.2f);
        }       
    }
    else
    {
        lateral_target = 0.0f;
        lateralSequenceArmed = false;
        lateralSequenceStart = AP_HAL::millis();
    }

    if (!motors->armed()) {
        // Motors should be Stopped
        motors->set_desired_spool_state(AP_Motors::DesiredSpoolState::SHUT_DOWN);
    }
    
    else {
        motors->set_desired_spool_state(AP_Motors::DesiredSpoolState::THROTTLE_UNLIMITED);
    }

    // Only call controller each 8 timestep to have 50Hz
    if (counter>7){
        attitude_control->deleaves_controller_latHold(lateral_target, pitch_input, yaw_input, thrust_input, lateralSequenceArmed, motors->armed());
        counter=0;
    }
    counter++;


}

#endif
