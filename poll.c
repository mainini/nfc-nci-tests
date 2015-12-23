#include <stdio.h>
#include "linux_nfc_api.h"

int main(int argc, char ** argv)
{
    printf("nfcFactory_GetMwVersion:\t%x\n", nfcFactory_GetMwVersion());
    printf("nfcManager_getFwVersion:\t%x\n", nfcManager_getFwVersion);
}
