#include <stdio.h>      // Used for printf() function to display output
#include <string.h>     // Used for strcpy() to copy strings (e.g., interface name)
#include <unistd.h>     // Used for read() and close() functions for socket operations
#include <net/if.h>     // Used for struct ifreq to configure network interfaces
#include <sys/types.h>  // Required for data types like `ssize_t` and `socket_t`
#include <sys/socket.h> // Used for socket(), bind(), and other socket-related functions
#include <sys/ioctl.h>  // Used for ioctl() to control the network device (e.g., to set CAN mode)
#include <linux/can.h>  // Defines CAN-specific structures and constants (e.g., struct can_frame)
#include <linux/can/raw.h> // Defines constants and structures specific to RAW CAN sockets

int main() {
    int s;
    int nbytes;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

    /* Create the CAN socket */
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("Error while opening socket");
        return -1;
    }

    /* Name of the CAN interface */
    strcpy(ifr.ifr_name, "can0");
    ioctl(s, SIOCGIFINDEX, &ifr);

    /* Bind the socket to the CAN interface */
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error in socket bind");
        return -2;
    }

    /* Read CAN frames and print them */
    while (1) {
        nbytes = read(s, &frame, sizeof(struct can_frame));
        if (nbytes < 0) {
            perror("CAN frame read error");
            return -3;
        }

        printf("Received CAN frame - ID: 0x%X Data: ", frame.can_id);
        for (int i = 0; i < frame.can_dlc; i++) {
            printf("%02X ", frame.data[i]);
        }
        printf("\n");
    }

    /* Close the socket */
    close(s);

    return 0;
}

