
/* SILAB 47xx
 *
 * Copyright (C) 2019 Draekko, Ben Touchette
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

#include <linux/ioctl.h>
#include <sys/time.h>
#include "fmr.h"
#include "silab_ioctl.h"

int fd;

void setFd(int newfd) {
    fd = newfd;
}

int getFd() {
    return fd;
}

/* ================================================= */

int powerup () {
	if (ioctl(fd, SI47XX_IOC_POWERUP) != -1) {
		LOGI("SI47XX_IOC_POWERUP\n");
		return SILAB_TRUE;
	} 
	LOGI("POWERUP ERROR\n");
	return SILAB_FALSE;
}

int powerdown () {
	if (ioctl(fd, SI47XX_IOC_POWERDOWN) != -1) {
		LOGI("SI47XX_IOC_POWERDOWN\n");
		return SILAB_TRUE;
	} 
	LOGI("POWERDOWN ERROR\n");
	return SILAB_FALSE;
}


/* ================================================= */

int resetrds () {
	if (ioctl(fd, SI47XX_IOC_RESET_RDS_DATA) != -1) {
		LOGI("SI47XX_IOC_RESET_RDS_DATA\n");
		return SILAB_TRUE;
	} 
	LOGI("RESET RDS ERROR\n");
	return SILAB_FALSE;
}

int enablerds () {
	if (ioctl(fd, SI47XX_IOC_RDS_ENABLE) != -1) {
		LOGI("SI47XX_IOC_RDS_ENABLE\n");
		return SILAB_TRUE;
	} 
	LOGI("RDS ENABLE ERROR\n");
	return SILAB_FALSE;
}

int disablerds () {
	if (ioctl(fd, SI47XX_IOC_RDS_DISABLE) != -1) {
		LOGI("SI47XX_IOC_RDS_DISABLE\n");
		return SILAB_TRUE;
	} 
	LOGI("RDS DISABLE ERROR\n");
	return SILAB_FALSE;
}

int getrdsdata (struct radio_data_t *rds) {
	if (ioctl(fd, SI47XX_IOC_RDS_DATA_GET, rds) != -1) {
#ifdef DEBUG_RDS
		LOGI("SI47XX_IOC_RDS_DATA_GET\n");
#endif
		return SILAB_TRUE;
	} 
#ifdef DEBUG_RDS
	LOGI("GET RDS DATA ERROR\n");
#endif
	return SILAB_FALSE;
}

/* ================================================= */

int setsysconfig2 (struct sys_config2 *config2) {
	if (ioctl(fd, SI47XX_IOC_SYS_CONFIG2_SET, config2) != -1) {
		LOGI("SI47XX_IOC_SYS_CONFIG2_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("SET SYS CONGIG2 ERROR\n");
	return SILAB_FALSE;
}


int setsysconfig3 (struct sys_config3 *config3) {
	if (ioctl(fd, SI47XX_IOC_SYS_CONFIG3_SET, config3) != -1) {
		LOGI("SI47XX_IOC_SYS_CONFIG3_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("SET SYS CONGIG3 ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setdeconstant (u8 deconstant) {
	if (ioctl(fd, SI47XX_IOC_DE_SET, &deconstant) != -1) {
		LOGI("SI47XX_IOC_DE_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("SET DE TIME CONSTANT ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setrssi_th (u8 threshold) {
	if (ioctl(fd, SI47XX_IOC_RSSI_SEEK_TH_SET, &threshold) != -1) {
		LOGI("SI47XX_IOC_RSSI_SEEK_TH_SET\n");
		return SILAB_TRUE;
    } 
    LOGI("SET RSSI THRESHOLD ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setsnr_th (u8 threshold) {
	if (ioctl(fd, SI47XX_IOC_SEEK_SNR_SET, &threshold) != -1) {
		LOGI("SI47XX_IOC_SEEK_SNR_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("SET SNR THRESHOLD ERROR\n");
	return SILAB_FALSE;
}


/* ================================================= */

int setcnt_th (u8 threshold) {
	if (ioctl(fd, SI47XX_IOC_SEEK_CNT_SET, &threshold) != -1) {
		LOGI("SI47XX_IOC_SEEK_CNT_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("SET CNT THRESHOLD ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setband (int band) {
	if (ioctl(fd, SI47XX_IOC_BAND_SET, &band) != -1) {
		LOGI("SI47XX_IOC_BAND_SET\n");
		return SILAB_TRUE;
    } 
    LOGI("SET BAND ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setchannelspacing (int chansp) {
	if (ioctl(fd, SI47XX_IOC_CHAN_SPACING_SET, &chansp) != -1) {
		LOGI("SI47XX_IOC_CHAN_SPACING_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("SET CHANNEL SPACING ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setvolume (u8 vol) {
	if (ioctl(fd, SI47XX_IOC_VOLUME_SET, &vol) != -1) {
		LOGI("SI47XX_IOC_VOLUME_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("SET VOLUME ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setfreq (u32 freq) {
	if (ioctl(fd, SI47XX_IOC_CHAN_SELECT, &freq) != -1) {
		LOGI("SI47XX_IOC_CHAN_SELECT\n");
		return SILAB_TRUE;
	} 
	LOGI("CHAN SELECT ERROR\n");
	return SILAB_FALSE;
}

int getfreq (u32 *freq) {
	if (ioctl(fd, SI47XX_IOC_CHAN_GET, freq) != -1) {
		LOGI("SI47XX_IOC_CHAN_GET\n");
		return SILAB_TRUE;
	} 
	LOGI("CHAN SELECT ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setmono () {
	if (ioctl(fd, SI47XX_IOC_MONO_SET) != -1) {
		LOGI("SI47XX_IOC_MONO_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("MONO SET ERROR\n");
	return SILAB_FALSE;
}

int setstereo () {
	if (ioctl(fd, SI47XX_IOC_STEREO_SET) != -1) {
		LOGI("SI47XX_IOC_STEREO_SET\n");
		return SILAB_TRUE;
	} 
	LOGI("STEREO SET ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setdsmuteon () {
	if (ioctl(fd, SI47XX_IOC_DSMUTE_ON) != -1) {
		LOGI("SI47XX_IOC_DSMUTE_ON\n");
		return SILAB_TRUE;
	} 
	LOGI("DS MUTE ON ERROR\n");
	return SILAB_FALSE;
}

int setdsmuteoff () {
	if (ioctl(fd, SI47XX_IOC_DSMUTE_OFF) != -1) {
		LOGI("SI47XX_IOC_DSMUTE_OFF\n");
		return SILAB_TRUE;
	} 
	LOGI("DS MUTE OFF ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int setmuteon () {
	if (ioctl(fd, SI47XX_IOC_MUTE_ON) != -1) {
		LOGI("SI47XX_IOC_MUTE_ON\n");
		return SILAB_TRUE;
	} 
	LOGI("MUTE ON ERROR\n");
	return SILAB_FALSE;
}

int setmuteoff () {
	if (ioctl(fd, SI47XX_IOC_MUTE_OFF) != -1) {
		LOGI("SI47XX_IOC_MUTE_OFF\n");
		return SILAB_TRUE;
	} 
	LOGI("MUTE OFF ERROR\n");
	return SILAB_FALSE;
}

/* ================================================= */

int seekdown(u32 *freq) {
	if (ioctl(fd, SI47XX_IOC_SEEK_DOWN, freq) != -1) {
		LOGI("SI47XX_IOC_SEEK_DOWN\n");
		return SILAB_TRUE;
	} 
	LOGI("SEEK DOWN ERROR\n");
	return SILAB_FALSE;
}

int seekup(u32 *freq) {
	if (ioctl(fd, SI47XX_IOC_SEEK_UP, freq) != -1) {
		LOGI("SI47XX_IOC_SEEK_UP\n");
		return SILAB_TRUE;
	} 
	LOGI("SEEK UP ERROR\n");
	return SILAB_FALSE;
}

int seekfull(u32 *freqs) {
	if (ioctl(fd, SI47XX_IOC_SEEK_FULL, freqs) != -1) {
		LOGI("SI47XX_IOC_SEEK_FULL\n");
		return SILAB_TRUE;
	} 
	LOGI("SEEK FULL SCAN ERROR\n");
	return SILAB_FALSE;
}

int seekstop() {
	if (ioctl(fd, SI47XX_IOC_SEEK_CANCEL) != -1) {
		LOGI("SI47XX_IOC_SEEK_CANCEL\n");
		return SILAB_TRUE;
	} 
	LOGI("SEEK STOP ERROR\n");
	return SILAB_FALSE;
}

short activeaf() {
	return -1;
}

//nt enabledigitalmode() {
//	if (ioctl(fd, SI47XX_IOC_ENABLE_DIGITALMODE) != -1) {
//		LOGI("SI47XX_IOC_ENABLE_DIGITALMODE\n");
//		return SILAB_TRUE;
//	} 
//	LOGI("ENABLE DIGITALMODE ERROR\n");
//	return SILAB_FALSE;
//

//int disabledigitalmode() {
//	if (ioctl(fd, SI47XX_IOC_DISABLE_DIGITALMODE) != -1) {
//		LOGI("SI47XX_IOC_DISABLE_DIGITALMODE\n");
//		return SILAB_TRUE;
//	} 
//	LOGI("DISABLE DIGITALMODE ERROR\n");
//	return SILAB_FALSE;
//}

/* ================================================= */

