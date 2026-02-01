/*
 * iDescriptor: A free and open-source idevice management tool.
 *
 * Copyright (C) 2025 Uncore <https://github.com/uncor3>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../../iDescriptor.h"

#include <string>

bool detect_jailbroken(AfcClientHandle *afc)
{
    char **dirs = NULL;
    size_t count = 0;
    if (!afc_list_directory(afc, (std::string(POSSIBLE_ROOT) + "bin").c_str(),
                            &dirs, &count)) {
        free_directory_listing(dirs, count);
    }

    return count > 0;
}