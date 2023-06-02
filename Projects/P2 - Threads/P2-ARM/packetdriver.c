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
static BoundedBuffer *PDPool;
static BoundedBuffer *sendQ;

/* Definitions */
#define MAX_ATTEMPTS 5
#define MAX_SEND_BUF 10
#define MAX_POOL_BUF 5
#define MAX_APP_BUF 2

/* DIAGNOSTICS */
#define FAIL_SEND "[Driver> Info: Packet could not be sent after %d tries!\n"
#define APP_RECEIVED "[DRIVER> Packet stored for application %u\n"
#define SUCCESS_SEND "[Driver> Info: Packet sent after %d tr%s!\n"
#define APP_FULL "[DRIVER> Warning: Packet could not be enqueued for application %u\n"
#define FULL_STORE "[DRIVER> Warning: FreePacketDescriptorStore is full!\n"

/* Functions required for threads */
static void* send_thread_func() {
    while (1) {
        // Wait for a packet to be available to send
        PacketDescriptor *pd;
        sendQ -> blockingRead(sendQ, (void **)&pd);

        // Attempt to send the packet
        int success = 0;
        for (int i = 0; i < MAX_ATTEMPTS && !success; i++) {
            if ((success = nDevice->sendPacket(nDevice, pd)))
                DIAGNOSTICS(SUCCESS_SEND, i+1, (i == 0) ? "y" : "ies");
        } if (!success) DIAGNOSTICS(FAIL_SEND, MAX_ATTEMPTS);

        // return the packetDescriptor to the pool or free list
        if (PDPool -> nonblockingWrite(PDPool, (void *)pd) == 0)
            fpds -> blockingPut(fpds, pd); 
    } return NULL;
}

static void* receive_thread_func() {
    PacketDescriptor *curr;
    PID pid; int success;
    while (1) {
        // Retrieve a packet descriptor from the pool
        if (!(success = PDPool -> nonblockingRead(PDPool, (void **)&curr)))
            success = fpds -> nonblockingGet(fpds, &curr);

        if(success) {
            initPD(curr);
            nDevice -> registerPD(nDevice, curr);

            // Wait for the network to populate the current PacketDescriptor
            nDevice -> awaitIncomingPacket(nDevice);

            // Get the PID of the current packet
            pid = getPID(curr);

            // Enqueue the packet
            if(!(success = in[pid] -> nonblockingWrite(in[pid], (void *)curr))) {
                DIAGNOSTICS (APP_FULL, pid);
                
                // return the packetDescriptor to the pool or free list
                if (PDPool -> nonblockingWrite(PDPool, (void *) curr) == 0)
                    fpds -> blockingPut(fpds, curr); 
            }
                
            DIAGNOSTICS(APP_RECEIVED, pid);
        } else DIAGNOSTICS(FULL_STORE);
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

    // Initialize the application buffers
    for (int i = 0; i < MAX_PID + 1; i++) {
        if ((in[i] = BoundedBuffer_create(MAX_APP_BUF)) == NULL) {
            printf("Error creating inbound buffer\n");
            exit(1);
        }
    }

    // Initialize the PD pool buffer
    PacketDescriptor *pd;
    PDPool = BoundedBuffer_create(MAX_POOL_BUF);
    for (int i = 0; i < MAX_POOL_BUF; i++) {
        fpds -> blockingGet(fpds, &pd);
        PDPool -> blockingWrite(PDPool, (void *)pd);
    }

    // Initialize the outbound buffer
    sendQ = BoundedBuffer_create(MAX_SEND_BUF);

    // Initialize the send and receive threads
    pthread_t send_thread, receive_thread;
    if((pthread_create(&send_thread, NULL, send_thread_func, NULL)) != 0) {
        fprintf(stderr, "Error creating send thread\n");
        exit(1);
    }
    
    if ((pthread_create(&receive_thread, NULL, receive_thread_func, NULL)) != 0) {
        fprintf(stderr, "Error creating receive thread\n");
        exit(1);
    }
}