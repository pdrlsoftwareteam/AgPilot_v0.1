#include "AG_FWVersion.h"

namespace AP {

const AG_FWVersion &fwversion()
{
    return AG_FWVersion::get_fwverz();
}

}
