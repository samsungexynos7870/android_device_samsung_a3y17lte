/*
 * Copyright (C) 2026 The LineageOS Project
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
#include <log/log.h>

/*
 * Shim for the JNI helpers the ANT radio JNI library (libantradio.so)
 * was built against on stock Oreo. Newer libnativehelper builds no
 * longer export them, so the library is patched to also link this
 * shim during extraction.
 */

int jniThrowException(JNIEnv* env, const char* className, const char* msg)
{
    jclass exceptionClass;

    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }

    exceptionClass = (*env)->FindClass(env, className);
    if (exceptionClass == NULL) {
        ALOGE("Unable to find exception class %s", className);
        /* ClassNotFoundException; fall back to RuntimeException */
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionClear(env);
        }
        exceptionClass = (*env)->FindClass(env, "java/lang/RuntimeException");
        if (exceptionClass == NULL) {
            ALOGE("Falling back to RuntimeException failed");
            return -1;
        }
    }

    if ((*env)->ThrowNew(env, exceptionClass, msg) != JNI_OK) {
        ALOGE("Failed throwing '%s'", className);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    return 0;
}
