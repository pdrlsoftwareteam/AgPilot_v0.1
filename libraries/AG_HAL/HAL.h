#pragma once

class AG_Param;

#include "AG_HAL_Namespace.h"

#include "AnalogIn.h"
#include "GPIO.h"
#include "RCInput.h"
#include "RCOutput.h"
#include "SPIDevice.h"
#include "QSPIDevice.h"
#include "Storage.h"
#include "UARTDriver.h"
#include "system.h"
#include "OpticalFlow.h"
#include "DSP.h"
#include "CANIface.h"


class AG_HAL::HAL {
public:
    HAL(AG_HAL::UARTDriver* _uartA, // console
        AG_HAL::UARTDriver* _uartB, // 1st GPS
        AG_HAL::UARTDriver* _uartC, // telem1
        AG_HAL::UARTDriver* _uartD, // telem2
        AG_HAL::UARTDriver* _uartE, // 2nd GPS
        AG_HAL::UARTDriver* _uartF, // extra1
        AG_HAL::UARTDriver* _uartG, // extra2
        AG_HAL::UARTDriver* _uartH, // extra3
        AG_HAL::UARTDriver* _uartI, // extra4
        AG_HAL::UARTDriver* _uartJ, // extra5
        AG_HAL::I2CDeviceManager* _i2c_mgr,
        AG_HAL::SPIDeviceManager* _spi,
        AG_HAL::QSPIDeviceManager* _qspi,
        AG_HAL::AnalogIn*   _analogin,
        AG_HAL::Storage*    _storage,
        AG_HAL::UARTDriver* _console,
        AG_HAL::GPIO*       _gpio,
        AG_HAL::RCInput*    _rcin,
        AG_HAL::RCOutput*   _rcout,
        AG_HAL::Scheduler*  _scheduler,
        AG_HAL::Util*       _util,
        AG_HAL::OpticalFlow*_opticalflow,
        AG_HAL::Flash*      _flash,
#if AP_SIM_ENABLED && CONFIG_HAL_BOARD != HAL_BOARD_SITL
        class AG_HAL::SIMState*   _simstate,
#endif
        AG_HAL::DSP*        _dsp,
#if HAL_NUM_CAN_IFACES > 0
        AG_HAL::CANIface* _can_ifaces[HAL_NUM_CAN_IFACES])
#else
        AG_HAL::CANIface** _can_ifaces)
#endif
        :
        uartA(_uartA),
        uartB(_uartB),
        uartC(_uartC),
        uartD(_uartD),
        uartE(_uartE),
        uartF(_uartF),
        uartG(_uartG),
        uartH(_uartH),
        uartI(_uartI),
        uartJ(_uartJ),
        i2c_mgr(_i2c_mgr),
        spi(_spi),
        qspi(_qspi),
        analogin(_analogin),
        storage(_storage),
        console(_console),
        gpio(_gpio),
        rcin(_rcin),
        rcout(_rcout),
        scheduler(_scheduler),
        util(_util),
        opticalflow(_opticalflow),
        flash(_flash),
#if AP_SIM_ENABLED && CONFIG_HAL_BOARD != HAL_BOARD_SITL
        simstate(_simstate),
#endif
        dsp(_dsp)
    {
#if HAL_NUM_CAN_IFACES > 0
        if (_can_ifaces == nullptr) {
            for (uint8_t i = 0; i < HAL_NUM_CAN_IFACES; i++)
                can[i] = nullptr;
        } else {
            for (uint8_t i = 0; i < HAL_NUM_CAN_IFACES; i++)
                can[i] = _can_ifaces[i];
        }
#endif

        AG_HAL::init();
    }

    struct Callbacks {
        virtual void setup() = 0;
        virtual void loop() = 0;
    };

    struct FunCallbacks : public Callbacks {
        FunCallbacks(void (*setup_fun)(void), void (*loop_fun)(void));

        void setup() override { _setup(); }
        void loop() override { _loop(); }

    private:
        void (*_setup)(void);
        void (*_loop)(void);
    };

    virtual void run(int argc, char * const argv[], Callbacks* callbacks) const = 0;

private:
    // the uartX ports must be contiguous in ram for the serial() method to work
    AG_HAL::UARTDriver* uartA;
    AG_HAL::UARTDriver* uartB UNUSED_PRIVATE_MEMBER;
    AG_HAL::UARTDriver* uartC UNUSED_PRIVATE_MEMBER;
    AG_HAL::UARTDriver* uartD UNUSED_PRIVATE_MEMBER;
    AG_HAL::UARTDriver* uartE UNUSED_PRIVATE_MEMBER;
    AG_HAL::UARTDriver* uartF UNUSED_PRIVATE_MEMBER;
    AG_HAL::UARTDriver* uartG UNUSED_PRIVATE_MEMBER;
    AG_HAL::UARTDriver* uartH UNUSED_PRIVATE_MEMBER;
    AG_HAL::UARTDriver* uartI UNUSED_PRIVATE_MEMBER;
    AG_HAL::UARTDriver* uartJ UNUSED_PRIVATE_MEMBER;

public:
    AG_HAL::I2CDeviceManager* i2c_mgr;
    AG_HAL::SPIDeviceManager* spi;
    AG_HAL::QSPIDeviceManager* qspi;
    AG_HAL::AnalogIn*   analogin;
    AG_HAL::Storage*    storage;
    AG_HAL::UARTDriver* console;
    AG_HAL::GPIO*       gpio;
    AG_HAL::RCInput*    rcin;
    AG_HAL::RCOutput*   rcout;
    AG_HAL::Scheduler*  scheduler;
    AG_HAL::Util        *util;
    AG_HAL::OpticalFlow *opticalflow;
    AG_HAL::Flash       *flash;
    AG_HAL::DSP         *dsp;
#if HAL_NUM_CAN_IFACES > 0
    AG_HAL::CANIface* can[HAL_NUM_CAN_IFACES];
#else
    AG_HAL::CANIface** can;
#endif

    // access to serial ports using SERIALn_ numbering
    UARTDriver* serial(uint8_t sernum) const;

    static constexpr uint8_t num_serial = 10;

#if AP_SIM_ENABLED && CONFIG_HAL_BOARD != HAL_BOARD_SITL
    AG_HAL::SIMState *simstate;
#endif

#ifndef HAL_CONSOLE_DISABLED
# define DEV_PRINTF(fmt, args ...)  do { hal.console->printf(fmt, ## args); } while(0)
#else
# define DEV_PRINTF(fmt, args ...)
#endif

};
