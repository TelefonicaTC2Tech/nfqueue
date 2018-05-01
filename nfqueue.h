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
// - queue_id (queue identifier)
extern int handle(uint32_t id, unsigned char* buffer, int len, int queue_id);

int nfqueue_cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *cb_data)
{
    unsigned char *buffer = NULL;
    struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfa);
    uint32_t id = ntohl(ph->packet_id);
    int ret = nfq_get_payload(nfa, &buffer);
    return handle(id, buffer, ret, (intptr_t)cb_data);
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
