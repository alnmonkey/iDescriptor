#include <regex>
#include <stdexcept>
#include <string>

struct ProductTypeVersion {
    int major;
    int minor;

    ProductTypeVersion(int maj = 0, int min = 0) : major(maj), minor(min) {}

    // Compare two product type versions
    // Returns: -1 if this < other, 0 if equal, 1 if this > other
    int compareTo(const ProductTypeVersion &other) const
    {
        if (major != other.major) {
            return major < other.major ? -1 : 1;
        }
        if (minor != other.minor) {
            return minor < other.minor ? -1 : 1;
        }
        return 0; // Equal
    }

    bool operator<(const ProductTypeVersion &other) const
    {
        return compareTo(other) < 0;
    }

    bool operator==(const ProductTypeVersion &other) const
    {
        return compareTo(other) == 0;
    }

    bool operator>(const ProductTypeVersion &other) const
    {
        return compareTo(other) > 0;
    }
};

namespace iDescriptor
{
class Utils
{
private:
    // Extract version numbers from iPhone product type string
    // Example: "iPhone8,1" -> ProductTypeVersion{8, 1}
    static ProductTypeVersion
    extractProductTypeVersion(const std::string &productType)
    {
        // Regex to match iPhone followed by major,minor numbers
        std::regex pattern(R"(iPhone(\d+),(\d+))");
        std::smatch matches;

        if (std::regex_search(productType, matches, pattern)) {
            if (matches.size() >= 3) {
                try {
                    int major = std::stoi(matches[1].str());
                    int minor = std::stoi(matches[2].str());
                    return ProductTypeVersion(major, minor);
                } catch (const std::invalid_argument &e) {
                    throw std::invalid_argument(
                        "Invalid numeric values in product type: " +
                        productType);
                }
            }
        }

        throw std::invalid_argument("Invalid iPhone product type format: " +
                                    productType);
    }

    static bool compare_product_type(const std::string &productType,
                                     const std::string &otherProductType)
    {
        try {
            ProductTypeVersion version1 =
                extractProductTypeVersion(productType);
            ProductTypeVersion version2 =
                extractProductTypeVersion(otherProductType);

            // Return true if productType is newer/higher than otherProductType
            return version1 > version2;
        } catch (const std::exception &e) {
            // Handle invalid product types - you might want to log this
            return false;
        }
    }

public:
    static bool isProductTypeNewer(const std::string &productType,
                                   const std::string &otherProductType)
    {
        return compare_product_type(productType, otherProductType);
    }

    static QString formatSize(uint64_t bytes)
    {
        const char *units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = bytes;

        while (size >= 1024 && unitIndex < 4) {
            size /= 1024;
            unitIndex++;
        }

        return QString("%1 %2")
            .arg(QString::number(size, 'f', 1))
            .arg(units[unitIndex]);
    };

    static bool isVideoFile(const QString &fileName)
    {
        /* known iPhone video file extensions */
        return fileName.endsWith(".MOV", Qt::CaseInsensitive) ||
               fileName.endsWith(".MP4", Qt::CaseInsensitive) ||
               fileName.endsWith(".M4V", Qt::CaseInsensitive);
    }
};
} // namespace iDescriptor