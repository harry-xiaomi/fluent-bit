/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2015-2022 The Fluent Bit Authors
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

#include <fluent-bit/flb_output_plugin.h>
#include <fluent-bit/flb_pack.h>
#include <fluent-bit/flb_str.h>
#include <fluent-bit/flb_time.h>
#include <fluent-bit/flb_utils.h>
#include <fluent-bit/flb_pack.h>
#include <fluent-bit/flb_sds.h>
#include <fluent-bit/flb_config_map.h>
#include <msgpack.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tcp.h"
#include "tcp_conf.h"

static int cb_tcp_init(struct flb_output_instance *ins,
                       struct flb_config *config, void *data)
{
    struct flb_out_tcp *ctx = NULL;
    (void) data;

    ctx = flb_tcp_conf_create(ins, config);
    if (!ctx) {
        return -1;
    }

    /* Set the plugin context */
    flb_output_set_context(ins, ctx);

    return 0;
}

static void cb_tcp_flush(struct flb_event_chunk *event_chunk,
                         struct flb_output_flush *out_flush,
                         struct flb_input_instance *i_ins,
                         void *out_context,
                         struct flb_config *config)
{
    int ret = FLB_ERROR;
    size_t bytes_sent;
    flb_sds_t json = NULL;
    struct flb_upstream *u;
    struct flb_upstream_conn *u_conn;
    struct flb_out_tcp *ctx = out_context;
    (void) i_ins;

    /* Get upstream context and connection */
    u = ctx->u;
    u_conn = flb_upstream_conn_get(u);
    if (!u_conn) {
        flb_plg_error(ctx->ins, "no upstream connections available to %s:%i",
                      u->tcp_host, u->tcp_port);
        FLB_OUTPUT_RETURN(FLB_RETRY);
    }

    if (ctx->out_format == FLB_PACK_JSON_FORMAT_NONE) {
        ret = flb_io_net_write(u_conn,
                               event_chunk->data, event_chunk->size,
                               &bytes_sent);
    }
    else {
        json = flb_pack_msgpack_to_json_format(event_chunk->data,
                                               event_chunk->size,
                                               ctx->out_format,
                                               ctx->json_date_format,
                                               ctx->date_key);
        if (!json) {
            flb_plg_error(ctx->ins, "error formatting JSON payload");
            flb_upstream_conn_release(u_conn);
            FLB_OUTPUT_RETURN(FLB_ERROR);
        }
        ret = flb_io_net_write(u_conn, json, flb_sds_len(json), &bytes_sent);
        flb_sds_destroy(json);
    }

    if (ret == -1) {
        flb_errno();
        flb_upstream_conn_release(u_conn);
        FLB_OUTPUT_RETURN(FLB_RETRY);
    }

    flb_upstream_conn_release(u_conn);
    FLB_OUTPUT_RETURN(FLB_OK);
}

static int cb_tcp_exit(void *data, struct flb_config *config)
{
    struct flb_out_tcp *ctx = data;

    flb_tcp_conf_destroy(ctx);
    return 0;
}

/* Configuration properties map */
static struct flb_config_map config_map[] = {
    {
     FLB_CONFIG_MAP_STR, "format", "msgpack",
     0, FLB_FALSE, 0,
     "Specify the payload format, supported formats: msgpack, json, "
     "json_lines or json_stream."
    },

    {
     FLB_CONFIG_MAP_STR, "json_date_format", "double",
     0, FLB_FALSE, 0,
     FBL_PACK_JSON_DATE_FORMAT_DESCRIPTION
    },

    {
     FLB_CONFIG_MAP_STR, "json_date_key", "date",
     0, FLB_TRUE, offsetof(struct flb_out_tcp, json_date_key),
     "Specify the name of the date field in output."
    },

    /* EOF */
    {0}
};

/* Plugin reference */
struct flb_output_plugin out_tcp_plugin = {
    .name           = "tcp",
    .description    = "TCP Output",
    .cb_init        = cb_tcp_init,
    .cb_flush       = cb_tcp_flush,
    .cb_exit        = cb_tcp_exit,
    .config_map     = config_map,
    .workers        = 2,
    .flags          = FLB_OUTPUT_NET | FLB_IO_OPT_TLS,
};
