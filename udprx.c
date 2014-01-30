/*
 * udprx.c - UDP Packet Generator Receiver
 *
 * Copyright (C) 2014 Citrix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "udp.h"

udpdata_t *udpdata;
int       npkts = 1;

void usage(char *argv0){
    fprintf(stderr, "RTFSC\n");
}

void dump(void){
    int                i;
    unsigned long long hz = get_hz();

    for (i=0; i<npkts; i++){
        printf("%5u %llu %llu (%llu) %llu\n", udpdata[i].seq, udpdata[i].tsctx,
               udpdata[i].tscrx, udpdata[i].tscrx-udpdata[i].tsctx,
               (udpdata[i].tscrx-udpdata[i].tsctx)*1000000000/hz);
    }
}

void sigterm_h(int signal){
   dump();
   exit(0);
}

int main(int argc, char **argv){
    // Local variables
    struct sockaddr_in our_addr;
    int                sock  = -1;
    unsigned int       slen  = 0;
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

    // Allocate receive buffer
    if ((udpdata = (udpdata_t *)calloc(npkts, sizeof(*udpdata))) == NULL)
    {
        perror("calloc");
        goto err;
    }

    // Handles kill
    signal(SIGINT, sigterm_h);

    // Setup UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket");
        fprintf(stderr, "Error creating temporary socket.\n");
        goto err;
    }
    memset(&our_addr, 0, sizeof(our_addr));
    our_addr.sin_family      = AF_INET;
    our_addr.sin_addr.s_addr = INADDR_ANY;
    our_addr.sin_port        = htons(UDPPORT);
    if (bind(sock, (struct sockaddr *)&our_addr, sizeof(struct sockaddr)) < 0){
        perror("bind");
        goto err;
    }

    // Receive packets
    for (i=0; i<npkts; i++){
        slen = sizeof(struct sockaddr);
        err = recvfrom(sock, &udpdata[i], sizeof(udpdata_t), 0, (struct sockaddr *)&our_addr, &slen);
        if (err < 0){
            perror("recvfrom");
            goto err;
        } else
        if (err != sizeof(udpdata_t)){
            fprintf(stderr, "Received unknown packet.\n");
            goto err;
        }
        udpdata[i].tscrx = rdtsc();
    }

    // Dump
    dump();

out:
    if (sock >= 0)
        close(sock);

    // Return
    return(err);

err:
    err = 1;
    goto out;
}
