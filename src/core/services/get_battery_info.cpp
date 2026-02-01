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
#include "plist/plist.h"
#include <QDebug>
#include <string>

// FIXME: return bool
void get_battery_info(DiagnosticsRelay *diag_client, plist_t &diagnostics)
{
    qDebug() << "Fetching battery info via DiagnosticsRelay";
    auto ioreg_result = diag_client->ioregistry(
        IdeviceFFI::None,                                // current_plane
        IdeviceFFI::None,                                // entry_name
        IdeviceFFI::Some(std::string("IOPMPowerSource")) // entry_class
    );

    if (!ioreg_result.is_ok()) {
        qDebug() << "Failed to query IORegistry:"
                 << ioreg_result.unwrap_err().message.c_str();
        return;
    }

    auto plist_opt = std::move(ioreg_result).unwrap();
    if (plist_opt.is_some()) {
        diagnostics = std::move(plist_opt).unwrap();
    }
}