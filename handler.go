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

//export handle
func handle(id C.uint, buffer *C.uchar, len C.int, cbData *unsafe.Pointer) int {
	queueID := (*uint16)(unsafe.Pointer(cbData))
	q := queueRegistry.Get(*queueID)
	packet := &Packet{
		id:     uint32(id),
		Buffer: C.GoBytes(unsafe.Pointer(buffer), len),
		q:      q,
	}
	q.handler.Handle(packet)
	return 0
}
