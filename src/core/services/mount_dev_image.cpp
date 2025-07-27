/*
 * ideviceimagemounter.c
 * Mount developer/debug disk images on the device
 *
 * Copyright (C) 2010 Nikias Bassen <nikias@gmx.li>
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

//  TODO:
// we need to dynamically fetch dev imgs because every iOS version has a
// different one

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define TOOL_NAME "ideviceimagemounter"

#include <stdlib.h>
#define _GNU_SOURCE 1
#define __USE_GNU 1
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#ifndef _WIN32
#include <signal.h>
#endif

#include <QDebug>
#include <libimobiledevice-glue/sha.h>
#include <libimobiledevice-glue/utils.h>
#include <libimobiledevice/afc.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/mobile_image_mounter.h>
#include <libimobiledevice/notification_proxy.h>
#include <libtatsu/tss.h>
#include <plist/plist.h>
#include <printf.h>

static int list_mode = 0;
static int use_network = 0;
static int xml_mode = 0;
static const char *udid = NULL;
static const char *imagetype = NULL;

static const char PKG_PATH[] = "PublicStaging";
static const char PATH_PREFIX[] = "/private/var/mobile/Media";

typedef enum {
    DISK_IMAGE_UPLOAD_TYPE_AFC,
    DISK_IMAGE_UPLOAD_TYPE_UPLOAD_IMAGE
} disk_image_upload_type_t;

enum cmd_mode {
    CMD_NONE = 0,
    CMD_MOUNT,
    CMD_UNMOUNT,
    CMD_LIST,
    CMD_DEVMODESTATUS
};

// int cmd = CMD_NONE;

#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif
static ssize_t mim_upload_cb(void *buf, size_t size, void *userdata)
{
    return fread(buf, 1, size, (FILE *)userdata);
}
// TODO: cleanup
bool mount_dev_image(char *udid)
{
    mobile_image_mounter_client_t mim = NULL;
    int res = -1;
    size_t image_size = 0;
    lockdownd_client_t lckd = NULL;
    lockdownd_error_t ldret = LOCKDOWN_E_UNKNOWN_ERROR;
    afc_client_t afc = NULL;
    lockdownd_service_descriptor_t service = NULL;
    idevice_t device = NULL;

    if (IDEVICE_E_SUCCESS !=
        idevice_new_with_options(&device, udid,
                                 (use_network) ? IDEVICE_LOOKUP_NETWORK
                                               : IDEVICE_LOOKUP_USBMUX)) {
        qDebug() << "ERROR: Could not create idevice!";
        return false;
    }

    if (LOCKDOWN_E_SUCCESS != (ldret = lockdownd_client_new_with_handshake(
                                   device, &lckd, TOOL_NAME))) {
        qDebug() << "ERROR: Could not connect to lockdownd service!";
        return false;
        // goto leave;
    }

    lockdownd_error_t lerr =
        lockdownd_start_service(lckd, "com.apple.afc", &service);
    if (lerr != LOCKDOWN_E_SUCCESS) {
        qDebug() << "ERROR: Could not start AFC service!"
                 << lockdownd_strerror(lerr) << "(" << lerr << ")";
        lockdownd_client_free(lckd);
        lckd = NULL;
        idevice_free(device);
        device = NULL;
        return false;
    }

    afc_error_t rafc = afc_client_new(device, service, &afc);
    if (rafc != AFC_E_SUCCESS) {
        qDebug() << "ERROR: Could not connect to AFC!" << afc_strerror(rafc)
                 << "(" << rafc << ")";
        return false;
    }

    char *image_path = nullptr;
    char *image_sig_path = nullptr;

    if (asprintf(&image_path,
                 "%s/resources/dev-images/15/DeveloperDiskImage.dmg",
                 SOURCE_DIR) < 0) {
        qDebug() << "Out of memory constructing image path!";
        return false;
    }

    if (asprintf(&image_sig_path,
                 "%s/resources/dev-images/15/DeveloperDiskImage.dmg.signature",
                 SOURCE_DIR) < 0) {
        qDebug() << "Out of memory constructing signature path!";
        free(image_path);
        return false;
    }

    // #ifndef _WIN32
    // 	signal(SIGPIPE, SIG_IGN);
    // #endif

    unsigned int device_version = idevice_get_device_version(device);

    disk_image_upload_type_t disk_image_upload_type =
        DISK_IMAGE_UPLOAD_TYPE_AFC;
    if (device_version >= IDEVICE_DEVICE_VERSION(7, 0, 0)) {
        disk_image_upload_type = DISK_IMAGE_UPLOAD_TYPE_UPLOAD_IMAGE;
    }

    if (device_version >= IDEVICE_DEVICE_VERSION(16, 0, 0)) {
        uint8_t dev_mode_status = 0;
        plist_t val = NULL;
        ldret = lockdownd_get_value(lckd, "com.apple.security.mac.amfi",
                                    "DeveloperModeStatus", &val);
        if (ldret == LOCKDOWN_E_SUCCESS) {
            plist_get_bool_val(val, &dev_mode_status);
            plist_free(val);
        }
        if (!dev_mode_status) {
            qDebug() << "ERROR: You have to enable Developer Mode on the given "
                        "device in order to allowing mounting a developer disk "
                        "image.";
            return false;
        }
    }

    lockdownd_start_service(lckd, "com.apple.mobile.mobile_image_mounter",
                            &service);

    if (!service || service->port == 0) {
        qDebug() << "ERROR: Could not start mobile_image_mounter service!";
        return false;
    }

    if (mobile_image_mounter_new(device, service, &mim) !=
        MOBILE_IMAGE_MOUNTER_E_SUCCESS) {
        qDebug() << "ERROR: Could not connect to mobile_image_mounter!";
        return false;
    }

    // if (service)
    // {
    //     lockdownd_service_descriptor_free(service);
    //     service = NULL;
    // }

    struct stat fst;
    if (disk_image_upload_type == DISK_IMAGE_UPLOAD_TYPE_AFC) {
        if (!service || !service->port) {
            qDebug() << "Could not start com.apple.afc!";
            return false;
        }
        if (!afc) {
            qDebug() << "Could not connect to AFC!";
            return false;
        }
        if (service) {
            // lockdownd_service_descriptor_free(service);
            service = NULL;
        }
    }
    if (stat(image_path, &fst) != 0) {
        qDebug() << "ERROR: stat:" << image_path << ":" << strerror(errno);
        return false;
    }
    image_size = fst.st_size;
    if (device_version < IDEVICE_DEVICE_VERSION(17, 0, 0) &&
        stat(image_sig_path, &fst) != 0) {
        qDebug() << "ERROR: stat:" << image_sig_path << ":" << strerror(errno);
        return false;
    }

    // lockdownd_client_free(lckd);
    // lckd = NULL;

    mobile_image_mounter_error_t err = MOBILE_IMAGE_MOUNTER_E_UNKNOWN_ERROR;
    plist_t result = NULL;

    // if (cmd == CMD_LIST)
    // {
    //     /* list mounts mode */
    //     if (!imagetype)
    //     {
    //         if (device_version < IDEVICE_DEVICE_VERSION(17, 0, 0))
    //         {
    //             imagetype = "Developer";
    //         }
    //         else
    //         {
    //             imagetype = "Personalized";
    //         }
    //     }
    //     err = mobile_image_mounter_lookup_image(mim, imagetype, &result);
    //     if (err == MOBILE_IMAGE_MOUNTER_E_SUCCESS)
    //     {
    //         res = 0;
    //         plist_write_to_stream(result, stdout, (xml_mode) ?
    //         PLIST_FORMAT_XML : PLIST_FORMAT_LIMD, 0);
    //     }
    //     else
    //     {
    //         printf("Error: lookup_image returned %d\n", err);
    //     }
    // }

    unsigned char *sig = NULL;
    size_t sig_length = 0;
    FILE *f;
    // struct stat fst;
    plist_t mount_options = NULL;

    if (device_version < IDEVICE_DEVICE_VERSION(17, 0, 0)) {
        f = fopen(image_sig_path, "rb");
        if (!f) {
            qDebug() << "Error opening signature file" << image_sig_path << ":"
                     << strerror(errno);
            return false;
        }
        if (fstat(fileno(f), &fst) != 0) {
            qDebug() << "Error: fstat:" << strerror(errno);
            return false;
        }
        sig = (unsigned char *)malloc(fst.st_size);
        sig_length = fread(sig, 1, fst.st_size, f);
        fclose(f);
        if (sig_length == 0) {
            qDebug() << "Could not read signature from file" << image_sig_path;
            return false;
        }

        f = fopen(image_path, "rb");
        if (!f) {
            qDebug() << "Error opening image file" << image_path << ":"
                     << strerror(errno);
            return false;
        }
    } else {
        if (stat(image_path, &fst) != 0) {
            qDebug() << "Error: stat:" << image_path << ":" << strerror(errno);
            return false;
        }
        if (!S_ISDIR(fst.st_mode)) {
            qDebug() << "Error: Personalized Disk Image mount expects a "
                        "directory as image path.";
            return false;
        }
        char *build_manifest_path =
            string_build_path(image_path, "BuildManifest.plist", NULL);
        plist_t build_manifest = NULL;
        if (plist_read_from_file(build_manifest_path, &build_manifest, NULL) !=
            0) {
            free(build_manifest_path);
            build_manifest_path = string_build_path(
                image_path, "Restore", "BuildManifest.plist", NULL);
            if (plist_read_from_file(build_manifest_path, &build_manifest,
                                     NULL) == 0) {
                char *image_path_new =
                    string_build_path(image_path, "Restore", NULL);
                free(image_path);
                image_path = image_path_new;
            }
        }
        if (!build_manifest) {
            qDebug() << "Error: Could not locate BuildManifest.plist inside "
                        "given disk image path!";
            return false;
        }

        plist_t identifiers = NULL;
        mobile_image_mounter_error_t merr =
            mobile_image_mounter_query_personalization_identifiers(
                mim, NULL, &identifiers);
        if (merr != MOBILE_IMAGE_MOUNTER_E_SUCCESS) {
            qDebug() << "Failed to query personalization identifiers:" << merr;
            return false;
        }

        unsigned int board_id = plist_dict_get_uint(identifiers, "BoardId");
        unsigned int chip_id = plist_dict_get_uint(identifiers, "ChipID");

        plist_t build_identities =
            plist_dict_get_item(build_manifest, "BuildIdentities");
        plist_array_iter iter;
        plist_array_new_iter(build_identities, &iter);
        plist_t item = NULL;
        plist_t build_identity = NULL;
        do {
            plist_array_next_item(build_identities, iter, &item);
            if (!item) {
                break;
            }
            unsigned int bi_board_id =
                (unsigned int)plist_dict_get_uint(item, "ApBoardID");
            unsigned int bi_chip_id =
                (unsigned int)plist_dict_get_uint(item, "ApChipID");
            if (bi_chip_id == chip_id && bi_board_id == board_id) {
                build_identity = item;
                break;
            }
        } while (item);
        plist_mem_free(iter);
        if (!build_identity) {
            qDebug() << "Error: The given disk image is not compatible with "
                        "the current device.";
            return false;
        }
        plist_t p_tc_path =
            plist_access_path(build_identity, 4, "Manifest",
                              "LoadableTrustCache", "Info", "Path");
        if (!p_tc_path) {
            qDebug() << "Error: Could not determine path for trust cache!";
            return false;
        }
        plist_t p_dmg_path = plist_access_path(
            build_identity, 4, "Manifest", "PersonalizedDMG", "Info", "Path");
        if (!p_dmg_path) {
            qDebug() << "Error: Could not determine path for disk image!";
            return false;
        }
        char *tc_path = string_build_path(
            image_path, plist_get_string_ptr(p_tc_path, NULL), NULL);
        unsigned char *trust_cache = NULL;
        uint64_t trust_cache_size = 0;
        if (!buffer_read_from_filename(tc_path, (char **)&trust_cache,
                                       &trust_cache_size)) {
            qDebug() << "Error: Trust cache does not exist at" << tc_path
                     << "!";
            return false;
        }
        mount_options = plist_new_dict();
        plist_dict_set_item(
            mount_options, "ImageTrustCache",
            plist_new_data((char *)trust_cache, trust_cache_size));
        free(trust_cache);
        char *dmg_path = string_build_path(
            image_path, plist_get_string_ptr(p_dmg_path, NULL), NULL);
        free(image_path);
        image_path = dmg_path;
        f = fopen(image_path, "rb");
        if (!f) {
            qDebug() << "Error opening image file" << image_path << ":"
                     << strerror(errno);
            return false;
        }

        unsigned char buf[8192];
        unsigned char sha384_digest[48];
        sha384_context ctx;
        sha384_init(&ctx);
        fstat(fileno(f), &fst);
        image_size = fst.st_size;
        while (!feof(f)) {
            ssize_t fr = fread(buf, 1, sizeof(buf), f);
            if (fr <= 0) {
                break;
            }
            sha384_update(&ctx, buf, fr);
        }
        rewind(f);
        sha384_final(&ctx, sha384_digest);
        unsigned char *manifest = NULL;
        unsigned int manifest_size = 0;
        /* check if the device already has a personalization manifest for this
         * image */
        if (mobile_image_mounter_query_personalization_manifest(
                mim, "DeveloperDiskImage", sha384_digest, sizeof(sha384_digest),
                &manifest, &manifest_size) == MOBILE_IMAGE_MOUNTER_E_SUCCESS) {
            qDebug() << "Using existing personalization manifest from device.";
        } else {
            /* we need to re-connect in this case */
            mobile_image_mounter_free(mim);
            mim = NULL;
            if (mobile_image_mounter_start_service(device, &mim, TOOL_NAME) !=
                MOBILE_IMAGE_MOUNTER_E_SUCCESS) {
                return false;
            }
            qDebug() << "No personalization manifest, requesting from TSS...";
            unsigned char *nonce = NULL;
            unsigned int nonce_size = 0;

            /* create new TSS request and fill parameters */
            plist_t request = tss_request_new(NULL);
            plist_t params = plist_new_dict();
            tss_parameters_add_from_manifest(params, build_identity, 1);

            /* copy all `Ap,*` items from identifiers */
            plist_dict_iter di = NULL;
            plist_dict_new_iter(identifiers, &di);
            plist_t node = NULL;
            do {
                char *key = NULL;
                plist_dict_next_item(identifiers, di, &key, &node);
                if (node) {
                    if (!strncmp(key, "Ap,", 3)) {
                        plist_dict_set_item(request, key, plist_copy(node));
                    }
                }
                free(key);
            } while (node);
            plist_mem_free(di);

            plist_dict_copy_uint(params, identifiers, "ApECID", "UniqueChipID");
            plist_dict_set_item(params, "ApProductionMode", plist_new_bool(1));
            plist_dict_set_item(params, "ApSecurityMode", plist_new_bool(1));
            plist_dict_set_item(params, "ApSupportsImg4", plist_new_bool(1));

            /* query nonce from image mounter service */
            merr = mobile_image_mounter_query_nonce(mim, "DeveloperDiskImage",
                                                    &nonce, &nonce_size);
            if (merr == MOBILE_IMAGE_MOUNTER_E_SUCCESS) {
                plist_dict_set_item(params, "ApNonce",
                                    plist_new_data((char *)nonce, nonce_size));
            } else {
                qDebug()
                    << "ERROR: Failed to query nonce for developer disk image:"
                    << merr;
                return false;
            }
            mobile_image_mounter_free(mim);
            mim = NULL;

            plist_dict_set_item(
                params, "ApSepNonce",
                plist_new_data("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                               "\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                               20));
            plist_dict_set_item(params, "UID_MODE", plist_new_bool(0));
            tss_request_add_ap_tags(request, params, NULL);
            tss_request_add_common_tags(request, params, NULL);
            tss_request_add_ap_img4_tags(request, params);
            plist_free(params);

            /* request IM4M from TSS */
            plist_t response = tss_request_send(request, NULL);
            plist_free(request);

            plist_t p_manifest = plist_dict_get_item(response, "ApImg4Ticket");
            if (!PLIST_IS_DATA(p_manifest)) {
                qDebug() << "Failed to get Img4Ticket";
                return false;
            }

            uint64_t m4m_len = 0;
            plist_get_data_val(p_manifest, (char **)&manifest, &m4m_len);
            manifest_size = m4m_len;
            plist_free(response);
            qDebug() << "Done.";
        }
        sig = manifest;
        sig_length = manifest_size;

        imagetype = "Personalized";
    }

    char *targetname = NULL;
    if (asprintf(&targetname, "%s/%s", PKG_PATH, "staging.dimage") < 0) {
        qDebug() << "Out of memory!?";
        return false;
    }
    char *mountname = NULL;
    if (asprintf(&mountname, "%s/%s", PATH_PREFIX, targetname) < 0) {
        qDebug() << "Out of memory!?";
        return false;
    }

    if (!imagetype) {
        imagetype = "Developer";
    }

    if (!mim) {
        if (mobile_image_mounter_start_service(device, &mim, TOOL_NAME) !=
            MOBILE_IMAGE_MOUNTER_E_SUCCESS) {
            return false;
        }
    }

    switch (disk_image_upload_type) {
    case DISK_IMAGE_UPLOAD_TYPE_UPLOAD_IMAGE:
        qDebug() << "Uploading" << image_path;
        err = mobile_image_mounter_upload_image(mim, imagetype, image_size, sig,
                                                sig_length, mim_upload_cb, f);
        break;
    case DISK_IMAGE_UPLOAD_TYPE_AFC:
    default:
        qDebug() << "Uploading" << image_path << "--> afc:///" << targetname;
        plist_t fileinfo = NULL;
        if (afc_get_file_info_plist(afc, PKG_PATH, &fileinfo) !=
            AFC_E_SUCCESS) {
            if (afc_make_directory(afc, PKG_PATH) != AFC_E_SUCCESS) {
                qDebug() << "WARNING: Could not create directory" << PKG_PATH
                         << "on device!";
            }
        }
        plist_free(fileinfo);

        uint64_t af = 0;
        if ((afc_file_open(afc, targetname, AFC_FOPEN_WRONLY, &af) !=
             AFC_E_SUCCESS) ||
            !af) {
            fclose(f);
            qDebug() << "afc_file_open on" << targetname << "failed!";
            return false;
        }

        char buf[8192];
        size_t amount = 0;
        do {
            amount = fread(buf, 1, sizeof(buf), f);
            if (amount > 0) {
                uint32_t written, total = 0;
                while (total < amount) {
                    written = 0;
                    if (afc_file_write(afc, af, buf + total, amount - total,
                                       &written) != AFC_E_SUCCESS) {
                        qDebug() << "AFC Write error!";
                        break;
                    }
                    total += written;
                }
                if (total != amount) {
                    qDebug() << "Error: wrote only" << total << "of"
                             << (unsigned int)amount;
                    afc_file_close(afc, af);
                    fclose(f);
                    return false;
                }
            }
        } while (amount > 0);

        afc_file_close(afc, af);
        break;
    }

    fclose(f);

    if (err != MOBILE_IMAGE_MOUNTER_E_SUCCESS) {
        if (err == MOBILE_IMAGE_MOUNTER_E_DEVICE_LOCKED) {
            qDebug() << "ERROR: Device is locked, can't mount. Unlock device "
                        "and try again.";
        } else {
            qDebug() << "ERROR: Unknown error occurred, can't mount.";
        }
        return false;
    }
    qDebug() << "done.";

    qDebug() << "Mounting...";
    err = mobile_image_mounter_mount_image_with_options(
        mim, mountname, sig, sig_length, imagetype, mount_options, &result);
    if (err == MOBILE_IMAGE_MOUNTER_E_SUCCESS) {
        if (result) {
            plist_t node = plist_dict_get_item(result, "Status");
            if (node) {
                char *status = NULL;
                plist_get_string_val(node, &status);
                if (status) {
                    if (!strcmp(status, "Complete")) {
                        qDebug() << "Done.";
                        res = 0;
                    } else {
                        qDebug() << "unexpected status value:";
                        plist_write_to_stream(result, stdout,
                                              (xml_mode) ? PLIST_FORMAT_XML
                                                         : PLIST_FORMAT_LIMD,
                                              (plist_write_options_t)0);
                    }
                    free(status);
                } else {
                    qDebug() << "unexpected result:";
                    plist_write_to_stream(result, stdout,
                                          (xml_mode) ? PLIST_FORMAT_XML
                                                     : PLIST_FORMAT_LIMD,
                                          (plist_write_options_t)0);
                }
            }
            node = plist_dict_get_item(result, "Error");
            if (node) {
                char *error = NULL;
                plist_get_string_val(node, &error);
                if (error) {
                    qDebug() << "Error:" << error;
                    free(error);
                } else {
                    qDebug() << "unexpected result:";
                    plist_write_to_stream(result, stdout,
                                          (xml_mode) ? PLIST_FORMAT_XML
                                                     : PLIST_FORMAT_LIMD,
                                          (plist_write_options_t)0);
                }
                node = plist_dict_get_item(result, "DetailedError");
                if (node) {
                    qDebug()
                        << "DetailedError:" << plist_get_string_ptr(node, NULL);
                }
            } else {
                plist_write_to_stream(result, stdout,
                                      (xml_mode) ? PLIST_FORMAT_XML
                                                 : PLIST_FORMAT_LIMD,
                                      (plist_write_options_t)0);
            }
        }
    } else {
        qDebug() << "Error: mount_image returned" << err;
    }

    if (result) {
        plist_free(result);
    }
    // error_out:
    //     /* perform hangup command */
    //     mobile_image_mounter_hangup(mim);
    //     /* free client */
    //     mobile_image_mounter_free(mim);

    // leave:
    //     if (afc)
    //     {
    //         afc_client_free(afc);
    //     }
    //     if (lckd)
    //     {
    //         lockdownd_client_free(lckd);
    //     }
    //     idevice_free(device);

    //     if (image_path)
    //         free(image_path);
    //     if (image_sig_path)
    //         free(image_sig_path);

    //     return res;
    return true;
}

// int main(){return 0;}