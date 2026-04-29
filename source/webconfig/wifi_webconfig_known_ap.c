/************************************************************************************
  If not stated otherwise in this file or this component's LICENSE file the
  following copyright and licenses apply:

  Copyright 2018 RDK Management

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
**************************************************************************/

#include "collection.h"
#include "wifi_webconfig.h"
#include "wifi_util.h"
#include "wifi_ctrl.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

webconfig_subdoc_object_t wifi_knownap_objects[3] = {
    { webconfig_subdoc_object_type_version, "Version"    },
    { webconfig_subdoc_object_type_subdoc,  "SubDocName" },
    { webconfig_subdoc_object_type_config,  "Parameters" }
};

webconfig_error_t init_knownap_subdoc(webconfig_subdoc_t *doc)
{
    doc->num_objects = sizeof(wifi_knownap_objects) / sizeof(webconfig_subdoc_object_t);
    memcpy((unsigned char *)doc->objects, (unsigned char *)&wifi_knownap_objects,
        sizeof(wifi_knownap_objects));
    return webconfig_error_none;
}

webconfig_error_t access_knownap_subdoc(webconfig_t *config, webconfig_subdoc_data_t *data)
{
    return webconfig_error_none;
}

webconfig_error_t translate_from_knownap_subdoc(webconfig_t *config,
    webconfig_subdoc_data_t *data)
{
    return webconfig_error_none;
}

webconfig_error_t translate_to_knownap_subdoc(webconfig_t *config,
    webconfig_subdoc_data_t *data)
{
    return webconfig_error_none;
}

/* ================================================================
 *  encode_knownap_subdoc
 * ================================================================ */
webconfig_error_t encode_knownap_subdoc(webconfig_t          *config,
                                         webconfig_subdoc_data_t *data)
{
    cJSON                          *json      = NULL;
    cJSON                          *obj_array = NULL;
    webconfig_subdoc_decoded_data_t *params   = NULL;
    wifi_vap_info_map_t            *vap_map   = NULL;
    rdk_wifi_vap_info_t            *rdk_vap_info = NULL;
    char                           *str       = NULL;
    unsigned int                    i, j;

    if (!data) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d NULL data pointer\n",
                              __func__, __LINE__);
        return webconfig_error_encode;
    }

    params = &data->u.decoded;
    if (!params) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d NULL params pointer\n",
                              __func__, __LINE__);
        return webconfig_error_encode;
    }

    wifi_util_dbg_print(WIFI_WEBCONFIG,
                        "%s:%d enter num_radios=%u\n",
                        __func__, __LINE__, params->num_radios);

    json = cJSON_CreateObject();
    if (!json) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d cJSON_CreateObject failed\n",
                              __func__, __LINE__);
        return webconfig_error_encode;
    }

    data->u.encoded.json = json;

    cJSON_AddStringToObject(json, "Version",    "1.0");
    cJSON_AddStringToObject(json, "SubDocName", "known_ap");

    obj_array = cJSON_CreateArray();
    if (!obj_array) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d cJSON_CreateArray failed\n",
                              __func__, __LINE__);
        cJSON_Delete(json);
        return webconfig_error_encode;
    }
    cJSON_AddItemToObject(json, "KnownAPTable", obj_array);

    for (i = 0; i < params->num_radios; i++) {
        vap_map = &params->radios[i].vaps.vap_map;

        wifi_util_dbg_print(WIFI_WEBCONFIG,
                            "%s:%d encoding radio=%u num_vaps=%u\n",
                            __func__, __LINE__, i, vap_map->num_vaps);

        for (j = 0; j < vap_map->num_vaps; j++) {
            rdk_vap_info = &params->radios[i].vaps.rdk_vap_array[j];

            wifi_util_dbg_print(WIFI_WEBCONFIG,
                                "%s:%d encoding radio=%u vap_idx=%u "
                                "vap_name=%s vap_index=%u\n",
                                __func__, __LINE__,
                                i, j,
                                rdk_vap_info->vap_name,
                                rdk_vap_info->vap_index);

            if (encode_knownap_object(rdk_vap_info, obj_array)
                    != webconfig_error_none) {
                wifi_util_error_print(WIFI_WEBCONFIG,
                                      "%s:%d encode_knownap_object failed "
                                      "radio=%u vap_idx=%u vap_name=%s\n",
                                      __func__, __LINE__,
                                      i, j, rdk_vap_info->vap_name);
                cJSON_Delete(json);
                return webconfig_error_encode;
            }
        }
    }

    str = cJSON_Print(json);
    if (!str) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d cJSON_Print failed\n",
                              __func__, __LINE__);
        cJSON_Delete(json);
        return webconfig_error_encode;
    }

    data->u.encoded.raw =
        (webconfig_subdoc_encoded_raw_t)calloc(strlen(str) + 1, sizeof(char));
    if (!data->u.encoded.raw) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d calloc failed len=%zu\n",
                              __func__, __LINE__, strlen(str) + 1);
        cJSON_free(str);
        cJSON_Delete(json);
        return webconfig_error_encode;
    }

    memcpy(data->u.encoded.raw, str, strlen(str));
    cJSON_free(str);
    cJSON_Delete(json);

    wifi_util_info_print(WIFI_WEBCONFIG,
                         "%s:%d encode success num_radios=%u\n",
                         __func__, __LINE__, params->num_radios);
    return webconfig_error_none;
}

/* ================================================================
 *  decode_knownap_subdoc
 * ================================================================ */
webconfig_error_t decode_knownap_subdoc(webconfig_t          *config,
                                         webconfig_subdoc_data_t *data)
{
    webconfig_subdoc_t              *doc          = NULL;
    webconfig_subdoc_decoded_data_t *params       = NULL;
    cJSON                           *json         = NULL;
    cJSON                           *obj_table    = NULL;
    rdk_wifi_vap_info_t             *rdk_vap_info = NULL;
    unsigned int                     i            = 0;
    unsigned int                     size         = 0;
    unsigned int                     radio_index  = 0;
    unsigned int                     vap_array_index = 0;

    params = &data->u.decoded;
    if (!params) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d NULL params\n", __func__, __LINE__);
        return webconfig_error_decode;
    }

    json = data->u.encoded.json;
    if (!json) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d NULL json pointer\n",
                              __func__, __LINE__);
        return webconfig_error_decode;
    }

    doc = &config->subdocs[data->type];

    wifi_util_dbg_print(WIFI_WEBCONFIG,
                        "%s:%d Encoded JSON:\n%s\n",
                        __func__, __LINE__,
                        (char *)data->u.encoded.raw);

    /* ---- Validate required top-level objects ------------------- */
    for (i = 0; i < doc->num_objects; i++) {
        if (!cJSON_GetObjectItem(json, doc->objects[i].name)) {
            wifi_util_error_print(WIFI_WEBCONFIG,
                                  "%s:%d object '%s' not present, "
                                  "validation failed\n",
                                  __func__, __LINE__,
                                  doc->objects[i].name);
            cJSON_Delete(json);
            return webconfig_error_invalid_subdoc;
        }
    }

    /* ---- Get KnownAPTable array -------------------------------- */
    obj_table = cJSON_GetObjectItem(json, "KnownAPTable");
    if (!cJSON_IsArray(obj_table)) {
        wifi_util_error_print(WIFI_WEBCONFIG,
                              "%s:%d KnownAPTable not present or not array\n",
                              __func__, __LINE__);
        cJSON_Delete(json);
        return webconfig_error_invalid_subdoc;
    }

    size = (unsigned int)cJSON_GetArraySize(obj_table);

    wifi_util_dbg_print(WIFI_WEBCONFIG,
                        "%s:%d KnownAPTable array size=%u\n",
                        __func__, __LINE__, size);

    /* ---- Decode each VAP entry --------------------------------- */
    for (i = 0; i < size; i++) {
        cJSON *obj_vap = cJSON_GetArrayItem(obj_table, i);
        if (!obj_vap) {
            wifi_util_error_print(WIFI_WEBCONFIG,
                                  "%s:%d NULL array item at i=%u\n",
                                  __func__, __LINE__, i);
            continue;
        }

        char *name = cJSON_GetStringValue(
            cJSON_GetObjectItem(obj_vap, "VapName"));
        if (!name) {
            wifi_util_error_print(WIFI_WEBCONFIG,
                                  "%s:%d VapName missing at i=%u\n",
                                  __func__, __LINE__, i);
            continue;
        }

        radio_index =
            convert_vap_name_to_radio_array_index(
                &params->hal_cap.wifi_prop, name);
        vap_array_index =
            convert_vap_name_to_array_index(
                &params->hal_cap.wifi_prop, name);

        if ((int)radio_index < 0 || (int)vap_array_index < 0) {
            wifi_util_error_print(WIFI_WEBCONFIG,
                                  "%s:%d invalid index for vap_name=%s "
                                  "radio_index=%u vap_array_index=%u\n",
                                  __func__, __LINE__,
                                  name, radio_index, vap_array_index);
            continue;
        }

        rdk_vap_info =
            &params->radios[radio_index].vaps.rdk_vap_array[vap_array_index];

        rdk_vap_info->vap_index =
            convert_vap_name_to_index(&params->hal_cap.wifi_prop, name);

        if ((int)rdk_vap_info->vap_index < 0) {
            wifi_util_error_print(WIFI_WEBCONFIG,
                                  "%s:%d invalid vap_index for "
                                  "vap_name=%s\n",
                                  __func__, __LINE__, name);
            continue;
        }

        wifi_util_dbg_print(WIFI_WEBCONFIG,
                            "%s:%d decoding i=%u vap_name=%s "
                            "radio_index=%u vap_array_index=%u "
                            "vap_index=%u\n",
                            __func__, __LINE__,
                            i, name, radio_index,
                            vap_array_index, rdk_vap_info->vap_index);

        if (decode_knownap_object(rdk_vap_info, obj_vap)
                != webconfig_error_none) {
            wifi_util_error_print(WIFI_WEBCONFIG,
                                  "%s:%d decode_knownap_object failed "
                                  "vap_name=%s radio_index=%u "
                                  "vap_array_index=%u\n",
                                  __func__, __LINE__,
                                  name, radio_index, vap_array_index);
            cJSON_Delete(json);
            return webconfig_error_decode;
        }
    }

    wifi_util_info_print(WIFI_WEBCONFIG,
                         "%s:%d decode success size=%u\n",
                         __func__, __LINE__, size);
    cJSON_Delete(json);
    return webconfig_error_none;
}