/* SILAB 47xx Object-Oriented Radio Controller Implementation
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

#include "FmRadioController_silab.h"
#include "fmr.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FmRadioController_silab"

FmRadioController_silab::FmRadioController_silab()
    : mFdRadio(-1),
      mFdCancel(-1),
      mFdSearch(-1),
      mIsPoweredOn(false),
      mIsMuteOn(true),
      mIsDsMuteOn(false),
      mIsScanning(false),
      mRDSEnabled(false),
      mDNSEnabled(false),
      mAFEnabled(false),
      mAFSwitchingActive(false),
      mRDSThread(0),
      mAFThread(0),
      mStopRDSThread(false),
      mCurrentBand(BAND_87500_108000_kHz),
      mCurrentSpacing(CHAN_SPACING_100_kHz),
      mCurrentDeConstant(DE_TIME_CONSTANT_50),
      mCurrentVolume(15),
      mCurrentFreqKHz(87500),
      mRssiThreshold(0x01),
      mSnrThreshold(0x02),
      mCntThreshold(0x00),
      mAfThreshold(0x04),
      mAfValidThreshold(0x02)
{
    LOGI("FmRadioController_silab constructed (Timestamp: 2026-07-12 Flominator build)\n");
}

FmRadioController_silab::~FmRadioController_silab() {
    LOGI("FmRadioController_silab destructor called\n");
    StopRDSThread();
    CancelAfSwitchingProcess();
    PowerOff();
    if (mFdCancel >= 0) close(mFdCancel);
    if (mFdSearch >= 0) close(mFdSearch);
    if (mFdRadio >= 0)  close(mFdRadio);
    mFdRadio = -1;
    mFdCancel = -1;
    mFdSearch = -1;
}

int FmRadioController_silab::Initialise() {
    if (mFdRadio < 0) {
        mFdRadio  = open("/dev/radio0", O_RDWR);
        mFdCancel = open("/dev/radio0", O_RDWR);
        mFdSearch = open("/dev/radio0", O_RDWR);
    }
    LOGI("Initialise: fd_radio=%d, fd_cancel=%d, fd_search=%d\n", mFdRadio, mFdCancel, mFdSearch);
    if (mFdRadio < 0) {
        LOGE("Unable to open /dev/radio0: %s\n", strerror(errno));
        return -1;
    }
    setFd(mFdRadio);
    return 0;
}

int FmRadioController_silab::PowerOn() {
    LOGI("PowerOn entered\n");
    if (mFdRadio < 0 && Initialise() < 0) {
        return -1;
    }

    if (powerup() != SILAB_TRUE) {
        LOGE("powerup() failed\n");
        return -1;
    }

    Int_MuteOn();

    struct sys_config2 c2;
    c2.rssi_th = (u16)mRssiThreshold;
    c2.fm_band = (u8)mCurrentBand;
    c2.fm_chan_spac = (u8)mCurrentSpacing;
    c2.fm_vol = (u8)mCurrentVolume;
    setsysconfig2(&c2);

    struct sys_config3 c3;
    c3.smmute = 0;
    c3.smutea = 0;
    c3.volext = 0;
    c3.sksnr = (u8)mSnrThreshold;
    c3.skcnt = (u8)mCntThreshold;
    setsysconfig3(&c3);

    setrssi_th((u8)mRssiThreshold);
    setsnr_th((u8)mSnrThreshold);
    setcnt_th((u8)mCntThreshold);
    setband(mCurrentBand);
    setchannelspacing(mCurrentSpacing);
    setdeconstant((u8)mCurrentDeConstant);
    setstereo();
    setvolume((u8)mCurrentVolume);

    mIsPoweredOn = true;
    LOGI("PowerOn successful\n");
    return 0;
}

int FmRadioController_silab::PowerOff() {
    LOGI("PowerOff entered\n");
    if (!mIsPoweredOn || mFdRadio < 0) {
        return 0;
    }
    StopRDSThread();
    Int_MuteOn();
    if (powerdown() != SILAB_TRUE) {
        LOGE("powerdown() failed\n");
    }
    mIsPoweredOn = false;
    return 0;
}

int FmRadioController_silab::TuneChannel(int freq_khz) {
    LOGI("TuneChannel(%d kHz)\n", freq_khz);
    if (mFdRadio < 0) return -1;

    u32 frq;
    if (freq_khz > 50000) {
        frq = (u32)(freq_khz / 10);
    } else {
        frq = (u32)freq_khz;
    }

    DS_MuteOn();
    Int_MuteOn();

    bool wasRdsEnabled = mRDSEnabled;
    if (wasRdsEnabled) {
        disablerds();
    }
    resetrds();
    mRDSParser.ResetData();

    if (setfreq(frq) != SILAB_TRUE) {
        LOGE("setfreq(%u) failed\n", frq);
        return -1;
    }

    mCurrentFreqKHz = frq * 10;
    if (wasRdsEnabled) {
        enablerds();
    }
    DS_MuteOff();
    if (!mIsMuteOn) {
        MuteOff();
    }
    return 0;
}

int FmRadioController_silab::GetChannel() {
    if (mFdRadio < 0) return -1;
    u32 freq = 0;
    if (getfreq(&freq) == SILAB_TRUE) {
        mCurrentFreqKHz = freq * 10;
        return mCurrentFreqKHz;
    }
    return -1;
}

int FmRadioController_silab::checkBaseFreq() {
    int channel = GetChannel();
    LOGI("checkBaseFreq: current channel = %d kHz\n", channel);
    return channel;
}

int FmRadioController_silab::SeekUp() {
    LOGI("SeekUp entered\n");
    if (mFdRadio < 0) return -1;
    DS_MuteOn();
    Int_MuteOn();

    bool wasRdsEnabled = mRDSEnabled;
    if (wasRdsEnabled) {
        disablerds();
    }
    resetrds();
    mRDSParser.ResetData();

    u32 frq = 0;
    int ret = -1;
    if (seekup(&frq) == SILAB_TRUE) {
        mCurrentFreqKHz = frq * 10;
        ret = mCurrentFreqKHz;
    } else {
        LOGE("seekup failed\n");
    }

    if (wasRdsEnabled) {
        enablerds();
    }
    DS_MuteOff();
    if (!mIsMuteOn) MuteOff();
    return ret;
}

int FmRadioController_silab::SeekDown() {
    LOGI("SeekDown entered\n");
    if (mFdRadio < 0) return -1;
    DS_MuteOn();
    Int_MuteOn();

    bool wasRdsEnabled = mRDSEnabled;
    if (wasRdsEnabled) {
        disablerds();
    }
    resetrds();
    mRDSParser.ResetData();

    u32 frq = 0;
    int ret = -1;
    if (seekdown(&frq) == SILAB_TRUE) {
        mCurrentFreqKHz = frq * 10;
        ret = mCurrentFreqKHz;
    } else {
        LOGE("seekdown failed\n");
    }

    if (wasRdsEnabled) {
        enablerds();
    }
    DS_MuteOff();
    if (!mIsMuteOn) MuteOff();
    return ret;
}

int FmRadioController_silab::SearchUp() {
    return SeekUp();
}

int FmRadioController_silab::SearchDown() {
    return SeekDown();
}

int FmRadioController_silab::SearchAll() {
    LOGI("SearchAll entered\n");
    if (mFdRadio < 0) return -1;
    mIsScanning = true;
    u32 bottom = BOTTOM_FREQ_8750;
    setfreq(bottom);
    mRDSParser.ResetData();
    return 0;
}

int FmRadioController_silab::SeekCancel() {
    LOGI("SeekCancel entered\n");
    if (mFdRadio < 0) return -1;
    mIsScanning = false;
    if (seekstop() == SILAB_TRUE) {
        return 0;
    }
    return -1;
}

int FmRadioController_silab::SetVolume(int vol) {
    LOGI("SetVolume(%d)\n", vol);
    if (vol < 0) vol = 0;
    if (vol > 15) vol = 15;
    mCurrentVolume = vol;
    if (mFdRadio < 0) return 0;
    if (setvolume((u8)vol) == SILAB_TRUE) {
        return 0;
    }
    return -1;
}

int FmRadioController_silab::GetVolume() {
    return mCurrentVolume;
}

int FmRadioController_silab::GetMaxVolume() {
    return 15;
}

int FmRadioController_silab::SetSpeakerOn(bool on) {
    LOGI("SetSpeakerOn(%d)\n", on);
    return 0;
}

int FmRadioController_silab::SetRecordMode(int mode) {
    LOGI("SetRecordMode(%d)\n", mode);
    return 0;
}

int FmRadioController_silab::SetBand(int band) {
    LOGI("SetBand(%d)\n", band);
    mCurrentBand = band;
    if (mFdRadio >= 0) {
        setband(band);
    }
    return 0;
}

int FmRadioController_silab::SetChannelSpacing(int spacing) {
    LOGI("SetChannelSpacing(%d)\n", spacing);
    mCurrentSpacing = spacing;
    if (mFdRadio >= 0) {
        setchannelspacing(spacing);
    }
    return 0;
}

int FmRadioController_silab::SetStereo() {
    LOGI("SetStereo()\n");
    if (mFdRadio >= 0) setstereo();
    return 0;
}

int FmRadioController_silab::SetMono() {
    LOGI("SetMono()\n");
    if (mFdRadio >= 0) setmono();
    return 0;
}

int FmRadioController_silab::MuteOn() {
    LOGI("MuteOn()\n");
    mIsMuteOn = true;
    return Int_MuteOn();
}

int FmRadioController_silab::MuteOff() {
    LOGI("MuteOff()\n");
    mIsMuteOn = false;
    if (mFdRadio < 0) return -1;
    if (setmuteoff() == SILAB_TRUE) return 0;
    return -1;
}

int FmRadioController_silab::DS_MuteOn() {
    LOGI("DS_MuteOn()\n");
    mIsDsMuteOn = true;
    if (mFdRadio < 0) return -1;
    if (setdsmuteon() == SILAB_TRUE) return 0;
    return -1;
}

int FmRadioController_silab::DS_MuteOff() {
    LOGI("DS_MuteOff()\n");
    mIsDsMuteOn = false;
    if (mFdRadio < 0) return -1;
    if (setdsmuteoff() == SILAB_TRUE) return 0;
    return -1;
}

int FmRadioController_silab::Int_MuteOn() {
    LOGI("Int_MuteOn()\n");
    if (mFdRadio < 0) return -1;
    if (setmuteon() == SILAB_TRUE) return 0;
    return -1;
}

bool FmRadioController_silab::IsMuteOn() {
    return mIsMuteOn;
}

int FmRadioController_silab::setSoftmute(int mode) {
    LOGI("setSoftmute(%d)\n", mode);
    return 0;
}

int FmRadioController_silab::SetDeConstant(int de) {
    LOGI("SetDeConstant(%d)\n", de);
    mCurrentDeConstant = de;
    if (mFdRadio >= 0) setdeconstant((u8)de);
    return 0;
}

int FmRadioController_silab::GetCurrentRSSI() {
    if (mFdRadio < 0) return 0;
    struct radio_data_t rds;
    if (getrdsdata(&rds) == SILAB_TRUE) {
        return (int)rds.curr_rssi;
    }
    return 0;
}

int FmRadioController_silab::GetCurrentSNR() {
    return 0;
}

int FmRadioController_silab::EnableRDS() {
    LOGI("EnableRDS()\n");
    if (mFdRadio < 0) return -1;
    if (enablerds() == SILAB_TRUE) {
        mRDSEnabled = true;
        StartRDSThread();
        return 0;
    }
    return -1;
}

int FmRadioController_silab::DisableRDS() {
    LOGI("DisableRDS()\n");
    mRDSEnabled = false;
    StopRDSThread();
    if (mFdRadio >= 0) disablerds();
    return 0;
}

int FmRadioController_silab::EnableDNS() {
    LOGI("EnableDNS()\n");
    mDNSEnabled = true;
    StartRDSThread();
    return 0;
}

int FmRadioController_silab::DisableDNS() {
    LOGI("DisableDNS()\n");
    mDNSEnabled = false;
    if (!mRDSEnabled && !mAFEnabled) StopRDSThread();
    return 0;
}

int FmRadioController_silab::EnableAF() {
    LOGI("EnableAF()\n");
    mAFEnabled = true;
    StartRDSThread();
    return 0;
}

int FmRadioController_silab::DisableAF() {
    LOGI("DisableAF()\n");
    mAFEnabled = false;
    CancelAfSwitchingProcess();
    if (!mRDSEnabled && !mDNSEnabled) StopRDSThread();
    return 0;
}

void* FmRadioController_silab::_ThreadHandleRDSData(void* arg) {
    FmRadioController_silab* controller = (FmRadioController_silab*)arg;
    if (controller) {
        controller->HandleRDSData();
    }
    return NULL;
}

int FmRadioController_silab::StartRDSThread() {
    if (mRDSThread != 0) {
        return 0;
    }
    mStopRDSThread = false;
    int ret = pthread_create(&mRDSThread, NULL, _ThreadHandleRDSData, this);
    LOGI("StartRDSThread: pthread_create result=%d\n", ret);
    if (ret != 0) {
        mRDSThread = 0;
        return -1;
    }
    return 0;
}

int FmRadioController_silab::StopRDSThread() {
    if (mRDSThread == 0) {
        return 0;
    }
    LOGI("StopRDSThread stopping thread\n");
    mStopRDSThread = true;
    pthread_join(mRDSThread, NULL);
    mRDSThread = 0;
    return 0;
}

int FmRadioController_silab::HandleRDSData() {
    LOGI("HandleRDSData thread started\n");
    struct radio_data_t rds;
    int idle_count = 0;
    while (!mStopRDSThread && (mRDSEnabled || mDNSEnabled || mAFEnabled)) {
        if (mFdRadio < 0) break;
        int count = 0;
        while (!mStopRDSThread && getrdsdata(&rds) == SILAB_TRUE && count < 64) {
            unsigned short rds_blocks[4] = { rds.rdsa, rds.rdsb, rds.rdsc, rds.rdsd };
            unsigned char rds_bler[4] = { rds.blera, rds.blerb, rds.blerc, rds.blerd };
            mRDSParser.ParseData(rds_blocks, rds_bler);
            count++;
        }

        if (count > 0) {
            idle_count = 0;
        } else {
            idle_count++;
        }

        /* Adaptive sleep to prevent kernel log spam:
         * - When actively receiving groups (or just turned on), poll every 100ms (~1.1 groups/sec).
         * - When FIFO has been empty for > 2 iterations, back off to 300ms intervals.
         * We sleep in 50ms steps so StopRDSThread() can still shut down rapidly (< 50ms latency).
         */
        int sleep_us = (idle_count > 2) ? 300000 : 100000;
        for (int s = 0; s < sleep_us && !mStopRDSThread; s += 50000) {
            usleep(50000);
        }
    }
    LOGI("HandleRDSData thread exited\n");
    return 0;
}

void* FmRadioController_silab::_ThreadHandleAfSwitching(void* arg) {
    FmRadioController_silab* controller = (FmRadioController_silab*)arg;
    if (controller) {
        controller->HandleAfSwitchingProcess();
    }
    return NULL;
}

int FmRadioController_silab::StartAfSwitchingProcess(int target_freq) {
    LOGI("StartAfSwitchingProcess(%d)\n", target_freq);
    if (mAFSwitchingActive) return 0;
    mAFSwitchingActive = true;
    pthread_create(&mAFThread, NULL, _ThreadHandleAfSwitching, this);
    return 0;
}

int FmRadioController_silab::HandleAfSwitchingProcess() {
    LOGI("HandleAfSwitchingProcess running\n");
    char af_buf[64];
    int count = mRDSParser.GetAFList(af_buf);
    if (count > 0) {
        unsigned short* af_list = (unsigned short*)af_buf;
        int best_rssi = -1;
        int best_freq = -1;
        for (int i = 0; i < count; i++) {
            int freq_khz = af_list[i];
            LOGI("AF candidate[%d]: %d kHz\n", i, freq_khz);
        }
    }
    mAFSwitchingActive = false;
    return 0;
}

int FmRadioController_silab::CancelAfSwitchingProcess() {
    LOGI("CancelAfSwitchingProcess()\n");
    if (mAFSwitchingActive && mAFThread != 0) {
        mAFSwitchingActive = false;
        pthread_join(mAFThread, NULL);
        mAFThread = 0;
    }
    return 0;
}

int FmRadioController_silab::SortBasedOnSignalStrength(int* freqs, int count) {
    if (freqs == NULL || count <= 1) return 0;
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (freqs[j] < freqs[j + 1]) {
                int tmp = freqs[j];
                freqs[j] = freqs[j + 1];
                freqs[j + 1] = tmp;
            }
        }
    }
    return 0;
}

int FmRadioController_silab::SetAF_th(int th) {
    mAfThreshold = th;
    return 0;
}

int FmRadioController_silab::GetAF_th() {
    return mAfThreshold;
}

int FmRadioController_silab::SetAFValid_th(int th) {
    mAfValidThreshold = th;
    return 0;
}

int FmRadioController_silab::GetAFValid_th() {
    return mAfValidThreshold;
}

int FmRadioController_silab::SetSeekRSSI(int th) {
    mRssiThreshold = th;
    setrssi_th((u8)th);
    return 0;
}

int FmRadioController_silab::SetSeekSNR(int th) {
    mSnrThreshold = th;
    setsnr_th((u8)th);
    return 0;
}

int FmRadioController_silab::SetRSSI_th(int th) {
    mRssiThreshold = th;
    setrssi_th((u8)th);
    return 0;
}

int FmRadioController_silab::GetRSSI_th() {
    return mRssiThreshold;
}

int FmRadioController_silab::SetSNR_th(int th) {
    mSnrThreshold = th;
    setsnr_th((u8)th);
    return 0;
}

int FmRadioController_silab::GetSNR_th() {
    return mSnrThreshold;
}

int FmRadioController_silab::SetCnt_th(char th) {
    mCntThreshold = (u8)th;
    setcnt_th((u8)th);
    return 0;
}

int FmRadioController_silab::GetCnt_th() {
    return mCntThreshold;
}

int FmRadioController_silab::setScanning(int scanning) {
    mIsScanning = (scanning != 0);
    return 0;
}
