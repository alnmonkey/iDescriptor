#include "../../iDescriptor.h"

void mounted_image_info_free(MountedImageInfo &info)
{
    if (info.err) {
        idevice_error_free(info.err);
    }
    if (info.signature) {
        idevice_data_free(info.signature, info.signature_len);
    }
}