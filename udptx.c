#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "udp.h"

void usage(char *argv0){
    fprintf(stderr, "RTFM\n");
}

int main(int argc, char **argv){
    // Local variables
    struct in_addr     dst_ip;
    struct sockaddr_in dst_sock;
    udpdata_t          udpdata;
    int                seq   = 0;
    int                sock  = -1;
    int                npkts = 1;
    int                err   = 0;
    int                i;

    // Parse parameters
    while ((i = getopt(argc, argv, "n:")) != -1){
        switch(i){
        case 'n': // Number of packets
            npkts = atoi(optarg);
            break;
        default:
            usage(argv[0]);
            goto err;
        }
    }
    if (argc != optind+1){
        usage(argv[0]);
        goto err;
    }
    if (inet_aton(argv[optind], &dst_ip) == 0){
        fprintf(stderr, "Invalid IP address: %s\n", optarg);
        goto err;
    }
    printf("Sending to: %s\n", inet_ntoa(dst_ip));

    // Setup UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket");
        fprintf(stderr, "Error creating temporary socket.\n");
        goto err;
    }

    // Send packets
    dst_sock.sin_family = AF_INET;
    memcpy(&dst_sock.sin_addr, &dst_ip, sizeof(dst_sock.sin_addr));
    dst_sock.sin_port = htons(UDPPORT);

    for (i=0; i<npkts; i++){
        udpdata.seq   = seq++;
        udpdata.tsctx = rdtsc();

        if (sendto(sock, &udpdata, sizeof(udpdata_t), 0, (struct sockaddr *)&dst_sock, sizeof(dst_sock)) < 0){
            perror("sendto");
            fprintf(stderr, "Error sending UDP packet.\n");
            goto err;
        }
    }

out:
    if (sock >= 0)
        close(sock);

    // Return
    return(err);

err:
    err = 1;
    goto out;
}
