/*
   Copyright (c) 2019, The LineageOS Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/logging.h>

#include "vendor_init.h"
#include "property_service.h"

using android::base::GetProperty;
using android::init::property_set;

void property_override(const std::string& name, const std::string& value)
{
    size_t valuelen = value.size();
    prop_info* pi = (prop_info*) __system_property_find(name.c_str());
    if (pi != nullptr) {
        __system_property_update(pi, value.c_str(), valuelen);
    }
    else {
        int rc = __system_property_add(name.c_str(), name.size(), value.c_str(), valuelen);
        if (rc < 0) {
            LOG(ERROR) << "property_set(\"" << name << "\", \"" << value << "\") failed: "
                       << "__system_property_add failed";
        }
    }
}

void property_override_dual(char const system_prop[], char const vendor_prop[],
    char const value[])
{
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

void property_override_quad(const std::string& boot_prop, const std::string& product_prop, const std::string& system_prop, const std::string& vendor_prop, const std::string& value)
{
    property_override(boot_prop, value);
    property_override(product_prop, value);
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

void init_dsds() {
    property_set("ro.vendor.multisim.set_audio_params", "true");
    property_set("ro.vendor.multisim.simslotcount", "2");
    property_set("persist.radio.multisim.config", "dsds");
}

void vendor_load_properties()
{
    // Init a dummy BT MAC address, will be overwritten later
    property_set("ro.boot.btmacaddr", "00:00:00:00:00:00");

    std::string bootloader = GetProperty("ro.bootloader","");

    if (bootloader.find("A320FL") == 0) {
    /* SM-A320FL */
        property_override_quad("ro.product.model", "ro.product.odm.model", "ro.product.system.model", "ro.product.vendor.model", "SM-A320FL");
        property_override_quad("ro.product.name", "ro.product.odm.name", "ro.product.system.name", "ro.product.vendor.name", "a3y17ltexc");

    } else if (bootloader.find("A320FX") == 0) {
    /* SM-A320FX */
        property_override_quad("ro.product.model", "ro.product.odm.model", "ro.product.system.model", "ro.product.vendor.model", "SM-A320F");
        property_override_quad("ro.product.name", "ro.product.odm.name", "ro.product.system.name", "ro.product.vendor.name", "a3y17ltexx");

        init_dsds();

    } else if (bootloader.find("A320Y") == 0) {
    /* SM-A320Y */
        property_override_quad("ro.product.model", "ro.product.odm.model", "ro.product.system.model", "ro.product.vendor.model", "SM-A320Y");
        property_override_quad("ro.product.name", "ro.product.odm.name", "ro.product.system.name", "ro.product.vendor.name", "a3y17ltedx");

        init_dsds();
    }

    /* Common properties*/
    property_override_dual("ro.build.fingerprint", "ro.vendor.build.fingerprint", "google/redfin/redfin:11/RQ2A.210505.003/7255357:user/release-keys");
    property_override("ro.build.description", "samsung/a3y17ltexc/a3y17lte:8.0.0/R16NW/A320FLXXS5CSL5:user/release-keys");
    property_override_quad("ro.product.device", "ro.product.odm.device", "ro.product.system.device", "ro.product.vendor.device", "a3y17lte");

    std::string device = GetProperty("ro.product.device", "");
    LOG(ERROR) << "Found bootloader id %s setting build properties for %s device\n" << bootloader.c_str() << device.c_str();
}

