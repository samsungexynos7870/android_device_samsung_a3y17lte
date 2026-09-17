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
 * Minimal ANT-only DataHandler surface for the QTI AntHci implementation
 * (ant/1.0/AntHci.cpp). On the a3y17lte open-source QCA9377 stack the
 * UART + firmware path belongs to libbt-vendor/wcnss_filter, so this
 * class routes the ANT channel through libbt-vendor's ANT userial ops
 * instead of the giant QTI UartController transport.
 */

#pragma once

#include <hidl/HidlSupport.h>
#include <atomic>
#include <functional>
#include <thread>

#include "types.h"

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_vec;
using InitializeCallback = std::function<void(bool success)>;
using DataReadCallback =
    std::function<void(HciPacketType, const hidl_vec<uint8_t>*)>;

class DataHandler {
 public:
  // Called on ANT HIDL init: opens the ANT channel over libbt-vendor and
  // runs init_cb(true/false) once the channel is usable.
  static bool Init(ProtocolType type, InitializeCallback init_cb,
      DataReadCallback data_read_cb);
  static void CleanUp(ProtocolType type);
  static DataHandler* Get();

  /* Sends an ANT packet to the controller ([type][len][payload]). */
  size_t SendData(ProtocolType type, HciPacketType packet_type,
                  const uint8_t *data, size_t length);

  unsigned int GetClientStatus(ProtocolType type);
  void SetClientStatus(bool status, ProtocolType type);

 private:
  DataHandler() = default;
  void InitThread(int type);
  void RxThread();

  InitializeCallback init_cb_;
  DataReadCallback data_read_cb_;
  int fd_ = -1;
  std::thread init_thread_;
  std::thread rx_thread_;
  std::atomic<bool> client_active_{false};
  std::atomic<bool> rx_running_{false};
  static DataHandler* data_handler;
};

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
