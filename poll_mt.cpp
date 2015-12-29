/*
 * Copyright 2015 Pascal Mainini
 * Licensed under MIT license, see included file LICENSE or
 * http://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <vector>

#define NXP_HW_SELF_TEST
#include "linux_nfc_factory_api.h"
#include "linux_nfc_api.h"

#define MAX_UID_LENGTH 32
#define MAIN_SLEEP 10000

static int do_poll = 1;
static std::vector <nfc_tag_info_t*> v_tags;
static pthread_mutex_t mutex;

static void interrupted(int signal)
{
    do_poll = 0;
}

void tagArrived(nfc_tag_info_t *p_taginfo)
{
    pthread_mutex_lock(&mutex);

    nfc_tag_info_t *temp = (nfc_tag_info_t *) malloc(sizeof(nfc_tag_info_t));
    memcpy(temp, p_taginfo, sizeof(nfc_tag_info_t));
    v_tags.push_back(temp);

    pthread_mutex_unlock(&mutex);
}

void tagDeparted() { }

void printTagUID(nfc_tag_info_t *p_tag)
{
    char uid[3*MAX_UID_LENGTH+1];
    int len = p_tag->uid_length;
    if(len > MAX_UID_LENGTH)
        len = MAX_UID_LENGTH;

    for(int i = 0; i < len; i++) {
        sprintf(uid + (i*3), "%02x ", p_tag->uid[i]);
    }
    uid[len*3] = '\0';

    printf("UID: %s\n", uid);
}

int main(int argc, char ** argv)
{
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &interrupted;

    if (sigaction(SIGINT, &act, NULL) < 0 || sigaction(SIGTERM, &act, NULL)) {
        perror("sigaction");
        exit(1);
    }

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
    printf("nfcFactory_GetMwVersion:\t0x%x\n", nfcFactory_GetMwVersion());
    #endif
    printf("nfcManager_getFwVersion:\t0x%x\n\n", nfcManager_getFwVersion());

    // set up tag-callback
    nfcTagCallback_t cb_tag;
    cb_tag.onTagArrival = tagArrived;
    cb_tag.onTagDeparture = tagDeparted;
    nfcManager_registerTagCallback(&cb_tag);

    // start tag-discovery
    // TODO is it possible to look for multiple tags "at the same time"?
    nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0x00, 0x00, 0);

    // main loop, performs handling of read tags
    while(do_poll)
    {
        nfc_tag_info_t *tag = NULL;

        pthread_mutex_lock(&mutex);

        int tagcount = v_tags.size();
        if(tagcount)
        {
            tag = v_tags.front();
            v_tags.erase(v_tags.begin());
        }
        
        pthread_mutex_unlock(&mutex);

        if(tag)
        {
            printTagUID(tag);
            free(tag);
        }

        usleep(MAIN_SLEEP);
    }

    // TODO:
    // - free remaining tags
    // - shutdown PN7120 ?

    pthread_mutex_destroy(&mutex);

    printf("\n\nExiting...\n");
    return 0;
}
