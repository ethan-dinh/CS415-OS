/* Name: Ethan Dinh
 * Class: CS 415 - Operating Systems
 * Professor: Joe Sventek
 * Project 2 - Threads
 */

/* Importing Libraries */
#include "packetdescriptor.h"
#include "pid.h"
#include "freepacketdescriptorstore.h"
#include "freepacketdescriptorstore__full.h"
#include "networkdevice.h"
#include "queue.h"
#include "BoundedBuffer.h"
#include "packetdriver.h"
#include "diagnostics.h"
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* Initializing Global Variables */
static NetworkDevice *nDevice = NULL;
static FreePacketDescriptorStore *fpds = NULL;
static BoundedBuffer *in[MAX_PID + 1];
static BoundedBuffer *procQ;
static BoundedBuffer *sendQ;

/* Definitions */
#define MAX_ATTEMPTS 5
#define MAX_SEND_BUF 10
#define MAX_PROC_BUF 4
#define MAX_RECEIVE_BUF 2

/* Functions required for threads */
static void* send_thread_func() {
    while (1) {
        // Wait for a packet to be available to send
        PacketDescriptor *pd;
        sendQ -> blockingRead(sendQ, (void **)&pd);

        // Attempt to send the packet
        int success = 0;
        for (int i = 0; i < MAX_ATTEMPTS && !success; i++) {
            if ((success = nDevice->sendPacket(nDevice, pd))) {
                DIAGNOSTICS("[Driver> Info: Packet sent after %d tr%s!\n", i+1, (i == 0) ? "y" : "ies");\
                fpds -> blockingPut(fpds, pd); // return the packetDescriptor to the free list
            }
        } if (!success) DIAGNOSTICS("[Driver> Info: Packet could not be sent after %d tries!\n", MAX_ATTEMPTS);
    } return NULL;
}

static void* receive_thread_func() {
    PacketDescriptor *curr;
    while (1) {
        fpds -> blockingGet(fpds, &curr);
        initPD(curr);
        nDevice -> registerPD(nDevice, curr);

        // Wait for the network to populate the current PacketDescriptor
        nDevice -> awaitIncomingPacket(nDevice);
        if(!(procQ -> nonblockingWrite(procQ, (void *)curr)))
            DIAGNOSTICS("[DRIVER> Info: Packet dropped!\n");
    } return NULL;
}

static void* process_thread_func() {
    while(1) {
        PacketDescriptor *curr;
        PID pid;

        // Wait for a packet to be available to process
        procQ -> blockingRead(procQ, (void **)&curr);

        // Get the PID of the packet
        pid = getPID(curr);

        // Enqueue the packet
        DIAGNOSTICS("[DRIVER> Packet received for application %u\n", pid);
        in[pid] -> blockingWrite(in[pid], (void *)curr);
        // fpds -> blockingPut(fpds, curr);
    } return NULL;
}

void blocking_send_packet(PacketDescriptor *pd) {
    sendQ -> blockingWrite(sendQ, (void *)pd);
}

int nonblocking_send_packet(PacketDescriptor *pd) {
    return sendQ -> nonblockingWrite(sendQ, (void *)pd);
}

void blocking_get_packet(PacketDescriptor **pd, PID pid) {
    in[pid] -> blockingRead(in[pid], (void **)pd);
}

int nonblocking_get_packet(PacketDescriptor **pd, PID pid) {
    return in[pid] -> nonblockingRead(in[pid], (void **)pd);
}

/* Initialize Data Structures and Start Internal Threads */
void init_packet_driver(NetworkDevice *nd, 
                        void *mem_start, 
                        unsigned long mem_length, 
                        FreePacketDescriptorStore **fpds_ptr) {
    
    // Initialize the global NetworkDevice
    nDevice = nd;

    // Initialize a PacketDescriptorStore with the given memory
    *fpds_ptr = FreePacketDescriptorStore_create(mem_start, mem_length);
    fpds = *fpds_ptr;

    // Initialize the buffers
    for (int i = 0; i < MAX_PID + 1; i++) {
        if ((in[i] = BoundedBuffer_create(MAX_RECEIVE_BUF)) == NULL) {
            printf("Error creating inbound buffer\n");
            exit(1);
        }
    }

    procQ = BoundedBuffer_create(MAX_PROC_BUF);
    sendQ = BoundedBuffer_create(MAX_SEND_BUF);

    // Initialize the send and receive threads
    pthread_t send_thread, receive_thread, process_thread;
    if((pthread_create(&send_thread, NULL, send_thread_func, NULL)) != 0) {
        fprintf(stderr, "Error creating send thread\n");
        exit(1);
    }
    
    if ((pthread_create(&receive_thread, NULL, receive_thread_func, NULL)) != 0) {
        fprintf(stderr, "Error creating receive thread\n");
        exit(1);
    }

    if ((pthread_create(&process_thread, NULL, process_thread_func, NULL)) != 0) {
        fprintf(stderr, "Error creating process thread\n");
        exit(1);
    }
}