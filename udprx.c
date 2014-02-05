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
int       bufsz = sizeof(udpdata_t);

void usage(char *argv0){
    fprintf(stderr, "RTFSC\n");
}

void dump(void){
    int                i;
    unsigned long long hz = get_hz();
    int                last_recv_pkt = -1;
    int                total_pkts_dropped = npkts;
    float              tput_mbs;

    for (i=0; i<npkts; i++){
        printf("%5u %llu %llu (%llu) %llu\n", udpdata[i].seq, udpdata[i].tsctx,
               udpdata[i].tscrx, udpdata[i].tscrx-udpdata[i].tsctx,
               (udpdata[i].tscrx-udpdata[i].tsctx)*1000000/hz);

        if(udpdata[i].tsctx != 0) {
            last_recv_pkt = i;
            --total_pkts_dropped;
        }
    }

    // Print throughput
    if (last_recv_pkt >= 0) {
        tput_mbs = (npkts - total_pkts_dropped) * bufsz /
            ((udpdata[last_recv_pkt].tscrx - udpdata[0].tsctx)/hz) /
            1000000.0;
    } else {
        tput_mbs = 0.0;
    }
    fprintf(stderr, "Average throughput: %.0f MB/s = %.2f Gbit/s\n",
            tput_mbs, tput_mbs * 8 / 1000);

    // Print number of dropped packets
    fprintf(stderr, "Dropped packets: %d of %d (%.1f%%)\n",
            total_pkts_dropped, npkts, total_pkts_dropped*100.0 / npkts);
}

void sigterm_h(int signal){
   dump();
   exit(0);
}

int main(int argc, char **argv){
    // Local variables
    struct sockaddr_in our_addr;
    void               *buf  = NULL;
    int                sock  = -1;
    unsigned int       slen  = 0;
    int                err   = 0;
    int                i;

    // Parse parameters
    while ((i = getopt(argc, argv, "n:s:")) != -1){
        switch(i){
        case 'n': // Number of packets
            npkts = atoi(optarg);
            break;

        case 's': // Payload size
            bufsz = atoi(optarg);
            if (bufsz < sizeof(udpdata_t)){
                fprintf(stderr, "Payload size must be at least %zd bytes\n",
                        sizeof(udpdata_t));
                goto err;
            }
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
    if ((buf = malloc(bufsz)) == NULL){
        perror("malloc");
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
        err = recvfrom(sock, buf, bufsz, 0, (struct sockaddr *)&our_addr, &slen);
        if (err < 0){
            perror("recvfrom");
            goto err;
        } else
        if (err != bufsz){
            fprintf(stderr, "Received unknown packet.\n");
            goto err;
        }
        memcpy(&udpdata[i], buf, sizeof(udpdata_t));
        udpdata[i].tscrx = rdtsc();
    }

    // Dump
    dump();

out:
    if (sock >= 0)
        close(sock);
    if (buf)
        free(buf);

    // Return
    return(err);

err:
    err = 1;
    goto out;
}
