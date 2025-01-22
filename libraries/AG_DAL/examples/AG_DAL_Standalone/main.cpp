//
// Ensure that AG_NavEKF libraries can be compiled when not linked to
// anything except the DAL.
//

#include <AG_DAL/AG_DAL.h>
#include <AG_NavEKF3/AG_NavEKF3.h>
#include <AG_Logger/AG_Logger.h>

void AG_Param::setup_object_defaults(void const*, AG_Param::GroupInfo const*) {}

int AG_HAL::Util::vsnprintf(char*, size_t, char const*, va_list) { return -1; }

void *nologger = nullptr;
AG_Logger &AP::logger() {
    return *((AG_Logger*)nologger);  // this is not usually a good idea...
}
void AG_Logger::WriteBlock(void const*, unsigned short) {}

class AG_HAL_DAL_Standalone : public AG_HAL::HAL {
public:
    AG_HAL_DAL_Standalone() :
        AG_HAL::HAL(
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
            ) {}
    void run(int argc, char* const argv[], Callbacks* callbacks) const override {}
    void setup() { }
    void loop() { }
};

AG_HAL_DAL_Standalone _hal;
const AG_HAL::HAL &hal = _hal;

NavEKF2 navekf2;
NavEKF3 navekf3;

int main(int argc, const char *argv[])
{
    navekf2.InitialiseFilter();
    navekf3.InitialiseFilter();
    navekf2.UpdateFilter();
    navekf3.UpdateFilter();
    return navekf2.healthy() && navekf3.healthy()?0:1;
}
