/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  CMetrics
 *  ========
 *  Copyright 2021 Eduardo Silva <eduardo@calyptia.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#ifndef CMT_ENCODE_OPENTELEMETRY_H
#define CMT_ENCODE_OPENTELEMETRY_H

#include <cmetrics/cmetrics.h>
#include <cmetrics/cmt_sds.h>
#include <opentelemetry/metrics.pb-c.h>
#include <opentelemetry/metrics_service.pb-c.h>

#define CMT_ENCODE_OPENTELEMETRY_SUCCESS                0
#define CMT_ENCODE_OPENTELEMETRY_ALLOCATION_ERROR       1
#define CMT_ENCODE_OPENTELEMETRY_INVALID_ARGUMENT_ERROR 2
#define CMT_ENCODE_OPENTELEMETRY_UNEXPECTED_METRIC_TYPE 3

struct cmt_opentelemetry_context
{
    Opentelemetry__Proto__Metrics__V1__MetricsData *metrics_data;
    struct cmt                                     *cmt;
};

cmt_sds_t cmt_encode_opentelemetry_create(struct cmt *cmt);
void cmt_encode_opentelemetry_destroy(cmt_sds_t text);

#endif
