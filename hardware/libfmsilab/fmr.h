
/*
 * Copyright (C) 2019 Draekko, Ben Touchette
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

#ifndef __FMR_H__
#define __FMR_H__

#include <jni.h>
#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <dlfcn.h>

#undef FM_LIB_USE_XLOG

#ifdef FM_LIB_USE_XLOG
#include <cutils/xlog.h>
#undef LOGV
#define LOGV(...) XLOGV(__VA_ARGS__)
#undef LOGD
#define LOGD(...) XLOGD(__VA_ARGS__)
#undef LOGI
#define LOGI(...) XLOGI(__VA_ARGS__)
#undef LOGW
#define LOGW(...) XLOGW(__VA_ARGS__)
#undef LOGE
#define LOGE(...) XLOGE(__VA_ARGS__)
#else
#undef LOGV
#define LOGV(...) ALOGV(__VA_ARGS__)
#undef LOGD
#define LOGD(...) ALOGD(__VA_ARGS__)
#undef LOGI
#define LOGI(...) ALOGI(__VA_ARGS__)
#undef LOGW
#define LOGW(...) ALOGW(__VA_ARGS__)
#undef LOGE
#define LOGE(...) ALOGE(__VA_ARGS__)
#endif

/****************** Function declaration ******************/
//fmr_err.cpp
char *FMR_strerr();
void FMR_seterr(int err);


#define FMR_ASSERT(a) { \
    if ((a) == NULL) { \
        LOGE("%s,invalid buf\n", __func__);\
        return -ERR_INVALID_BUF; \
    } \
}
#endif

