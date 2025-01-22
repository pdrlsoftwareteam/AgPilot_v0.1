#include "AG_DAL_RangeFinder.h"

#include <AG_Logger/AG_Logger.h>

#include <AG_RangeFinder/AG_RangeFinder_Backend.h>
#include "AG_DAL.h"
#include <AG_BoardConfig/AG_BoardConfig.h>
#include <AG_Vehicle/AG_Vehicle_Type.h>

AG_DAL_RangeFinder::AG_DAL_RangeFinder()
{
#if !APM_BUILD_TYPE(APM_BUILD_AG_DAL_Standalone) && !APM_BUILD_TYPE(APM_BUILD_Replay)
    _RRNH.num_sensors = AP::rangefinder()->num_sensors();
    _RRNI = new log_RRNI[_RRNH.num_sensors];
    _backend = new AG_DAL_RangeFinder_Backend *[_RRNH.num_sensors];
    if (!_RRNI || !_backend) {
        goto failed;
    }
    for (uint8_t i=0; i<_RRNH.num_sensors; i++) {
        _RRNI[i].instance = i;
    }
    for (uint8_t i=0; i<_RRNH.num_sensors; i++) {
        // this avoids having to discard a const....
        _backend[i] = new AG_DAL_RangeFinder_Backend(_RRNI[i]);
        if (!_backend[i]) {
            goto failed;
        }
    }
    return;
failed:
    AG_BoardConfig::allocation_error("DAL backends");
#endif
}

int16_t AG_DAL_RangeFinder::ground_clearance_cm_orient(enum Rotation orientation) const
{
#if !APM_BUILD_TYPE(APM_BUILD_AG_DAL_Standalone)
    const auto *rangefinder = AP::rangefinder();

    if (orientation != ROTATION_PITCH_270) {
        // the EKF only asks for this from a specific orientation.  Thankfully.
        INTERNAL_ERROR(AG_InternalError::error_t::flow_of_control);
        return rangefinder->ground_clearance_cm_orient(orientation);
    }
#endif

    return _RRNH.ground_clearance_cm;
}

int16_t AG_DAL_RangeFinder::max_distance_cm_orient(enum Rotation orientation) const
{
#if !APM_BUILD_TYPE(APM_BUILD_AG_DAL_Standalone)
    if (orientation != ROTATION_PITCH_270) {
        const auto *rangefinder = AP::rangefinder();
        // the EKF only asks for this from a specific orientation.  Thankfully.
        INTERNAL_ERROR(AG_InternalError::error_t::flow_of_control);
        return rangefinder->max_distance_cm_orient(orientation);
    }
#endif

    return _RRNH.max_distance_cm;
}

void AG_DAL_RangeFinder::start_frame()
{
    const auto *rangefinder = AP::rangefinder();
    if (rangefinder == nullptr) {
        return;
    }

    const log_RRNH old = _RRNH;

    // EKF only asks for this *down*.
    _RRNH.ground_clearance_cm = rangefinder->ground_clearance_cm_orient(ROTATION_PITCH_270);
    _RRNH.max_distance_cm = rangefinder->max_distance_cm_orient(ROTATION_PITCH_270);

    WRITE_REPLAY_BLOCK_IFCHANGED(RRNH, _RRNH, old);

    for (uint8_t i=0; i<_RRNH.num_sensors; i++) {
        auto *backend = rangefinder->get_backend(i);
        if (backend == nullptr) {
            continue;
        }
        _backend[i]->start_frame(backend);
    }
}




AG_DAL_RangeFinder_Backend::AG_DAL_RangeFinder_Backend(struct log_RRNI &RRNI) :
    _RRNI(RRNI)
{
}

void AG_DAL_RangeFinder_Backend::start_frame(AG_RangeFinder_Backend *backend) {
    const log_RRNI old = _RRNI;
    _RRNI.orientation = backend->orientation();
    _RRNI.status = (uint8_t)backend->status();
    _RRNI.pos_offset = backend->get_pos_offset();
    _RRNI.distance_cm = backend->distance_cm();
    WRITE_REPLAY_BLOCK_IFCHANGED(RRNI, _RRNI, old);
}

// return true if we have a range finder with the specified orientation
bool AG_DAL_RangeFinder::has_orientation(enum Rotation orientation) const
{
    for (uint8_t i=0; i<_RRNH.num_sensors; i++) {
        if (_RRNI[i].orientation == orientation) {
            return true;
        }
    }
    return false;
}


AG_DAL_RangeFinder_Backend *AG_DAL_RangeFinder::get_backend(uint8_t id) const
{
   if (id >= RANGEFINDER_MAX_INSTANCES) {
       INTERNAL_ERROR(AG_InternalError::error_t::flow_of_control);
       return nullptr;
   }
   if (id >= _RRNH.num_sensors) {
        return nullptr;
    }

   return _backend[id];
}

void AG_DAL_RangeFinder::handle_message(const log_RRNH &msg)
{
    _RRNH = msg;
    if (_RRNH.num_sensors > 0 && _RRNI == nullptr) {
        _RRNI = new log_RRNI[_RRNH.num_sensors];
        _backend = new AG_DAL_RangeFinder_Backend *[_RRNH.num_sensors];
    }
}

void AG_DAL_RangeFinder::handle_message(const log_RRNI &msg)
{
    if (_RRNI != nullptr && msg.instance < _RRNH.num_sensors) {
        _RRNI[msg.instance] = msg;
        if (_backend != nullptr && _backend[msg.instance] == nullptr) {
            _backend[msg.instance] = new AG_DAL_RangeFinder_Backend(_RRNI[msg.instance]);
        }
    }
}
