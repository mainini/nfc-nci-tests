/*
 * Copyright 2015 Pascal Mainini
 * Licensed under MIT license, see included file LICENSE or
 * http://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NXP_HW_SELF_TEST
#include "linux_nfc_factory_api.h"
#include "linux_nfc_api.h"

#define MAIN_SLEEP 10000

static void tagArrived(nfc_tag_info_t *p_taginfo)
{
    printf("\nTag arrived!\n");
}

static void tagDeparted(void)
{
    printf("\nTag departed!\n");
}

int main(int argc, char ** argv)
{
    // initialize nfcManager
    int result = nfcManager_doInitialize();
    if(result != 0) {
        printf("nfcManager_doInitialize failed with error code %d", result);
        exit(result);
    }

    // try to get versions
    #ifdef NXP_HW_SELF_TEST
    printf("nfcFactory_GetMwVersion:\t0x%x\n", nfcFactory_GetMwVersion());
    #endif
    printf("nfcManager_getFwVersion:\t0x%x\n\n", nfcManager_getFwVersion());

    // set up tag-callback
    nfcTagCallback_t cb_tag;
	cb_tag.onTagArrival = tagArrived;
	cb_tag.onTagDeparture = tagDeparted;
	nfcManager_registerTagCallback(&cb_tag);

	nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0, 0, 0);

    while(1) {
        usleep(MAIN_SLEEP);
    }

    printf("\n\nExiting...\n");
    return 0;
}
