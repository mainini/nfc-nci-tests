#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>

#define NXP_HW_SELF_TEST
#include "linux_nfc_factory_api.h"
#include "linux_nfc_api.h"

static nfcTagCallback_t cb_tag;
static int keepPolling = 1;
static std::vector <nfc_tag_info_t> tags;
static pthread_mutex_t mutex;

void tagArrived(nfc_tag_info_t *p_taginfo)
{
    printf("\nTag arrived!\n");
    fflush(stdout);

    pthread_mutex_lock(&mutex);
    tags.push_back(*p_taginfo);
    pthread_mutex_unlock(&mutex);
}

void tagDeparted()
{
    printf("\nTag departed!\n");
    fflush(stdout);
}

int main(int argc, char ** argv)
{
    pthread_mutex_init(&mutex, NULL);

    // initialize nfcManager
    int result = nfcManager_doInitialize();
    if(result != 0) {
        printf("nfcManager_doInitialize failed with error code %d", result);
        return result;
    }

    // try to get versions
#ifdef NXP_HW_SELF_TEST
    printf("nfcFactory_GetMwVersion:\t%x\n", nfcFactory_GetMwVersion());
#endif
    printf("nfcManager_getFwVersion:\t%x\n", nfcManager_getFwVersion());

    // set up tag-callback
	cb_tag.onTagArrival = tagArrived;
	cb_tag.onTagDeparture = tagDeparted;
	nfcManager_registerTagCallback(&cb_tag);

	nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0x00, 0x00, 0);

    while(keepPolling) {
        nfc_tag_info_t tag;

        pthread_mutex_lock(&mutex);

        int tagcount = tags.size();

        printf("Number of tags in queue: %d\n", tagcount);
        if(tagcount) {
            tag = tags.front();
            tags.erase(tags.begin());
        }
        
        pthread_mutex_unlock(&mutex);

        
        sleep(1);
    }

    pthread_mutex_destroy(&mutex);

    printf("\nExiting...\n");
    return 0;
}
