#include "../../iDescriptor.h"
#include <string>

std::string parse_product_type(const std::string &productType)
{
    // Check if the product type is in the DEVICE_MAP
    auto it = DEVICE_MAP.find(productType);
    if (it != DEVICE_MAP.end()) {
        return it->second; // Return the marketing name if found
    }
    return "Unknown Device"; // Return a default value if not found
}