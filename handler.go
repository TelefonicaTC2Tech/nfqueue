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

import "C"
import (
	"unsafe"
)

// Main distribs still use libnetfilter_queue 1.0.2, which does not contain UID and GID stuff
// To work on most systems, I disable them for now

//export handle
func handle(
	id uint32, buffer *C.uchar, len C.int,
	inDev, outDev, physInDev, physOutDev, nfMark uint32, hwAddrlen, hwPad uint16,
	// hasUID, hasGID int, uid, gid, inDev, outDev, physInDev, physOutDev, nfMark uint32, hwAddrlen, hwPad uint16,
	hwAddress0, hwAddress1, hwAddress2, hwAddress3, hwAddress4, hwAddress5, hwAddress6, hwAddress7 uint8,
	queueID int) int {
	q := queueRegistry.Get(uint16(queueID))
	if q == nil {
		return 0
	}
	packet := &Packet{
		id:     id,
		Buffer: C.GoBytes(unsafe.Pointer(buffer), len),
		q:      q,
		Meta: &PacketMeta{
			HasUID: false,
			UID:    0,
			HasGID: false,
			GID:    0,
			// HasUID:     hasUID == 1,
			// UID:        uid,
			// HasGID:     hasGID == 1,
			// GID:        gid,
			InDev:      inDev,
			OutDev:     outDev,
			PhysInDev:  physInDev,
			PhysOutDev: physOutDev,
			NFMark:     nfMark,
			HWAddr: []byte{
				hwAddress0,
				hwAddress1,
				hwAddress2,
				hwAddress3,
				hwAddress4,
				hwAddress5,
				hwAddress6,
				hwAddress7,
			},
		},
	}
	q.handler.Handle(packet)
	return 0
}
