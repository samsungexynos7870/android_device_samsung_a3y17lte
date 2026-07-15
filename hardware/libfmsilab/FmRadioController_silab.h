/* SILAB 47xx Object-Oriented Radio Controller
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

#ifndef __FM_RADIO_CONTROLLER_SILAB_H__
#define __FM_RADIO_CONTROLLER_SILAB_H__

#include <pthread.h>
#include <unistd.h>
#include "silab_fm.h"
#include "silab_ioctl.h"
#include "FmRadioRDSParser.h"

class FmRadioController_silab {
public:
    FmRadioController_silab();
    virtual ~FmRadioController_silab();

    int Initialise();
    int PowerOn();
    int PowerOff();
    int TuneChannel(int freq_khz);
    int GetChannel();
    int checkBaseFreq();

    int SeekUp();
    int SeekDown();
    int SearchUp();
    int SearchDown();
    int SearchAll();
    int SeekCancel();

    int SetVolume(int vol);
    int GetVolume();
    int GetMaxVolume();
    int SetSpeakerOn(bool on);
    int SetRecordMode(int mode);

    int SetBand(int band);
    int SetChannelSpacing(int spacing);
    int SetStereo();
    int SetMono();

    int MuteOn();
    int MuteOff();
    int DS_MuteOn();
    int DS_MuteOff();
    int Int_MuteOn();
    bool IsMuteOn();

    int setSoftmute(int mode);
    int SetDeConstant(int de);
    int GetCurrentRSSI();
    int GetCurrentSNR();

    int EnableRDS();
    int DisableRDS();
    int EnableDNS();
    int DisableDNS();
    int EnableAF();
    int DisableAF();

    int StartRDSThread();
    int StopRDSThread();
    int HandleRDSData();

    int StartAfSwitchingProcess(int target_freq);
    int HandleAfSwitchingProcess();
    int CancelAfSwitchingProcess();
    static int SortBasedOnSignalStrength(int* freqs, int count);

    int SetAF_th(int th);
    int GetAF_th();
    int SetAFValid_th(int th);
    int GetAFValid_th();
    int SetSeekRSSI(int th);
    int SetSeekSNR(int th);
    int SetRSSI_th(int th);
    int GetRSSI_th();
    int SetSNR_th(int th);
    int GetSNR_th();
    int SetCnt_th(char th);
    int GetCnt_th();
    int setScanning(int scanning);

    FmRadioRDSParser mRDSParser;

private:
    static void* _ThreadHandleRDSData(void* arg);
    static void* _ThreadHandleAfSwitching(void* arg);

    int mFdRadio;
    int mFdCancel;
    int mFdSearch;

    bool mIsPoweredOn;
    bool mIsMuteOn;
    bool mIsDsMuteOn;
    bool mIsScanning;

    bool mRDSEnabled;
    bool mDNSEnabled;
    bool mAFEnabled;
    bool mAFSwitchingActive;

    pthread_t mRDSThread;
    pthread_t mAFThread;
    bool mStopRDSThread;

    int mCurrentBand;
    int mCurrentSpacing;
    int mCurrentDeConstant;
    int mCurrentVolume;
    int mCurrentFreqKHz;

    int mRssiThreshold;
    int mSnrThreshold;
    int mCntThreshold;
    int mAfThreshold;
    int mAfValidThreshold;
};

#endif // __FM_RADIO_CONTROLLER_SILAB_H__
