#pragma once

#include "AG_EFI.h"
#include "AG_EFI_Backend.h"

#if AG_EFI_DRONECAN_ENABLED
#include <AG_UAVCAN/AG_UAVCAN.h>
#include <uavcan/equipment/ice/reciprocating/Status.hpp>

class EFIStatusCb;

class AG_EFI_DroneCAN : public AG_EFI_Backend {
public:
    AG_EFI_DroneCAN(AG_EFI &_frontend);

    void update() override;

    static void subscribe_msgs(AG_UAVCAN* ap_uavcan);
    static void trampoline_status(AG_UAVCAN* ap_uavcan, uint8_t node_id, const EFIStatusCb &cb);

private:
    void handle_status(const uavcan::equipment::ice::reciprocating::Status &pkt);

    // singleton for trampoline
    static AG_EFI_DroneCAN *driver;
};
#endif // AG_EFI_DRONECAN_ENABLED

