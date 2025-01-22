#pragma once

#include "AG_EFI.h"
#include "AG_EFI_Backend.h"

#if AG_EFI_SCRIPTING_ENABLED

class AG_EFI_Scripting : public AG_EFI_Backend {
public:
    using AG_EFI_Backend::AG_EFI_Backend;

    void update() override;

    bool handle_scripting(const EFI_State &efi_state) override;
};
#endif // AG_EFI_SCRIPTING_ENABLED
