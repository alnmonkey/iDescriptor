#include "libirecovery.h"
#include <string>

std::string parse_recovery_mode(irecv_mode productType)
{
    switch (productType) {
    case irecv_mode::IRECV_K_RECOVERY_MODE_1:
    case irecv_mode::IRECV_K_RECOVERY_MODE_2:
    case irecv_mode::IRECV_K_RECOVERY_MODE_3:
    case irecv_mode::IRECV_K_RECOVERY_MODE_4:
        return "Recovery Mode";
    case irecv_mode::IRECV_K_WTF_MODE:
        return "WTF Mode";
    case irecv_mode::IRECV_K_DFU_MODE:
    case irecv_mode::IRECV_K_PORT_DFU_MODE:
        return "DFU Mode";
    default:
        return "Unknown Mode";
    }
}
