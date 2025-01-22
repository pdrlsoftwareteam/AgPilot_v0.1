//
// scan I2C and SPI buses for expected devices
//

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

static struct {
    const char *name;
    uint8_t whoami_reg;
} whoami_list[] = {
    { "ms5611",     0x00 | 0x80 },
    { "mpu9250",    0x75 | 0x80 },
    { "mpu6000",    0x75 | 0x80 },
    { "lsm9ds0_am", 0x0F | 0x80 },
    { "lsm9ds0_g",  0x0F | 0x80 },
};

AG_HAL::OwnPtr<AG_HAL::Device>
get_device(const char *name)
{
    AG_HAL::OwnPtr<AG_HAL::Device> dev;

    const char *spi_name = hal.spi->get_device_name(0);
    /* Dummy is the default device at index 0 
       if there isn't registered device for the target board.
    */
    const char *dummy = "**dummy**";
    if (!strcmp(dummy, spi_name)) {
        hal.console->printf("No devices, nothing to do.\n");
        while (1) {
            hal.scheduler->delay(500);
        }
    }

    /* We get possible registered device count on the target board */
    uint8_t spicount = hal.spi->get_count();
    for (uint8_t ref = 0; ref < spicount; ref++) {
        /*
         * We get the name from the index and we compare
         * with our possible devices list.
         */
        spi_name = hal.spi->get_device_name(ref);
        if (!strcmp(name, spi_name)) {
            dev = hal.spi->get_device(spi_name);
            break;
        }
    }

    return dev;
}

void setup(void)
{
    hal.console->println("BusTest startup...\n");
}

void loop(void)
{
    AG_HAL::OwnPtr<AG_HAL::Device> dev;
    AG_HAL::Semaphore *spi_sem;
    const char *bus_type = new char;

    hal.console->printf("Scanning SPI and I2C bus devices\n");

    for (uint8_t i = 0; i < ARRAY_SIZE(whoami_list); i++) {
        dev = get_device(whoami_list[i].name);
        if (!dev) {
            continue;
        }

        if (dev->bus_type() == AG_HAL::Device::BusType::BUS_TYPE_SPI) {
            bus_type = "SPI";
        } else {
            bus_type = "I2C";
        }

        hal.console->printf("Device %s registered on %s bus\n", whoami_list[i].name, bus_type);
        hal.console->printf("Trying to send data...\n");

        spi_sem = dev->get_semaphore();
        if (!spi_sem->take(1000)) {
            hal.console->printf("Failed to get SPI semaphore for %s\n", whoami_list[i].name);
            break;
        }
        uint8_t tx[2] = { whoami_list[i].whoami_reg, 0 };
        uint8_t rx[2] = { 0, 0 };
        dev->transfer(tx, sizeof(tx), rx, sizeof(rx));
        if (rx[1] != 0) {
            hal.console->printf("WHO_AM_I for %s: 0x%02x\n\n", whoami_list[i].name, (unsigned)rx[1]);
        } else {
            hal.console->printf("No response %s!\n\n", whoami_list[i].name);
            break;
        }
        spi_sem->give();
    }

    hal.scheduler->delay(1000);
}

AG_HAL_MAIN();
