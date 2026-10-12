/* tools/mkbootimg/samsung_dtbh.h
**
** Copyright 2017, The LineageOS Project
** Copyright (C) 2026 samsungexynos7870
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/*
 * DTBH values for the Galaxy A3 2017 (a3y17lte).
 *
 * DTBH_MODEL is matched against the model property in each compiled DTB. The
 * kernel branch's A3 DTBs identify themselves as, for example,
 * "Samsung A3Y17 LTE EUR rev00 board based on Exynos7870". Keeping this match
 * prevents DTBs for other devices sharing the kernel source from being packed.
 *
 * DTBH_PLATFORM / DTBH_SUBTYPE must match model_info-platform and
 * model_info-subtype in those DTBs. The corresponding numeric codes below are
 * the Exynos 7870 DTBH codes written into the table and compared by sboot.
 */

#ifndef _SAMSUNG_DTBH_H_
#define _SAMSUNG_DTBH_H_

#define DTBH_MAGIC         "DTBH"
#define DTBH_VERSION       2
#define DTBH_MODEL         "A3Y17"
#define DTBH_PLATFORM      "android"
#define DTBH_SUBTYPE       "samsung"
#define DTBH_PLATFORM_CODE 0x000050a6
#define DTBH_SUBTYPE_CODE  0x217584da

/* DTBH_MAGIC + DTBH_VERSION + DTB counts */
#define DT_HEADER_PHYS_SIZE 12

/* Keep the eight uint32_t entries first in this struct for memcpy to file. */
#define DT_ENTRY_PHYS_SIZE (sizeof(uint32_t) * 8)

#endif // _SAMSUNG_DTBH_H_
