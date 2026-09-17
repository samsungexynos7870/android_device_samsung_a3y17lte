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
 * Passthrough HIDL service for the ANT HCI HAL: registers
 * com.qualcomm.qti.ant@1.0::IAntHci/default so the ANT Radio Service
 * (com.dsi.ant.server, via libantradio) can bind to us. In stock this
 * instance was registered by the prebuilt QTI bluetooth service; here it
 * gets its own small service, sharing the hal_bluetooth_default SELinux
 * domain via seclabel so it may load/link libbt-vendor and talk to the
 * wcnss_filter daemon's abstract sockets.
 */

#include <hidl/LegacySupport.h>
#include <com/qualcomm/qti/ant/1.0/IAntHci.h>
#include <utils/Errors.h>
#include <utils/Log.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

int main() {
    configureRpcThreadpool(1, true /* callerWillJoin */);
    ::android::status_t status =
        android::hardware::registerPassthroughServiceImplementation<
            ::com::qualcomm::qti::ant::V1_0::IAntHci>();
    LOG_ALWAYS_FATAL_IF(status != android::OK,
                        "Failed to register com.qualcomm.qti.ant@1.0::IAntHci");
    joinRpcThreadpool();
    return 0;
}
