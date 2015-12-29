#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <vector>

#define NXP_HW_SELF_TEST
#include "linux_nfc_factory_api.h"
#include "linux_nfc_api.h"

static int keepPolling = 1;
static std::vector <nfc_tag_info_t*> tags;
static pthread_mutex_t mutex;

void tagArrived(nfc_tag_info_t *p_taginfo)
{
    pthread_mutex_lock(&mutex);

    nfc_tag_info_t *temp = (nfc_tag_info_t *) malloc(sizeof(nfc_tag_info_t));
    memcpy(temp, p_taginfo, sizeof(nfc_tag_info_t));
    tags.push_back(temp);

    pthread_mutex_unlock(&mutex);
}

void tagDeparted()
{
}

int main(int argc, char ** argv)
{
    pthread_mutex_init(&mutex, NULL);

    // initialize nfcManager
    int result = nfcManager_doInitialize();
    if(result != 0)
    {
        printf("nfcManager_doInitialize failed with error code %d", result);
        exit(result);
    }

    // try to get versions
#ifdef NXP_HW_SELF_TEST
    printf("nfcFactory_GetMwVersion:\t%x\n", nfcFactory_GetMwVersion());
#endif
    printf("nfcManager_getFwVersion:\t%x\n", nfcManager_getFwVersion());

    // set up tag-callback
    nfcTagCallback_t cb_tag;
    cb_tag.onTagArrival = tagArrived;
    cb_tag.onTagDeparture = tagDeparted;
    nfcManager_registerTagCallback(&cb_tag);

    nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0x00, 0x00, 0);

    while(keepPolling)
    {
        nfc_tag_info_t *tag = NULL;

        pthread_mutex_lock(&mutex);

        int tagcount = tags.size();

        if(tagcount)
        {
            tag = tags.front();
            tags.erase(tags.begin());
        }
        
        pthread_mutex_unlock(&mutex);

        if(tag)
        {
            char uid[97];
            int len = tag->uid_length;
            if(len > 32)
                len = 32;
            for(int i = 0; i < len; i++) {
                sprintf(uid + (i*3), "%02x ", tag->uid[i]);
            }
            uid[len*3] = '\0';
            printf("UID: %s\n", uid);

            free(tag);
        }

        usleep(10000);
    }

    pthread_mutex_destroy(&mutex);

    printf("\nExiting...\n");
    return 0;
}
