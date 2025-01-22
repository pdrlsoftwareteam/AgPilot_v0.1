#pragma once

#include "AG_RangeFinder_Benewake.h"

#ifndef AP_RANGEFINDER_BENEWAKE_TF03_ENABLED
#define AP_RANGEFINDER_BENEWAKE_TF03_ENABLED (AP_RANGEFINDER_BENEWAKE_ENABLED && AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED)
#endif

#if AP_RANGEFINDER_BENEWAKE_TF03_ENABLED

class AG_RangeFinder_Benewake_TF03 : public AG_RangeFinder_Benewake
{
public:

    static AG_RangeFinder_Backend_Serial *create(
        RangeFinder::RangeFinder_State &_state,
        AG_RangeFinder_Params &_params) {
        return new AG_RangeFinder_Benewake_TF03(_state, _params);
    }

protected:
    float model_dist_max_cm() const override { return 18000; }

private:
    using AG_RangeFinder_Benewake::AG_RangeFinder_Benewake;
};

#endif  // AP_RANGEFINDER_BENEWAKE_TF03_ENABLED
