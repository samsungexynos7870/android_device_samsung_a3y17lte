/* SILAB 47xx RDS Parser
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

#ifndef __FM_RADIO_RDS_PARSER_H__
#define __FM_RADIO_RDS_PARSER_H__

#include <pthread.h>
#include <string.h>
#include "silab_fm.h"
#include "silab_ioctl.h"

class FmRadioRDSParser {
public:
    FmRadioRDSParser();
    virtual ~FmRadioRDSParser();

    void ResetData();
    int ParseData(unsigned short* rds_blocks, unsigned char* rds_bler = NULL);
    int ParseAFList(unsigned short* rds_blocks);

    int GetPI();
    int GetPIECCData(int* pi, int* ecc);
    int GetFinalRDSData(char* ps_buf, char* rt_buf);
    int GetFinalRTPlusData(unsigned char* rtp_buf);
    int GetAFList(char* af_buf);
    bool isAFAvailable();

    unsigned short GetPTY() const { return mPTY; }
    const char* GetPS() const { return mPS; }
    const char* GetRT() const { return mRT; }
    bool IsPSUpdated() const { return mPSUpdated; }
    bool IsRTUpdated() const { return mRTUpdated; }
    void ClearUpdatedFlags() { mPSUpdated = false; mRTUpdated = false; }

private:
    bool ValidateBufferedData(const char* s, int len);

    pthread_mutex_t mMutex;
    char mPS[16];
    char mPSBuffer[16];
    char mRT[72];
    char mRTBuffer[72];
    unsigned short mPI;
    unsigned char mECC;
    unsigned short mPTY;
    unsigned short mAFList[32];
    int mAFCount;
    bool mPSUpdated;
    bool mRTUpdated;
    bool mAFUpdated;
    unsigned char mRTPlus[16];
    int mRTPlusLen;
};

#endif // __FM_RADIO_RDS_PARSER_H__
