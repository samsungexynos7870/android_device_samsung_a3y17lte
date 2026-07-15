/* SILAB 47xx FmNative JNI Wrapper
 *
 * Copyright (C) 2019 Draekko, Ben Touchette
 * Copyright (C) 2026 Flominator
 * Copyright (C) 2014 The Android Open Source Project
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
 */

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fmr.h"
#include "silab_fm.h"
#include "silab_ioctl.h"
#include "FmRadioController_silab.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FmNative_silab_jni"

static FmRadioController_silab* gRadioController = NULL;

static inline int fmFloatToKHz(jfloat freq) {
    if (freq <= 0.0f) return 0;
    if (freq > 1000.0f) return (int)(freq * 10.0f + 0.5f);
    return (int)(freq * 1000.0f + 0.5f);
}

static inline jfloat fmKHzToFloat(int freq_khz) {
    return (jfloat)(freq_khz / 1000.0f);
}

jboolean openDev(JNIEnv *env, jobject thiz) {
    LOGI("%s entered\n", __func__);
    if (gRadioController == NULL) {
        gRadioController = new FmRadioController_silab();
    }
    if (gRadioController->Initialise() < 0) {
        LOGE("openDev: Initialise failed\n");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

jboolean closeDev(JNIEnv *env, jobject thiz) {
    LOGI("%s entered\n", __func__);
    if (gRadioController != NULL) {
        gRadioController->PowerOff();
        delete gRadioController;
        gRadioController = NULL;
    }
    return JNI_TRUE;
}

jboolean powerUp(JNIEnv *env, jobject thiz, jfloat freq) {
    LOGI("%s entered (freq=%0.2f)\n", __func__, freq);
    if (gRadioController == NULL) {
        gRadioController = new FmRadioController_silab();
        gRadioController->Initialise();
    }
    if (gRadioController->PowerOn() < 0) {
        LOGE("powerUp: PowerOn failed\n");
        return JNI_FALSE;
    }
    int freq_khz = fmFloatToKHz(freq);
    if (gRadioController->TuneChannel(freq_khz) < 0) {
        LOGE("powerUp: TuneChannel failed\n");
        return JNI_FALSE;
    }
    gRadioController->EnableRDS();
    return JNI_TRUE;
}

jboolean powerDown(JNIEnv *env, jobject thiz, jint type) {
    LOGI("%s entered (type=%d)\n", __func__, type);
    if (gRadioController != NULL) {
        gRadioController->DisableRDS();
        gRadioController->PowerOff();
    }
    return JNI_TRUE;
}

jboolean tune(JNIEnv *env, jobject thiz, jfloat freq) {
    LOGI("%s entered (freq=%0.2f)\n", __func__, freq);
    if (gRadioController == NULL) return JNI_FALSE;
    int freq_khz = fmFloatToKHz(freq);
    if (gRadioController->TuneChannel(freq_khz) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

jfloat seek(JNIEnv *env, jobject thiz, jfloat freq, jboolean isUp) {
    LOGI("%s entered (freq=%0.2f, isUp=%d)\n", __func__, freq, isUp);
    if (gRadioController == NULL) return 0.0f;
    int ret_khz = isUp ? gRadioController->SeekUp() : gRadioController->SeekDown();
    if (ret_khz > 0) {
        return fmKHzToFloat(ret_khz);
    }
    return 0.0f;
}

jshortArray autoScan(JNIEnv *env, jobject thiz) {
    LOGI("%s entered\n", __func__);
    if (gRadioController == NULL) return NULL;

    gRadioController->TuneChannel(87500);
    jshort scanlist[MAX_FM_SCAN_CH_SIZE];
    int count = 0;

    gRadioController->setScanning(1);
    for (int i = 0; i < MAX_FM_SCAN_CH_SIZE; i++) {
        int ret_khz = gRadioController->SeekUp();
        if (ret_khz <= 0 || ret_khz >= 108000) {
            break;
        }
        scanlist[count++] = (jshort)(ret_khz / 10);
    }
    gRadioController->setScanning(0);

    jshortArray scanlistings = env->NewShortArray(count);
    env->SetShortArrayRegion(scanlistings, 0, count, (const jshort*)scanlist);
    return scanlistings;
}

jboolean stopScan(JNIEnv *env, jobject thiz) {
    LOGI("%s entered\n", __func__);
    if (gRadioController == NULL) return JNI_FALSE;
    if (gRadioController->SeekCancel() < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

jint setRds(JNIEnv *env, jobject thiz, jboolean rdson) {
    LOGI("%s entered (rdson=%d)\n", __func__, rdson);
    if (gRadioController == NULL) return JNI_FALSE;
    if (rdson) {
        gRadioController->EnableRDS();
    } else {
        gRadioController->DisableRDS();
    }
    return JNI_TRUE;
}

jshort readRds(JNIEnv *env, jobject thiz) {
    if (gRadioController == NULL) return 0;
    jshort status = 0;
    if (gRadioController->mRDSParser.IsPSUpdated()) {
        status |= RDS_EVENT_PROGRAMNAME;
    }
    if (gRadioController->mRDSParser.IsRTUpdated()) {
        status |= (RDS_EVENT_LAST_RADIOTEXT | RDS_EVENT_PTY);
    }
    if (gRadioController->mRDSParser.isAFAvailable()) {
        status |= RDS_EVENT_AF;
    }
    if (status != 0) {
        gRadioController->mRDSParser.ClearUpdatedFlags();
    }
    return status;
}

jbyteArray getPs(JNIEnv *env, jobject thiz) {
    jbyteArray radiotext9 = env->NewByteArray(9);
    char ps_buf[16] = {0};
    if (gRadioController != NULL) {
        memcpy(ps_buf, gRadioController->mRDSParser.GetPS(), 8);
    }
    env->SetByteArrayRegion(radiotext9, 0, 9, (const jbyte*)ps_buf);
    return radiotext9;
}

jbyteArray getLrText(JNIEnv *env, jobject thiz) {
    jbyteArray radiotext65 = env->NewByteArray(65);
    char rt_buf[72] = {0};
    if (gRadioController != NULL) {
        memcpy(rt_buf, gRadioController->mRDSParser.GetRT(), 64);
    }
    env->SetByteArrayRegion(radiotext65, 0, 65, (const jbyte*)rt_buf);
    return radiotext65;
}

jshort activeAf(JNIEnv *env, jobject thiz) {
    if (gRadioController != NULL) {
        char af_buf[64] = {0};
        int count = gRadioController->mRDSParser.GetAFList(af_buf);
        if (count > 0) {
            unsigned short* af_list = (unsigned short*)af_buf;
            LOGI("activeAf: returning %d (%0.2f MHz)\n", af_list[0], (float)af_list[0] / 100.0f);
            return (jshort)af_list[0];
        }
    }
    return -1;
}

jint setMute(JNIEnv *env, jobject thiz, jboolean mute) {
    LOGI("%s entered (mute=%d)\n", __func__, mute);
    if (gRadioController == NULL) return JNI_FALSE;
    if (mute) {
        gRadioController->MuteOn();
    } else {
        gRadioController->MuteOff();
    }
    return JNI_TRUE;
}

jint isRdsSupport(JNIEnv *env, jobject thiz) {
    return 1;
}

jint switchAntenna(JNIEnv *env, jobject thiz, jint antenna) {
    LOGI("%s entered (antenna=%d not supported on silab wrapper)\n", __func__, antenna);
    return 2;
}

static const char *classPathNameRx = "com/android/fmradio/FmNative";

static JNINativeMethod methodsRx[] = {
    {(char*)"openDev",       (char*)"()Z",   (void*)openDev       }, // 1
    {(char*)"closeDev",      (char*)"()Z",   (void*)closeDev      }, // 2
    {(char*)"powerUp",       (char*)"(F)Z",  (void*)powerUp       }, // 3
    {(char*)"powerDown",     (char*)"(I)Z",  (void*)powerDown     }, // 4
    {(char*)"tune",          (char*)"(F)Z",  (void*)tune          }, // 5
    {(char*)"seek",          (char*)"(FZ)F", (void*)seek          }, // 6
    {(char*)"autoScan",      (char*)"()[S",  (void*)autoScan      }, // 7
    {(char*)"stopScan",      (char*)"()Z",   (void*)stopScan      }, // 8
    {(char*)"setRds",        (char*)"(Z)I",  (void*)setRds        }, // 9
    {(char*)"readRds",       (char*)"()S",   (void*)readRds       }, // 10
    {(char*)"getPs",         (char*)"()[B",  (void*)getPs         }, // 11
    {(char*)"getLrText",     (char*)"()[B",  (void*)getLrText     }, // 12
    {(char*)"activeAf",      (char*)"()S",   (void*)activeAf      }, // 13
    {(char*)"setMute",	     (char*)"(Z)I",  (void*)setMute       }, // 14
    {(char*)"isRdsSupport",  (char*)"()I",   (void*)isRdsSupport  }, // 15
    {(char*)"switchAntenna", (char*)"(I)I",  (void*)switchAntenna }, // 16
};

static jint registerNativeMethods(JNIEnv* env, const char* className,
                                  JNINativeMethod* gMethods, int numMethods) {
    jclass clazz = env->FindClass(className);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }
    LOGD("%s success for '%s'\n", __func__, className);
    return JNI_TRUE;
}

typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;

    LOGI("JNI_OnLoad entered\n");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        return result;
    }
    env = uenv.env;

    if (registerNativeMethods(env, classPathNameRx, methodsRx,
                              sizeof(methodsRx) / sizeof(methodsRx[0])) != JNI_TRUE) {
        LOGE("ERROR: registerNativeMethods failed\n");
        return result;
    }

    result = JNI_VERSION_1_4;
    return result;
}
