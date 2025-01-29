/// @file	GCS_AGPILOTLink.h
/// @brief	One size fits all header for AGPILOTLink integration.
#pragma once

#include <AP_HAL/AP_HAL_Boards.h>

// we have separate helpers disabled to make it possible
// to select AGPILOTLink 1.0 in the arduino GUI build
#define AGPILOTLINK_SEPARATE_HELPERS
#define AGPILOTLINK_NO_CONVERSION_HELPERS

#define AGPILOTLINK_SEND_UART_BYTES(chan, buf, len) comm_send_buffer(chan, buf, len)

#define AGPILOTLINK_START_UART_SEND(chan, size) comm_send_lock(chan, size)
#define AGPILOTLINK_END_UART_SEND(chan, size) comm_send_unlock(chan)

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
// allow extra mavlink channels in SITL for:
//    Vicon, ship
#define AGPILOTLINK_COMM_NUM_BUFFERS 7
#else
// allow five telemetry ports
#define AGPILOTLINK_COMM_NUM_BUFFERS 5
#endif

/*
  The AGPILOTLink protocol code generator does its own alignment, so
  alignment cast warnings can be ignored
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

#if defined(__GNUC__) && __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif

#include "include/mavlink/v2.0/all/version.h"

#define AGPILOTLINK_MAX_PAYLOAD_LEN 255

#include "include/mavlink/v2.0/mavlink_types.h"

/// AGPILOTLink stream used for uartA
extern AP_HAL::UARTDriver	*mavlink_comm_port[AGPILOTLINK_COMM_NUM_BUFFERS];
extern bool gcs_alternative_active[AGPILOTLINK_COMM_NUM_BUFFERS];

/// AGPILOTLink system definition
extern mavlink_system_t mavlink_system;

/// Sanity check AGPILOTLink channel
///
/// @param chan		Channel to send to
static inline bool valid_channel(mavlink_channel_t chan)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-constant-out-of-range-compare"
    return chan < AGPILOTLINK_COMM_NUM_BUFFERS;
#pragma clang diagnostic pop
}

void comm_send_buffer(mavlink_channel_t chan, const uint8_t *buf, uint8_t len);

/// Check for available transmit space on the nominated AGPILOTLink channel
///
/// @param chan		Channel to check
/// @returns		Number of bytes available
uint16_t comm_get_txspace(mavlink_channel_t chan);

#define AGPILOTLINK_USE_CONVENIENCE_FUNCTIONS
#include "include/mavlink/v2.0/all/mavlink.h"

// lock and unlock a channel, for multi-threaded mavlink send
void comm_send_lock(mavlink_channel_t chan, uint16_t size);
void comm_send_unlock(mavlink_channel_t chan);
HAL_Semaphore &comm_chan_lock(mavlink_channel_t chan);

#pragma GCC diagnostic pop
