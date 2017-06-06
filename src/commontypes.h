/*
 * @@@ LICENSE
 * Copyright (C) 2016-2017, LG Electronics, All Right Reserved.
 * No part of this source code may be communicated, distributed, reproduced
 * or transmitted in any form or by any means, electronic or mechanical or
 * otherwise, for any purpose, without the prior written permission of
 * LG Electronics.
 * LICENSE @@@
 *
 * Design/Author : hyobeom1.lee@lge.com, dhilipkumar.raman@lge.com
 * Date : 06/04/2017
 *
 * <IHU XCallService Ecnr plugin component> Detailed note is as follows.
 *
 *
 */

#ifndef _GST_LGECNR_COMMONTYPE_H_
#define _GST_LGECNR_COMMONTYPE_H_

#include <stdio.h>
#include <stdint.h>

typedef enum DataSizeSample {
    kMaxDataSizeSamples = 3840
} DataSizeSample;

typedef struct AudioFrame {  // Stereo, 32 kHz, 60 ms (2 * 32 * 60)

    int16_t data_[kMaxDataSizeSamples];
    size_t samples_per_channel_;
    int sample_rate_hz_;
    size_t num_channels_;
    size_t size;
    GstClockTime timestamp;

} AudioFrame;

#endif	//_GST_LGECNR_COMMONTYPE_H_
