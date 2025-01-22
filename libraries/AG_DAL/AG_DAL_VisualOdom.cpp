#include "AG_DAL_VisualOdom.h"

#include <AG_VisualOdom/AG_VisualOdom.h>

#if HAL_VISUALODOM_ENABLED

#include <AG_Logger/AG_Logger.h>
#include "AG_DAL.h"
#include <AG_Vehicle/AG_Vehicle_Type.h>

/*
   update position offsets to align to AHRS position
   should only be called when this library is not being used as the position source
   This function does not change EKF state, so does not need to be logged
*/
void AG_DAL_VisualOdom::align_position_to_ahrs(bool align_xy, bool align_z)
{
#if !APM_BUILD_TYPE(APM_BUILD_AG_DAL_Standalone)
    auto *vo = AP::visualodom();
    vo->align_position_to_ahrs(align_xy, align_z);
#endif
}

void AG_DAL_VisualOdom::start_frame()
{
    const auto *vo = AP::visualodom();

    const log_RVOH old = RVOH;
    if (vo != nullptr) {
        RVOH.pos_offset = vo->get_pos_offset();
        RVOH.delay_ms = vo->get_delay_ms();
        RVOH.healthy = vo->healthy();
        RVOH.enabled = vo->enabled();
    }

    WRITE_REPLAY_BLOCK_IFCHANGED(RVOH, RVOH, old);
}

#endif // HAL_VISUALODOM_ENABLED
