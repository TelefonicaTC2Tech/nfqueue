/**
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

package nfqueue

// queueRegistry is a singleton with the list of queues registered to dispatch the packets to the appropriate queue.
var queueRegistry = NewQueueRegistry()

// QueueRegistry stores a list of the netfilter queues registered.
// It returns a Queue based on the queue ID so that it is possible to build up
// a singleton registry to interact with netfilter library.
// This registry is not thread-safe on purpose. Queues should be registered before started.
type QueueRegistry struct {
	queues []*Queue
}

// NewQueueRegistry creates an instance of QueueRegistry.
func NewQueueRegistry() *QueueRegistry {
	return &QueueRegistry{
		queues: []*Queue{},
	}
}

// Register adds a queue to the registry.
func (r *QueueRegistry) Register(queueID uint16, queue *Queue) {
	if len(r.queues) <= int(queueID) {
		// Increase the capacity of the slice to store the new queue
		queues := make([]*Queue, queueID+1)
		copy(queues, r.queues)
		r.queues = queues
	}
	r.queues[queueID] = queue
}

// Unregister removes a queue from the registry.
func (r *QueueRegistry) Unregister(queueID uint16) {
	if len(r.queues) > int(queueID) {
		r.queues[queueID] = nil
	}
}

// Get returns a queue from the registry based on the queueID.
func (r *QueueRegistry) Get(queueID uint16) *Queue {
	if len(r.queues) > int(queueID) {
		return r.queues[queueID]
	}
	return nil
}
