/*
 * Copyright 2026 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Protocol/packet type enums matching the QTI bluetooth hidl_transport
 * surface used by AntHci.cpp.
 */

#pragma once

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

enum ProtocolType {
  TYPE_BT = 0,
  TYPE_ANT,
};

// Values intentionally match the H4 wire packet types on the UART.
enum HciPacketType {
  HCI_PACKET_TYPE_UNKNOWN = 0,
  HCI_PACKET_TYPE_COMMAND = 0x01,
  HCI_PACKET_TYPE_ACL_DATA = 0x02,
  HCI_PACKET_TYPE_SCO_DATA = 0x03,
  HCI_PACKET_TYPE_EVENT = 0x04,
  HCI_PACKET_TYPE_ANT_CTRL = 0x0C,
  HCI_PACKET_TYPE_ANT_DATA = 0x0E,
};

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
