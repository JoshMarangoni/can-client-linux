#include <stdio.h>         // Used for printf() function to display output
#include <string.h>        // Used for strcpy() to copy strings (e.g., interface name)
#include <unistd.h>        // Used for read() and close() functions for socket operations
#include <net/if.h>        // Used for struct ifreq to configure network interfaces
#include <sys/socket.h>    // Used for socket(), bind(), and other socket-related functions
#include <sys/ioctl.h>     // Used for ioctl() to control the network device (e.g., to set CAN mode)
#include <linux/can.h>     // Defines CAN-specific structures and constants (e.g., struct can_frame)
#include <linux/can/raw.h> // Defines constants and structures specific to RAW CAN sockets

int main() {
    int sockfd;
    int nbytes;

    // -----------------------------------------------------------------------------
    // For reference:
    //
    // struct sockaddr_can {
    //     sa_family_t can_family;
    //     int         can_ifindex;
    //     union {
    //             /* transport protocol class address info (e.g. ISOTP) */
    //             struct { canid_t rx_id, tx_id; } tp;
    //
    //             /* reserved for future CAN protocols address information */
    //     } can_addr;
    // };
    struct sockaddr_can sockaddr;

    // -----------------------------------------------------------------------------
    // For reference:
    //
    // struct ifreq {
    //     char ifr_name[IFNAMSIZ]; /* Interface name */
    //     union {
    //         struct sockaddr ifr_addr;
    //         struct sockaddr ifr_dstaddr;
    //         struct sockaddr ifr_broadaddr;
    //         struct sockaddr ifr_netmask;
    //         struct sockaddr ifr_hwaddr;
    //         short           ifr_flags;
    //         int             ifr_ifindex;
    //         int             ifr_metric;
    //         int             ifr_mtu;
    //         struct ifmap    ifr_map;
    //         char            ifr_slave[IFNAMSIZ];
    //         char            ifr_newname[IFNAMSIZ];
    //         char           *ifr_data;
    //     } ifr_ifru;
    // };
    struct ifreq ifr;  // interface request variable

    // -----------------------------------------------------------------------------
    // For reference:
    //
    // struct can_frame {
    //     canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    //     __u8    can_dlc; /* frame payload length in byte (0 .. 8) */
    //     __u8    __pad;   /* padding */
    //     __u8    __res0;  /* reserved / padding */
    //     __u8    __res1;  /* reserved / padding */
    //     __u8    data[8] __attribute__((aligned(8)));
    // };
    struct can_frame frame;
    // -----------------------------------------------------------------------------

    /* Create the CAN socket */
    /* Notes:
        - A socket is an endpoint for sending and receiving data on a computer network.
        - PF_CAN
            - stands for "Protocol Family CAN"
        - SOCK_RAW
            - allows direct access to lower-level protocols
            - Using SOCK_RAW means that your application is responsible for handling the
            CAN protocol's own headers and payload, providing more control over the communication.
        - CAN_RAW
            - This is the specific protocol to be used with the socket
            - CAN_RAW indicates that the raw CAN protocol is being used, as opposed
              to a higher-level protocol like ISO-TP
    */
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 0) {
        perror("Error while opening socket");
        return -1;
    }

    /* Name of the CAN interface */
    strcpy(ifr.ifr_name, "can0");
    /* Notes:
        - SIOCGIFINDEX
            - a constant that stands for "Socket I/O Control Get Interface INDEX"
            - It is a request to retrieve the index number of a network interface
            - interface index is a unique number assigned to each network interface on the system
        - After the call, the ifr_ifindex member of the struct ifreq will be filled with the
          interface index.
    */
    ioctl(sockfd, SIOCGIFINDEX, &ifr); // system call

    /* Bind the socket to the CAN interface */
    sockaddr.can_family = AF_CAN; // "Address Family CAN"
    sockaddr.can_ifindex = ifr.ifr_ifindex; // pass the system interface index

    if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
        perror("Error in socket bind");
        return -2;
    }

   /* Read CAN frames and print them */
    while (1) {
        nbytes = read(sockfd, &frame, sizeof(struct can_frame));
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
    close(sockfd);

    return 0;
}

