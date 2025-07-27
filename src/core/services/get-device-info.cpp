// https://github.com/libimobiledevice/libimobiledevice/blob/master/tools/ideviceinfo.c
/*
 * ideviceinfo.c
 * Simple utility to show information about an attached device
 *
 * Copyright (c) 2010-2019 Nikias Bassen, All Rights Reserved.
 * Copyright (c) 2009 Martin Szulecki All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <signal.h>
#endif

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <plist/plist.h>
#include <pugixml.hpp>

#define FORMAT_KEY_VALUE 1
#define FORMAT_XML 2

static const char *domains[] = {
    "com.apple.disk_usage", "com.apple.disk_usage.factory",
    "com.apple.mobile.battery",
    /* FIXME: For some reason lockdownd segfaults on this, works sometimes
       though "com.apple.mobile.debug",. */
    "com.apple.iqagent", "com.apple.purplebuddy", "com.apple.PurpleBuddy",
    "com.apple.mobile.chaperone", "com.apple.mobile.third_party_termination",
    "com.apple.mobile.lockdownd", "com.apple.mobile.lockdown_cache",
    "com.apple.xcode.developerdomain", "com.apple.international",
    "com.apple.mobile.data_sync", "com.apple.mobile.tethered_sync",
    "com.apple.mobile.mobile_application_usage", "com.apple.mobile.backup",
    "com.apple.mobile.nikita", "com.apple.mobile.restriction",
    "com.apple.mobile.user_preferences", "com.apple.mobile.sync_data_class",
    "com.apple.mobile.software_behavior",
    "com.apple.mobile.iTunes.SQLMusicLibraryPostProcessCommands",
    "com.apple.mobile.iTunes.accessories",
    "com.apple.mobile.internal",          /**< iOS 4.0+ */
    "com.apple.mobile.wireless_lockdown", /**< iOS 4.0+ */
    "com.apple.fairplay", "com.apple.iTunes", "com.apple.mobile.iTunes.store",
    "com.apple.mobile.iTunes", "com.apple.fmip", "com.apple.Accessibility",
    NULL};

plist_t get_device_info(const char *udid, int use_network, int simple,
                        lockdownd_client_t client, idevice_t device)
{
    lockdownd_error_t ldret = LOCKDOWN_E_UNKNOWN_ERROR;
    idevice_error_t ret = IDEVICE_E_UNKNOWN_ERROR;
    plist_t node = NULL;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    // FIXME: network support does not properly work yet
    // ret = idevice_new_with_options(&device, udid, (use_network) ?
    // IDEVICE_LOOKUP_NETWORK : IDEVICE_LOOKUP_USBMUX);
    ret = idevice_new_with_options(&device, udid, IDEVICE_LOOKUP_USBMUX);
    if (ret != IDEVICE_E_SUCCESS) {
        if (udid) {
            fprintf(stderr, "ERROR: Device %s not found!\n", udid);
        } else {
            fprintf(stderr, "ERROR: No device found!\n");
        }
        return NULL;
    }

    if (LOCKDOWN_E_SUCCESS !=
        (ldret = simple ? lockdownd_client_new(device, &client, "iDescriptor")
                        : lockdownd_client_new_with_handshake(device, &client,
                                                              "iDescriptor"))) {
        fprintf(stderr, "ERROR: Could not connect to lockdownd: %s (%d)\n",
                lockdownd_strerror(ldret), ldret);
        return NULL;
    }

    /* run query and output information */
    if (lockdownd_get_value(client, NULL, NULL, &node) != LOCKDOWN_E_SUCCESS) {
        fprintf(stderr, "ERROR: Could not get value\n");
    }

    return node;
}

// Change return type to void
void get_device_info_xml(const char *udid, int use_network, int simple,
                         pugi::xml_document &infoXml, lockdownd_client_t client,
                         idevice_t device)
{
    plist_t node = get_device_info(udid, use_network, simple, client, device);

    if (!node)
        return;

    char *xml_string = nullptr;
    uint32_t xml_length = 0;
    plist_to_xml(node, &xml_string, &xml_length);
    plist_free(node);

    if (xml_string) {
        infoXml.load_string(xml_string);
        free(xml_string);
    }
    // No return statement needed
}