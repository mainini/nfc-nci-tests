/*
 * Copyright 2015 Pascal Mainini
 * Licensed under MIT license, see included file LICENSE or
 * http://opensource.org/licenses/MIT
 *
 * A simple, threaded implementation of tag-discovery using linux_libnfc-nci
 * (https://github.com/NXPNFCLinux/linux_libnfc-nci).
 *
 * TODO
 * ====
 * - Can we detect multiple tags beeing present simultaneously?
 *   UM10819-PN7120-User-Manual.pdf gives almost no information, the only idea
 *   could be "PN7120 behavior with multiple VICCs", 7.1.4.4 / p. 61
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

static pthread_mutex_t mutex;
static std::vector <nfc_tag_info_t*> v_tags_p;
static int do_poll = 1;

static void interrupted(int signal)
{
    do_poll = 0;
}

static void tagArrived(nfc_tag_info_t *p_taginfo)
{
    pthread_mutex_lock(&mutex);

    nfc_tag_info_t *temp = (nfc_tag_info_t *) malloc(sizeof(nfc_tag_info_t));
    memcpy(temp, p_taginfo, sizeof(nfc_tag_info_t));
    v_tags_p.push_back(temp);

    pthread_mutex_unlock(&mutex);
}

static void tagDeparted() { }

static void printTagUID(nfc_tag_info_t *p_tag)
{
    char uid[3*MAX_UID_LENGTH+1];
    int len = p_tag->uid_length;
    if(len > MAX_UID_LENGTH)
        len = MAX_UID_LENGTH;

    for(int i = 0; i < len; i++)
    {
        sprintf(uid + (i*3), "%02x ", p_tag->uid[i]);
    }
    uid[len*3] = '\0';

    printf("UID: %s\n", uid);
}

int main(int argc, char ** argv)
{
    pthread_mutex_init(&mutex, NULL);

    // register signal handlers
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &interrupted;
    if (sigaction(SIGINT, &act, NULL) < 0 || sigaction(SIGTERM, &act, NULL))
    {
        perror("sigaction");
        exit(1);
    }

    // initialize nfcManager
    if(nfcManager_doInitialize())
    {
        perror("nfcManager_doInitialize");
        exit(1);
    }
    nfcTagCallback_t cb_tag;
    cb_tag.onTagArrival = tagArrived;
    cb_tag.onTagDeparture = tagDeparted;
    nfcManager_registerTagCallback(&cb_tag);

    // try to get versions from chip
    #ifdef NXP_HW_SELF_TEST
    printf("nfcFactory_GetMwVersion:\t0x%x\n", nfcFactory_GetMwVersion());
    #endif
    printf("nfcManager_getFwVersion:\t0x%x\n\n", nfcManager_getFwVersion());

    // start tag-discovery
    // enableDiscovery(technologies_masks, reader_only_mode, enable_host_routing, restart)
    nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 1, 0, 0);

    // main loop, performs handling of read tags
    while(do_poll)
    {
        nfc_tag_info_t *tag = NULL;

        pthread_mutex_lock(&mutex);
        if(v_tags_p.size())
        {
            tag = v_tags_p.front();
            v_tags_p.erase(v_tags_p.begin());
        }
        pthread_mutex_unlock(&mutex);

        if(tag)
        {
            printTagUID(tag);
            free(tag);
        }

        usleep(MAIN_SLEEP);
    }

    printf("\n\nAborting...\n");

    // free remaining tags
    pthread_mutex_lock(&mutex);
    while(v_tags_p.size())
    {
        nfc_tag_info_t *tag = v_tags_p.front();
        v_tags_p.erase(v_tags_p.begin());
        free(tag);
    }
    pthread_mutex_unlock(&mutex);

    // deinitialize nfcManager
    nfcManager_disableDiscovery();
    nfcManager_deregisterTagCallback();
    if(nfcManager_doDeinitialize())
    {
        perror("nfcManager_doDeinitialize");
        exit(1);
    }

    pthread_mutex_destroy(&mutex);

    return 0;
}
