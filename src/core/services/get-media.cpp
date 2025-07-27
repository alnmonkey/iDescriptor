#include "./get-media.h"
#include "../../iDescriptor.h"
#include <iostream>
#include <libimobiledevice/afc.h>
#include <libimobiledevice/lockdown.h>
#include <string.h>

MediaFileTree getMediaFileTree(afc_client_t afcClient,
                               lockdownd_service_descriptor_t lockdownService,

                               const std::string &path = "/")
{

    MediaFileTree result;
    result.currentPath = path;

    if (afcClient == nullptr) {
        qDebug() << "AFC client is not initialized in getMediaFileTree";
    }

    if (lockdownService == nullptr) {
        qDebug() << "Lockdown service is not initialized in getMediaFileTree";
    }

    char **dirs = NULL;
    if (afc_read_directory(afcClient, path.c_str(), &dirs) != AFC_E_SUCCESS) {
        // afc_client_free(afcClient);
        // lockdownd_service_descriptor_free(lockdownService);
        result.success = false;
        return result;
    }

    for (int i = 0; dirs[i]; i++) {
        std::string entryName = dirs[i];
        if (entryName == "." || entryName == "..")
            continue;

        // Determine if entry is a directory
        char **info = NULL;
        std::string fullPath = path;
        if (fullPath.back() != '/')
            fullPath += "/";
        fullPath += entryName;

        bool isDir = false;
        if (afc_get_file_info(afcClient, fullPath.c_str(), &info) ==
                AFC_E_SUCCESS &&
            info) {
            for (int j = 0; info[j]; j += 2) {
                if (strcmp(info[j], "st_ifmt") == 0) {
                    if (strcmp(info[j + 1], "S_IFDIR") == 0)
                        isDir = true;
                    break;
                }
            }
            afc_dictionary_free(info);
        }
        result.entries.push_back({entryName, isDir});
    }
    // TODO : Freed when device is disconnected
    // afc_client_free(afc);
    // lockdownd_service_descriptor_free(service);
    result.success = true;
    return result;
}