/*
 * @license
 * Copyright 2018 Telefónica Investigación y Desarrollo, S.A.U
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _GO_NFQUEUE_H
#define _GO_NFQUEUE_H

// XXX
#include <stdio.h>

#include <netinet/in.h>
#include <unistd.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

// Maximum packet size of a TCP packet
const uint MAX_PACKET_SIZE = 65535;


// handle is the packet handler function implemented in go.
// The arguments are:
// - id (packet identifier)
// - buffer (pointer to the packet data starting from IP layer)
// - len (buffer length)
// - had_uid (1 if the packet has an assigned UID)
// - had_gid (1 if the packet has an assigned GID)
// - uid (the UID assigned to the packet)
// - gid (the GID assigned to the packet)
// - indev (input device index)
// - outdev (output device index)
// - physindev (physical input device index)
// - physoutdev (physical output device index)
// - nfmark (NF mark for the packet)
// - hw_addrlen (length of the HW address)
// - hw_pad (padding of the HW address)
// - hw_addr (HW address)
// - queue_id (queue identifier)
extern int handle(uint32_t id, unsigned char* buffer, int len, int has_uid, int has_gid, u_int32_t uid, u_int32_t gid, u_int32_t indev, u_int32_t outdev, u_int32_t physindev, u_int32_t physoutdev, u_int32_t nfmark, u_int16_t hw_addrlen, u_int16_t hw_pad, u_int8_t hw_addr0, u_int8_t hw_addr1, u_int8_t hw_addr2, u_int8_t hw_addr3, u_int8_t hw_addr4, u_int8_t hw_addr5, u_int8_t hw_addr6, u_int8_t hw_addr7, int queue_id);

int nfqueue_cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *cb_data)
{
    unsigned char *buffer = NULL;
    struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfa);
    uint32_t id = ntohl(ph->packet_id);
    int ret = nfq_get_payload(nfa, &buffer);
    u_int32_t uid = 0;
    u_int32_t gid = 0;
    int has_uid = nfq_get_uid(nfa, &uid);
    int has_gid = nfq_get_gid(nfa, &gid);
    u_int32_t indev = nfq_get_indev(nfa);
    u_int32_t outdev = nfq_get_outdev(nfa);
    u_int32_t physindev = nfq_get_physindev(nfa);
    u_int32_t physoutdev = nfq_get_physoutdev(nfa);
    u_int32_t nfmark = nfq_get_nfmark(nfa);
    struct nfqnl_msg_packet_hw *packet_hw = nfq_get_packet_hw(nfa);
    u_int16_t hw_addrlen = 0;
    u_int16_t hw_pad = 0;
    u_int8_t hw_addr0 = 0;
    u_int8_t hw_addr1 = 0;
    u_int8_t hw_addr2 = 0;
    u_int8_t hw_addr3 = 0;
    u_int8_t hw_addr4 = 0;
    u_int8_t hw_addr5 = 0;
    u_int8_t hw_addr6 = 0;
    u_int8_t hw_addr7 = 0;
    if (packet_hw != NULL) {
        hw_addrlen = packet_hw->hw_addrlen;
        hw_pad = packet_hw->_pad;
        hw_addr0 = packet_hw->hw_addr[0];
        hw_addr1 = packet_hw->hw_addr[1];
        hw_addr2 = packet_hw->hw_addr[2];
        hw_addr3 = packet_hw->hw_addr[3];
        hw_addr4 = packet_hw->hw_addr[4];
        hw_addr5 = packet_hw->hw_addr[5];
        hw_addr6 = packet_hw->hw_addr[6];
        hw_addr7 = packet_hw->hw_addr[7];
    }
    return handle(id, buffer, ret, has_uid, has_gid, uid, gid, indev, outdev, physindev, physoutdev, nfmark, hw_addrlen, hw_pad, hw_addr0, hw_addr1, hw_addr2, hw_addr3, hw_addr4, hw_addr5, hw_addr6, hw_addr7, (intptr_t)cb_data);
}

static struct nfq_q_handle *nfqueue_create_queue(struct nfq_handle *h, u_int16_t queue_id) {
    return nfq_create_queue(h, queue_id, &nfqueue_cb, (void *)(intptr_t)queue_id);
}

static int nfqueue_loop(struct nfq_handle *h, int fd)
{
    char buf[MAX_PACKET_SIZE] __attribute__ ((aligned));
    int rv;

    // Avoid ENOBUFS on read() operation, otherwise the while loop is interrupted.
    int opt = 1;
    if (setsockopt(fd, SOL_NETLINK, NETLINK_NO_ENOBUFS, &opt, sizeof(int)) < 0) {
        return -1;
    }

    while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
        nfq_handle_packet(h, buf, rv);
    }

    return 0;
}

#endif
