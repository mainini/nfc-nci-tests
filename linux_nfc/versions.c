/*
 * Copyright 2015 Pascal Mainini
 * Licensed under MIT license, see included file LICENSE or
 * http://opensource.org/licenses/MIT
 *
 * Simple test-tool to detect FW and MW-versions
 */

#include <stdio.h>
#include <stdlib.h>

#define NXP_HW_SELF_TEST
#include "linux_nfc_factory_api.h"
#include "linux_nfc_api.h"

int main(int argc, char ** argv)
{
   if(nfcManager_doInitialize())
    {
        perror("nfcManager_doInitialize");
        exit(1);
    }

    // try to get versions from chip
    #ifdef NXP_HW_SELF_TEST
    printf("nfcFactory_GetMwVersion:\t0x%x\n", nfcFactory_GetMwVersion());
    #endif
    printf("nfcManager_getFwVersion:\t0x%x\n", nfcManager_getFwVersion());

    if(nfcManager_doDeinitialize())
    {
        perror("nfcManager_doDeinitialize");
        exit(1);
    }

    return 0;
}
