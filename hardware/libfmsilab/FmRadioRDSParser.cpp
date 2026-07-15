/* SILAB 47xx RDS Parser Implementation
 *
 * Copyright (C) 2019 Draekko, Ben Touchette
 * Copyright (C) 2026 Flominator
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

#include "FmRadioRDSParser.h"
#include "fmr.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FmRadioRDSParser"

FmRadioRDSParser::FmRadioRDSParser() {
    pthread_mutex_init(&mMutex, NULL);
    ResetData();
    LOGI("FmRadioRDSParser constructor initialized\n");
}

FmRadioRDSParser::~FmRadioRDSParser() {
    pthread_mutex_destroy(&mMutex);
    LOGI("FmRadioRDSParser destructor called\n");
}

void FmRadioRDSParser::ResetData() {
    pthread_mutex_lock(&mMutex);
    memset(mPS, 0, sizeof(mPS));
    memset(mPSBuffer, ' ', 8);
    mPSBuffer[8] = '\0';
    memset(mRT, 0, sizeof(mRT));
    memset(mRTBuffer, ' ', 64);
    mRTBuffer[64] = '\0';
    mPI = 0;
    mECC = 0;
    mPTY = 0;
    memset(mAFList, 0, sizeof(mAFList));
    mAFCount = 0;
    mPSUpdated = false;
    mRTUpdated = false;
    mAFUpdated = false;
    memset(mRTPlus, 0, sizeof(mRTPlus));
    mRTPlusLen = 0;
    pthread_mutex_unlock(&mMutex);
}

bool FmRadioRDSParser::ValidateBufferedData(const char* s, int len) {
    if (s == NULL || len <= 0) return false;
    for (int i = 0; i < len; i++) {
        if (s[i] != ' ' && s[i] != '\0') {
            return true;
        }
    }
    return false;
}

int FmRadioRDSParser::ParseAFList(unsigned short* rds_blocks) {
    if (rds_blocks == NULL) return 0;
    unsigned short rdsc = rds_blocks[2];
    
    /* In Group 0A, Block C contains two 8-bit Alternative Frequency codes or number of AFs */
    u8 af1 = (u8)(rdsc >> 8);
    u8 af2 = (u8)(rdsc & 0xFF);

    pthread_mutex_lock(&mMutex);
    /* VHF Band AF codes: 1..204 represent 87.6..107.9 MHz in 100 kHz steps */
    if (af1 >= 1 && af1 <= 204 && mAFCount < 32) {
        unsigned short freq_khz = (unsigned short)(8750 + af1 * 10);
        bool exists = false;
        for (int i = 0; i < mAFCount; i++) {
            if (mAFList[i] == freq_khz) { exists = true; break; }
        }
        if (!exists) {
            mAFList[mAFCount++] = freq_khz;
            mAFUpdated = true;
        }
    }
    if (af2 >= 1 && af2 <= 204 && mAFCount < 32) {
        unsigned short freq_khz = (unsigned short)(8750 + af2 * 10);
        bool exists = false;
        for (int i = 0; i < mAFCount; i++) {
            if (mAFList[i] == freq_khz) { exists = true; break; }
        }
        if (!exists) {
            mAFList[mAFCount++] = freq_khz;
            mAFUpdated = true;
        }
    }
    pthread_mutex_unlock(&mMutex);
    return mAFUpdated ? 1 : 0;
}

int FmRadioRDSParser::ParseData(unsigned short* rds_blocks, unsigned char* rds_bler) {
    if (rds_blocks == NULL) return 0;

    pthread_mutex_lock(&mMutex);
    unsigned short rdsa = rds_blocks[0];
    unsigned short rdsb = rds_blocks[1];
    unsigned short rdsc = rds_blocks[2];
    unsigned short rdsd = rds_blocks[3];

    /* Check block error rates (0 = no error, 1..2 = corrected, 3 = uncorrectable) */
    if (rds_bler != NULL) {
        if (rds_bler[1] >= 3 || rds_bler[3] >= 3) {
            pthread_mutex_unlock(&mMutex);
            return 0;
        }
    }

    if (rdsa != 0) {
        mPI = rdsa;
    }

    int grpcode = rdsb >> 11;
    mPTY = (rdsb >> 5) & 0x1F;

    int event_status = 0;

    if (grpcode == GROUP_TYPE_0A || grpcode == GROUP_TYPE_0B) {
        int pos = rdsb & 0x03;
        u8 c1 = (u8)(rdsd >> 8);
        u8 c2 = (u8)(rdsd & 0xFF);
        if (c1 < 32 || c1 > 126) c1 = ' ';
        if (c2 < 32 || c2 > 126) c2 = ' ';

        mPSBuffer[pos * 2]     = (char)c1;
        mPSBuffer[pos * 2 + 1] = (char)c2;

        if (pos == 3) {
            if (ValidateBufferedData(mPSBuffer, 8)) {
                memcpy(mPS, mPSBuffer, 8);
                mPS[8] = '\0';
                mPSUpdated = true;
                event_status |= RDS_EVENT_PROGRAMNAME;
            }
        }

        if (grpcode == GROUP_TYPE_0A) {
            pthread_mutex_unlock(&mMutex);
            if (ParseAFList(rds_blocks) > 0) {
                event_status |= RDS_EVENT_AF;
            }
            pthread_mutex_lock(&mMutex);
        }
    } else if (grpcode == GROUP_TYPE_2A || grpcode == GROUP_TYPE_2B) {
        int pos = rdsb & 0x0F;
        if (grpcode == GROUP_TYPE_2A) {
            u8 c1 = (u8)(rdsc >> 8);
            u8 c2 = (u8)(rdsc & 0xFF);
            u8 c3 = (u8)(rdsd >> 8);
            u8 c4 = (u8)(rdsd & 0xFF);
            if (c1 < 32 || c1 > 126) c1 = ' ';
            if (c2 < 32 || c2 > 126) c2 = ' ';
            if (c3 < 32 || c3 > 126) c3 = ' ';
            if (c4 < 32 || c4 > 126) c4 = ' ';

            if (pos * 4 + 3 < 64) {
                mRTBuffer[pos * 4 + 0] = (char)c1;
                mRTBuffer[pos * 4 + 1] = (char)c2;
                mRTBuffer[pos * 4 + 2] = (char)c3;
                mRTBuffer[pos * 4 + 3] = (char)c4;
            }

            if (pos == 15 || c4 == '\r' || c4 == '\n') {
                if (ValidateBufferedData(mRTBuffer, 64)) {
                    memcpy(mRT, mRTBuffer, 64);
                    mRT[64] = '\0';
                    mRTUpdated = true;
                    event_status |= (RDS_EVENT_LAST_RADIOTEXT | RDS_EVENT_PTY);
                }
            }
        }
    }

    pthread_mutex_unlock(&mMutex);
    return event_status;
}

int FmRadioRDSParser::GetPI() {
    pthread_mutex_lock(&mMutex);
    int pi = (int)mPI;
    pthread_mutex_unlock(&mMutex);
    return pi;
}

int FmRadioRDSParser::GetPIECCData(int* pi, int* ecc) {
    pthread_mutex_lock(&mMutex);
    if (pi != NULL) *pi = (int)mPI;
    if (ecc != NULL) *ecc = (int)mECC;
    pthread_mutex_unlock(&mMutex);
    return 0;
}

int FmRadioRDSParser::GetFinalRDSData(char* ps_buf, char* rt_buf) {
    if (ps_buf == NULL || rt_buf == NULL) return -1;
    pthread_mutex_lock(&mMutex);
    memcpy(ps_buf, mPS, sizeof(mPS));
    memcpy(rt_buf, mRT, sizeof(mRT));
    pthread_mutex_unlock(&mMutex);
    return 0;
}

int FmRadioRDSParser::GetFinalRTPlusData(unsigned char* rtp_buf) {
    if (rtp_buf == NULL) return -1;
    pthread_mutex_lock(&mMutex);
    memcpy(rtp_buf, mRTPlus, sizeof(mRTPlus));
    int len = mRTPlusLen;
    pthread_mutex_unlock(&mMutex);
    return len;
}

int FmRadioRDSParser::GetAFList(char* af_buf) {
    if (af_buf == NULL) return -1;
    pthread_mutex_lock(&mMutex);
    int count = mAFCount;
    memcpy(af_buf, mAFList, sizeof(mAFList));
    pthread_mutex_unlock(&mMutex);
    return count;
}

bool FmRadioRDSParser::isAFAvailable() {
    pthread_mutex_lock(&mMutex);
    bool available = (mAFCount > 0);
    pthread_mutex_unlock(&mMutex);
    return available;
}
