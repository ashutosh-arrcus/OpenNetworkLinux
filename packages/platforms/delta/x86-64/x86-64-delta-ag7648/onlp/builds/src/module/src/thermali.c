/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform_lib.h"
#include "x86_64_delta_ag7648_log.h"
#include <stdio.h>
#define prefix_path "/sys/bus/i2c/devices/"
#define LOCAL_DEBUG 0

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_1_CLOSE_TO_CPU,
    THERMAL_1_CLOSE_TO_MAC,
	THERMAL_2_CLOSE_TO_PHY_SFP_PLUS,
	THERMAL_3_CLOSE_TO_PHY_QSFP,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char* last_path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    "2-004d/hwmon/hwmon1/temp1_input",
    "3-004c/hwmon/hwmon2/temp1_input",
    "3-004d/hwmon/hwmon3/temp1_input",
    "3-004e/hwmon/hwmon4/temp1_input",
    "4-0058/psu_temp1_input",
	"5-0058/psu_temp1_input",
};
	
/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_CLOSE_TO_CPU), "Thermal Sensor 1- close to cpu", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_1_CLOSE_TO_MAC), "Thermal Sensor 1- close to mac", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_CLOSE_TO_PHY_SFP_PLUS), "Thermal Sensor 2- close to sfp+ phy", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_CLOSE_TO_PHY_QSFP), "Thermal Sensor 2- close to qsfp phy", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(1)},
	    ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(2)},
	    ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    }
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{ 
    return ONLP_STATUS_OK;
}

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int   len, nbytes = 10, temp_base=1, local_id;
    uint8_t r_data[10]={0};
    char  fullpath[50] = {0};
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    
    DEBUG_PRINT("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];
    /* get fullpath */
    sprintf(fullpath, "%s%s", prefix_path, last_path[local_id]);

    //OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
    onlp_file_read(r_data,nbytes,&len, fullpath);
    
    info->mcelsius =ONLPLIB_ATOI((char*)r_data) / temp_base;
    
    DEBUG_PRINT("\n[Debug][%s][%d][save data: %d]\n", __FUNCTION__, __LINE__, info->mcelsius);

    return ONLP_STATUS_OK;
}

