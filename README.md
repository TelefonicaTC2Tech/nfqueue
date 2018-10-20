[![Build Status](https://api.travis-ci.org/Telefonica/nfqueue.svg?branch=master)](https://travis-ci.org/Telefonica/nfqueue) [![](https://godoc.org/github.com/Telefonica/nfqueue?status.svg)](http://godoc.org/github.com/Telefonica/nfqueue) [![](http://goreportcard.com/badge/Telefonica/nfqueue)](http://goreportcard.com/report/Telefonica/nfqueue)

# nfqueue

nfqueue is a go binding to [libnetfilter_queue](http://www.netfilter.org/projects/libnetfilter_queue).

It implements partially the libnetfilter_queue functionality. Currently, it provides:

 - Start a netfilter queue
 - Stop a netfilter queue
 - Handle a packet received by netfilter queue and accept, drop or modify it.
 - Configure the netfilter queue

See [more information](https://home.regit.org/netfilter-en/using-nfqueue-and-libnetfilter_queue) about libnetfilter_queue.

## Using library

```import "github.com/Telefonica/nfqueue"```

## Netfilter queue configuration

It is possible to pass a configuration object with optional settings to tune the netfilter queue.

| Setting | Description |
| ----- | ----------- |
| MaxPackets | Maximum amount of packets that can be enqueued in a netfilter queue |
| QueueFlags | Slice of QueueFlag elements. See following table with the list of possible queue flags |
| BufferSize | Modify the default nfnetlink buffer size |

```
// QueueConfig contains optional configuration parameters to initialize a queue.
type QueueConfig struct {
	MaxPackets uint32
	QueueFlags []QueueFlag
	BufferSize uint32
}
```

The following table lists the queue flags. Note that some queue flags may be unavailable depending on the linux kernel version.

| QueueFlag | Description |
| --------- | ----------- |
| FailOpen | (requires Linux kernel >= 3.6) The kernel will accept the packets if the kernel queue gets full. If this flag is not set, the default action in this case is to drop packets. |
| Conntrack | (requires Linux kernel >= 3.6) The kernel will include the Connection Tracking system information. |
| GSO | (requires Linux kernel >= 3.10) The kernel will not normalize offload packets, i.e. your application will need to be able to handle packets larger than the mtu. |
| UIDGid | It makes the kernel dump UID and GID of the socket to which each packet belongs. |
| Secctx | It makes the kernel dump security context of the socket to which each packet belongs. |

## Packet and packet handler

Each packet that is processed by a netfilter queue is encapsulated in the type `Packet` that contains a byte slice with the packet buffer (starting with the IP layer). This buffer can be parsed with [gopacket](https://godoc.org/github.com/google/gopacket). It also provides an interface to accept, drop or modify the packet.

```
// Packet struct provides the packet data and methods to accept, drop or modify the packet.
type Packet struct {
	Buffer []byte
	id     uint32
	q      *Queue
}

// Accept the packet.
func (p *Packet) Accept() error {...}
// Drop the packet.
func (p *Packet) Drop() error {...}
// Modify the packet with a new buffer.
func (p *Packet) Modify(buffer []byte) error {...}
```

To receive these packets, the client needs to implement the `PacketHandler` interface.

```
// PacketHandler is an interface to handle a packet retrieved by netfilter.
type PacketHandler interface {
	Handle(p *Packet)
}
```

## Example

The following example instantiates a netfilter queue (with 1 as queue identifier) and the packet handler accepts all the packets.

```
import (
	"github.com/Telefonica/nfqueue"
)

type Queue struct {
	id    uint16
	queue *nfqueue.Queue
}

func NewQueue(id uint16) *Queue {
	q := &Queue{
		id: id,
	}
	queueCfg := &nfqueue.QueueConfig{
		MaxPackets: 1000,
		BufferSize: 16 * 1024 * 1024,
		QueueFlags: []nfqueue.QueueFlag{nfqueue.FailOpen},
	}
	// Pass as packet handler the current instance because it implements nfqueue.PacketHandler interface
	q.queue = nfqueue.NewQueue(q.id, q, queueCfg)
	return q
}

// Start the queue.
func (q *Queue) Start() error {
	return q.queue.Start()
}

// Stop the queue.
func (q *Queue) Stop() error {
	return q.queue.Stop()
}

// Handle a nfqueue packet. It implements nfqueue.PacketHandler interface.
func (q *Queue) Handle(p *nfqueue.Packet) {
	// Accept the packet
	p.Accept()
}

func main() {
	q := NewQueue(1)
	go q.Start()
}
```

## Performance

This library is inspired in other nfqueue go bindings. The main difference is that our design does not use go channels to process the packets to avoid introducing an overhead that degrades performance. Note that nfqueue cannot process multiple packets from the same queue concurrently.

The following table compares nfqueue with [freki](https://github.com/kung-foo/freki) using a host with 4 CPU cores:

| Library | 2 queues | 4 queues |
| ------- | -------- | -------- |
| freki | 25000 req/s | 22000 req/s |
| nfqueue | 22000 req/s | 38500 req/s |

Tuning the amount of nfqueue queues with the amount of CPU cores, this library achieves a 75% improvement over freki.


## License

Copyright 2018 [Telefónica Investigación y Desarrollo, S.A.U](http://www.tid.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
