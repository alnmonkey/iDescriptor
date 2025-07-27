/*
 * idevicescreenshot.c
 * Gets a screenshot from a device
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define TOOL_NAME "idevicescreenshot"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#ifndef _WIN32
#include <signal.h>
#endif

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/screenshotr.h>



static void get_image_filename(char *imgdata, char **filename)
{
	// If the provided filename already has an extension, use it as is.
	if (*filename) {
		char *last_dot = strrchr(*filename, '.');
		if (last_dot && !strchr(last_dot, '/')) {
			return;
		}
	}

	// Find the appropriate file extension for the filename.
	const char *fileext = NULL;
	if (memcmp(imgdata, "\x89PNG", 4) == 0) {
		fileext = ".png";
	} else if (memcmp(imgdata, "MM\x00*", 4) == 0) {
		fileext = ".tiff";
	} else {
		printf("WARNING: screenshot data has unexpected image format.\n");
		fileext = ".dat";
	}

	// If a filename without an extension is provided, append the extension.
	// Otherwise, generate a filename based on the current time.
	char *basename = NULL;
	if (*filename) {
		basename = (char*)malloc(strlen(*filename) + 1);
		strcpy(basename, *filename);
		free(*filename);
		*filename = NULL;
	} else {
		time_t now = time(NULL);
		basename = (char*)malloc(32);
		strftime(basename, 31, "screenshot-%Y-%m-%d-%H-%M-%S", gmtime(&now));
	}

	// Ensure the filename is unique on disk.
	char *unique_filename = (char*)malloc(strlen(basename) + strlen(fileext) + 7);
	sprintf(unique_filename, "%s%s", basename, fileext);
	int i;
	for (i = 2; i < (1 << 16); i++) {
		if (access(unique_filename, F_OK) == -1) {
			*filename = unique_filename;
			break;
		}
		sprintf(unique_filename, "%s-%d%s", basename, i, fileext);
	}
	if (!*filename) {
		free(unique_filename);
	}
	free(basename);
}

int take_screenshot(idevice_t device, lockdownd_service_descriptor_t service) {
    screenshotr_client_t *shotr = nullptr;
    char *filename = NULL;
    int result=1;
    try
    {
        /* code */
        
        if (screenshotr_client_new(device, service, &shotr) != SCREENSHOTR_E_SUCCESS) {
                printf("Could not connect to screenshotr!\n");
            } else {
                char *imgdata = NULL;
                uint64_t imgsize = 0;
                if (screenshotr_take_screenshot(*shotr, &imgdata, &imgsize) == SCREENSHOTR_E_SUCCESS) {
                    get_image_filename(imgdata, &filename);
                    if (!filename) {
                        printf("FATAL: Could not find a unique filename!\n");
                    } else {
                        FILE *f = fopen(filename, "wb");
                        if (f) {
                            if (fwrite(imgdata, 1, (size_t)imgsize, f) == (size_t)imgsize) {
                                printf("Screenshot saved to %s\n", filename);
                                result = 0;
                            } else {
                                printf("Could not save screenshot to file %s!\n", filename);
                            }
                            fclose(f);
                        } else {
                            printf("Could not open %s for writing: %s\n", filename, strerror(errno));
                        }
                    }
                } else {
                    printf("Could not get screenshot!\n");
                }
                screenshotr_client_free(shotr);
            }
    
        return result;
    }
    catch(const std::exception& e)
    {
        return result;
        std::cerr << e.what() << '\n';
    }
}