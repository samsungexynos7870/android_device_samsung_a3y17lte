/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch-service.a3y17lte"

#include <string>

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "HighTouchPollingRate.h"
#include "KeyDisabler.h"

using aidl::vendor::lineage::touch::HighTouchPollingRate;
using aidl::vendor::lineage::touch::KeyDisabler;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    std::shared_ptr<HighTouchPollingRate> htpr = ndk::SharedRefBase::make<HighTouchPollingRate>();
    if (htpr->isSupported()) {
        const std::string instance = std::string(HighTouchPollingRate::descriptor) + "/default";
        binder_status_t status = AServiceManager_addService(htpr->asBinder().get(), instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR) << "Failed to add service " << instance << ": " << status;
        }
    }

    std::shared_ptr<KeyDisabler> kd = ndk::SharedRefBase::make<KeyDisabler>();
    if (kd->isSupported()) {
        const std::string instance = std::string(KeyDisabler::descriptor) + "/default";
        binder_status_t status = AServiceManager_addService(kd->asBinder().get(), instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR) << "Failed to add service " << instance << ": " << status;
        }
    }

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
