#include <AG_OSD/AG_OSD_Backend.h>
#include <AG_MSP/AG_MSP.h>

class AG_OSD_MSP : public AG_OSD_Backend
{
    using AG_OSD_Backend::AG_OSD_Backend;
public:
    static AG_OSD_Backend *probe(AG_OSD &osd);

    //initilize display port and underlying hardware
    bool init() override;

    //draw given text to framebuffer
    void write(uint8_t x, uint8_t y, const char* text) override {};

    //flush framebuffer to screen
    void flush() override {};

    //clear framebuffer
    void clear() override {};

private:
    void setup_defaults(void);
};
