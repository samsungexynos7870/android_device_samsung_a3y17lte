/* SILAB 47xx IOCTL Header Matching Official Linux Kernel Driver (si47xx_ioctl.h / si47xx_dev.h)
 *
 * Copyright (C) 2015 Samsung Electronics Co., Ltd.
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

#ifndef __SILAB_IOCTL_H__
#define __SILAB_IOCTL_H__

#include <linux/types.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SILAB_TRUE  1
#define SILAB_FALSE 0

#ifndef BOTTOM_FREQ_8750
#define BOTTOM_FREQ_8750 8750
#endif

#ifndef TOP_FREQ_10800
#define TOP_FREQ_10800 10800
#endif

#ifndef MAX_FM_SCAN_CH_SIZE
#define MAX_FM_SCAN_CH_SIZE 60
#endif

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

struct dev_state_t {
    int power_state;
    int seek_state;
};

struct rssi_snr_t {
    u8 curr_rssi;
    u8 curr_rssi_th;
    u8 curr_snr;
};

struct device_id {
    u8 part_number;
    u16 manufact_number;
};

struct chip_id {
    u8 chip_version;
    u8 device;
    u8 firmware_version;
};

struct sys_config2 {
    u16 rssi_th;
    u8 fm_band;
    u8 fm_chan_spac;
    u8 fm_vol;
};

struct sys_config3 {
    u8 smmute;
    u8 smutea;
    u8 volext;
    u8 sksnr;
    u8 skcnt;
};

struct status_rssi {
    u8 rdsr;
    u8 stc;
    u8 sfbl;
    u8 afcrl;
    u8 rdss;
    u8 blera;
    u8 st;
    u16 rssi;
};

struct radio_data_t {
    u16 rdsa;
    u16 rdsb;
    u16 rdsc;
    u16 rdsd;
    u8 curr_rssi;
    u32 curr_channel;
    u8 blera;
    u8 blerb;
    u8 blerc;
    u8 blerd;
};

struct channel_into_t {
    u32 frequency;
    u8 rsssi_val;
};

struct tune_data_t {
    u8 stc;
    u8 bltf;
    u8 afcrl;
    u8 valid;
    u16 freq;
    u8 rssi;
    u8 asnr;
    u16 antcap;
};

/* MAGIC NUMBER */
#define SI47XX_IOC_MAGIC		0xFA

/* MAX SEQ NUMBER */
#define SI47XX_IOC_NR_MAX		41

/* COMMANDS */
#define SI47XX_IOC_POWERUP		_IO(SI47XX_IOC_MAGIC, 0)
#define SI47XX_IOC_POWERDOWN		_IO(SI47XX_IOC_MAGIC, 1)
#define SI47XX_IOC_BAND_SET		_IOW(SI47XX_IOC_MAGIC, 2, int)
#define SI47XX_IOC_CHAN_SPACING_SET	_IOW(SI47XX_IOC_MAGIC, 3, int)
#define SI47XX_IOC_CHAN_SELECT		_IOW(SI47XX_IOC_MAGIC, 4, u32)
#define SI47XX_IOC_CHAN_GET		_IOR(SI47XX_IOC_MAGIC, 5, u32)
#define SI47XX_IOC_SEEK_UP		_IOR(SI47XX_IOC_MAGIC, 6, u32)
#define SI47XX_IOC_SEEK_DOWN		_IOR(SI47XX_IOC_MAGIC, 7, u32)
#define SI47XX_IOC_RSSI_SEEK_TH_SET	_IOW(SI47XX_IOC_MAGIC, 9, u8)
#define SI47XX_IOC_SEEK_SNR_SET		_IOW(SI47XX_IOC_MAGIC, 10, u8)
#define SI47XX_IOC_SEEK_CNT_SET		_IOW(SI47XX_IOC_MAGIC, 11, u8)
#define SI47XX_IOC_CUR_RSSI_GET		_IOR(SI47XX_IOC_MAGIC, 12, struct rssi_snr_t)
#define SI47XX_IOC_VOLEXT_ENB		_IO(SI47XX_IOC_MAGIC, 13)
#define SI47XX_IOC_VOLEXT_DISB		_IO(SI47XX_IOC_MAGIC, 14)
#define SI47XX_IOC_VOLUME_SET		_IOW(SI47XX_IOC_MAGIC, 15, u8)
#define SI47XX_IOC_VOLUME_GET		_IOR(SI47XX_IOC_MAGIC, 16, u8)
#define SI47XX_IOC_MUTE_ON		_IO(SI47XX_IOC_MAGIC, 17)
#define SI47XX_IOC_MUTE_OFF		_IO(SI47XX_IOC_MAGIC, 18)
#define SI47XX_IOC_MONO_SET		_IO(SI47XX_IOC_MAGIC, 19)
#define SI47XX_IOC_STEREO_SET		_IO(SI47XX_IOC_MAGIC, 20)
#define SI47XX_IOC_RSTATE_GET		_IOR(SI47XX_IOC_MAGIC, 21, struct dev_state_t)
#define SI47XX_IOC_RDS_DATA_GET		_IOR(SI47XX_IOC_MAGIC, 22, struct radio_data_t)
#define SI47XX_IOC_RDS_ENABLE		_IO(SI47XX_IOC_MAGIC, 23)
#define SI47XX_IOC_RDS_DISABLE		_IO(SI47XX_IOC_MAGIC, 24)
#define SI47XX_IOC_RDS_TIMEOUT_SET	_IOW(SI47XX_IOC_MAGIC, 25, u32)
#define SI47XX_IOC_SEEK_CANCEL		_IO(SI47XX_IOC_MAGIC, 26)
#define SI47XX_IOC_DEVICE_ID_GET	_IOR(SI47XX_IOC_MAGIC, 27, struct device_id)
#define SI47XX_IOC_CHIP_ID_GET		_IOR(SI47XX_IOC_MAGIC, 28, struct chip_id)
#define SI47XX_IOC_SYS_CONFIG2_GET	_IOR(SI47XX_IOC_MAGIC, 29, struct sys_config2)
#define SI47XX_IOC_POWER_CONFIG_GET	_IO(SI47XX_IOC_MAGIC, 30)
#define SI47XX_IOC_AFCRL_GET		_IOR(SI47XX_IOC_MAGIC, 31, u8)
#define SI47XX_IOC_DE_SET		_IOW(SI47XX_IOC_MAGIC, 32, u8)
#define SI47XX_IOC_SYS_CONFIG3_GET	_IOR(SI47XX_IOC_MAGIC, 33, struct sys_config3)
#define SI47XX_IOC_STATUS_RSSI_GET	_IOR(SI47XX_IOC_MAGIC, 34, struct status_rssi)
#define SI47XX_IOC_SYS_CONFIG2_SET	_IOW(SI47XX_IOC_MAGIC, 35, struct sys_config2)
#define SI47XX_IOC_SYS_CONFIG3_SET	_IOW(SI47XX_IOC_MAGIC, 36, struct sys_config3)
#define SI47XX_IOC_DSMUTE_ON		_IO(SI47XX_IOC_MAGIC, 37)
#define SI47XX_IOC_DSMUTE_OFF		_IO(SI47XX_IOC_MAGIC, 38)
#define SI47XX_IOC_RESET_RDS_DATA	_IO(SI47XX_IOC_MAGIC, 39)
#define SI47XX_IOC_SEEK_FULL		_IOR(SI47XX_IOC_MAGIC, 40, u32)
#define SI47XX_IOC_CHAN_CHECK_VALID	_IOR(SI47XX_IOC_MAGIC, 41, bool)

/* Wrapper prototypes from silab_ioctl.cpp */
void setFd(int newfd);
int getFd(void);
int powerup(void);
int powerdown(void);
int resetrds(void);
int enablerds(void);
int disablerds(void);
int getrdsdata(struct radio_data_t *rds);
int setsysconfig2(struct sys_config2 *config2);
int setsysconfig3(struct sys_config3 *config3);
int setdeconstant(u8 deconstant);
int setrssi_th(u8 threshold);
int setsnr_th(u8 threshold);
int setcnt_th(u8 threshold);
int setband(int band);
int setchannelspacing(int chansp);
int setvolume(u8 vol);
int setfreq(u32 freq);
int getfreq(u32 *freq);
int setmono(void);
int setstereo(void);
int setdsmuteon(void);
int setdsmuteoff(void);
int setmuteon(void);
int setmuteoff(void);
int seekdown(u32 *freq);
int seekup(u32 *freq);
int seekfull(u32 *freqs);
int seekstop(void);
short activeaf(void);

#ifdef __cplusplus
}
#endif

#endif // __SILAB_IOCTL_H__
