#include <stdio.h>
#include <string.h>

int main() {
    char *fileNames[2] = {"logfile.txt", "mylogfile.txt"};

    for (int i = 0; i < 2; i++) {
        FILE* file = fopen(fileNames[i], "r");
        if (file == NULL) {
            printf("Could not open file\n");
            return 1;
        }

        char line[256];
        int packetReceived = 0, packetToApp = 0, failedToSend = 0, successfulSend = 0, successfulPacket = 0;

        while (fgets(line, sizeof(line), file)) {
            int pid;
            if (sscanf(line, "[Device> Packet received for PID %d", &pid) == 1) {
                packetReceived++;
                continue;
            }

            if (strstr(line, "[Device> packet successfully sent") != NULL) {
                successfulPacket++;
                continue;
            }

            int app_id;
            char prefix[30], rest[200];
            if (sscanf(line, "%s application %d %[^\n]", prefix, &app_id, rest) == 3) {
                if (strcmp(rest, "has received a packet.") == 0) {
                    packetToApp++;
                } else if (strcmp(rest, "failed to send a packet descriptor (blocking).") == 0) {
                    failedToSend++;
                } else if (strcmp(rest, "failed to send a packet descriptor (nonblocking).") == 0) {
                    failedToSend++;
                } else if (strcmp(rest, "has sent a packet descriptor (blocking).") == 0) {
                    successfulSend++;
                } else if (strcmp(rest, "has sent a packet descriptor (nonblocking).") == 0) {
                    successfulSend++;
                }
            }
        } int packetsLost = packetReceived - packetToApp;

        printf("Successfully queued writes: %d\n", successfulSend);
        printf("Failed queued writes: %d\n", failedToSend);
        printf("Successfully sent writes: %d\n", successfulPacket);
        printf("Unsuccessfully sent writes: %d\n", successfulSend - successfulPacket);
        printf("Packets received by device: %d\n", packetReceived);
        printf("Packets retrieved by applications: %d\n", packetToApp);
        printf("Packets lost due to congestion: %d\n\n", packetsLost);

        fclose(file);
    } return 0;
}
