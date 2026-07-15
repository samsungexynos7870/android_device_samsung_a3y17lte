
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

#ifndef __SILAB_FM_H__
#define __SILAB_FM_H__

#include "silab_ioctl.h"

/*band*/
#define BAND_87500_108000_kHz	1
#define BAND_76000_108000_kHz	2
#define BAND_76000_90000_kHz	3

/*channel spacing*/
#define CHAN_SPACING_200_kHz	20	/*US*/
#define CHAN_SPACING_100_kHz	10	/*Europe,Japan */
#define CHAN_SPACING_50_kHz	5

/*DE-emphasis Time Constant*/
#define DE_TIME_CONSTANT_50	    1	/*Europe,Japan,Australia */
#define DE_TIME_CONSTANT_75	    0	/*US*/

#define GROUP_TYPE_0A 0
#define GROUP_TYPE_0B 1
#define GROUP_TYPE_2A 4
#define GROUP_TYPE_2B 5

#define RDS_EVENT_PROGRAMNAME    0x0008
#define RDS_EVENT_LAST_RADIOTEXT 0x0040
#define RDS_EVENT_AF             0x0080
#define RDS_EVENT_PTY            0x0100

#endif // __SILAB_FM_H__
