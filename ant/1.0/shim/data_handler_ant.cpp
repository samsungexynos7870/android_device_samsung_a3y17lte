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
 * ANT DataHandler over libbt-vendor's ANT userial channel (a3y17lte,
 * QCA9377). BT_VND_OP_ANT_USERIAL_OPEN makes libbt-vendor power the chip
 * (and download the rampatch/NV on a cold start, coordinating with the
 * Bluetooth side through vendor.wc_transport.ref_count), start the
 * wcnss_filter mux if needed, and hand back an fd connected to the
 * daemon's "ant_sock" carrying full [type][len][payload] frames.
 */

#define LOG_TAG "com.qualcomm.qti.ant@1.0-DataHandler"

#include "data_handler.h"

#include <cutils/log.h>
#include "bt_vendor_lib.h"
#include "bt_vendor_qcom.h"

#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

DataHandler* DataHandler::data_handler = nullptr;

namespace {

/* Minimal no-op resource provider for libbt-vendor: on the ANT path the
 * vendor lib only uses fwcfg_cb (ignored - the ANT caller learns about
 * readiness through our init_cb once the channel fd exists). */
void IgnoreCfgResult(bt_vendor_op_result_t result) {
    ALOGV("%s: result %d (ignored on ANT channel)", __func__, result);
}

void* Allocate(int size) {
    return malloc(size);
}

void Deallocate(void* buf) {
    if (buf != nullptr) free(buf);
}

uint8_t TransmitCb(uint16_t /*opcode*/, void* /*buf*/, tINT_CMD_CBACK cback) {
    if (cback != nullptr) cback(nullptr);
    return 0;
}

void IgnoreA2dp(bt_vendor_op_result_t result, bt_vendor_opcode_t op,
                uint8_t handle) {
    ALOGV("%s: %d %d %d (ignored)", __func__, result, op, handle);
}

const bt_vendor_callbacks_t kNoopCallbacks = {
    sizeof(bt_vendor_callbacks_t), /* size */
    IgnoreCfgResult, /* fwcfg_cb */
    IgnoreCfgResult, /* scocfg_cb */
    IgnoreCfgResult, /* lpm_cb */
    IgnoreCfgResult, /* audio_state_cb */
    Allocate,        /* alloc */
    Deallocate,      /* dealloc */
    TransmitCb,      /* xmit_cb */
    IgnoreCfgResult, /* epilog_cb */
    IgnoreA2dp,      /* a2dp_offload_cb */
};

const bt_vendor_interface_t* VendorLibOpen() {
    void* handle = dlopen("libbt-vendor.so", RTLD_NOW);
    if (handle == nullptr) {
        ALOGE("%s: dlopen libbt-vendor.so failed: %s", __func__, dlerror());
        return nullptr;
    }
    const bt_vendor_interface_t* iface =
        (const bt_vendor_interface_t*)dlsym(handle,
                                            "BLUETOOTH_VENDOR_LIB_INTERFACE");
    if (iface == nullptr) {
        ALOGE("%s: BLUETOOTH_VENDOR_LIB_INTERFACE missing: %s", __func__,
              dlerror());
        dlclose(handle);
        return nullptr;
    }
    return iface;
}

/* Deliver one complete ANT frame's payload to the callback. */
inline HciPacketType WireToPacketType(uint8_t type) {
    return (type == HCI_PACKET_TYPE_ANT_CTRL ||
            type == HCI_PACKET_TYPE_ANT_DATA)
               ? (HciPacketType)type
               : HCI_PACKET_TYPE_UNKNOWN;
}

} // anonymous namespace

DataHandler* DataHandler::Get() {
    return data_handler;
}

bool DataHandler::Init(ProtocolType type, InitializeCallback init_cb,
                       DataReadCallback data_read_cb) {
    ALOGI("%s: type %d", __func__, type);
    if (type != TYPE_ANT) {
        ALOGE("%s: unsupported protocol type %d (ANT-only shim)", __func__,
              type);
        return false;
    }
    if (data_handler != nullptr) {
        ALOGE("%s: already initialized", __func__);
        return false;
    }

    data_handler = new DataHandler();
    data_handler->init_cb_ = init_cb;
    data_handler->data_read_cb_ = data_read_cb;
    data_handler->init_thread_ =
        std::thread(&DataHandler::InitThread, data_handler, (int)type);
    return true;
}

void DataHandler::CleanUp(ProtocolType type) {
    ALOGI("%s: type %d", __func__, type);
    DataHandler* thiz = data_handler;
    if (thiz == nullptr || type != TYPE_ANT)
        return;

    thiz->SetClientStatus(false, TYPE_ANT);

    const bt_vendor_interface_t* iface = VendorLibOpen();
    if (iface != nullptr && iface->op != nullptr) {
        iface->op((bt_vendor_opcode_t)BT_VND_OP_ANT_USERIAL_CLOSE, nullptr); /* closes fd_ on
                                                            the lib side */
    } else if (thiz->fd_ >= 0) {
        close(thiz->fd_);
    }
    thiz->fd_ = -1;

    if (thiz->rx_thread_.joinable())
        thiz->rx_thread_.join();
    if (thiz->init_thread_.joinable())
        thiz->init_thread_.join();

    data_handler = nullptr;
    delete thiz;
}

unsigned int DataHandler::GetClientStatus(ProtocolType type) {
    (void)type;
    return client_active_ ? 1 : 0;
}

void DataHandler::SetClientStatus(bool status, ProtocolType type) {
    (void)type;
    client_active_ = status;
}

void DataHandler::InitThread(int type) {
    bool ok = false;
    const bt_vendor_interface_t* iface = VendorLibOpen();
    uint8_t bdaddr[6] = {0};

    if (iface == nullptr || iface->init == nullptr || iface->op == nullptr) {
        ALOGE("%s: vendor interface unusable", __func__);
    } else if (iface->init(&kNoopCallbacks, bdaddr) == 0 &&
               fd_ == -1) {
        if (iface->op((bt_vendor_opcode_t)BT_VND_OP_ANT_USERIAL_OPEN, &fd_) == 0 && fd_ >= 0) {
            ALOGI("%s: ANT channel up (fd %d)", __func__, fd_);
            SetClientStatus(true, (ProtocolType)type);
            rx_running_ = true;
            rx_thread_ = std::thread(&DataHandler::RxThread, this);
            ok = true;
        } else {
            ALOGE("%s: BT_VND_OP_ANT_USERIAL_OPEN failed", __func__);
        }
    } else {
        ALOGE("%s: vendor init failed", __func__);
    }

    init_cb_(ok);
}

void DataHandler::RxThread() {
    /* ANT frames from wcnss_filter arrive as [type][len][payload]; parse
     * and forward the payload to the right IAntHciCallbacks method. */
    uint8_t frame[2 + 255];
    size_t have = 0;

    ALOGI("%s: started (fd %d)", __func__, fd_);
    while (rx_running_ && fd_ >= 0) {
        ssize_t rd = read(fd_, frame + have, sizeof(frame) - have);
        if (rd <= 0) {
            if (rd < 0 && errno == EINTR)
                continue;
            if (rd < 0)
                ALOGE("%s: read failed: %s", __func__, strerror(errno));
            else
                ALOGI("%s: channel closed", __func__);
            break;
        }
        have += (size_t)rd;

        size_t off = 0;
        while (have - off >= 2) {
            uint8_t type = frame[off];
            uint8_t len = frame[off + 1];
            size_t total = 2 + (size_t)len;
            if (have - off < total)
                break; /* wait for the rest of the frame */
            HciPacketType ptype = WireToPacketType(type);
            if (ptype != HCI_PACKET_TYPE_UNKNOWN &&
                data_read_cb_ != nullptr) {
                hidl_vec<uint8_t> packet;
                packet.resize(len);
                memcpy(packet.data(), frame + off + 2, len);
                data_read_cb_(ptype, &packet);
            } else if (ptype == HCI_PACKET_TYPE_UNKNOWN) {
                ALOGE("%s: unknown ANT frame type 0x%02x len %u, dropped",
                      __func__, type, len);
            }
            off += total;
        }
        if (off > 0) {
            memmove(frame, frame + off, have - off);
            have -= off;
        }
    }
    rx_running_ = false;
    ALOGI("%s: stopped", __func__);
}

size_t DataHandler::SendData(ProtocolType type, HciPacketType packet_type,
                             const uint8_t* data, size_t length) {
    if (type != TYPE_ANT || fd_ < 0)
        return 0;
    if (length > 255 || data == nullptr) {
        ALOGE("%s: invalid length %zu (ANT frame max 255)", __func__, length);
        return 0;
    }

    uint8_t frame[2 + 255];
    frame[0] = (uint8_t)packet_type;
    frame[1] = (uint8_t)length;
    memcpy(frame + 2, data, length);

    size_t total = 2 + length, written = 0;
    while (written < total) {
        ssize_t wr = write(fd_, frame + written, total - written);
        if (wr < 0) {
            if (errno == EINTR)
                continue;
            ALOGE("%s: write failed: %s", __func__, strerror(errno));
            return 0;
        }
        written += (size_t)wr;
    }
    return length;
}

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
