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

#ifndef JAILBROKENWIDGET_H
#define JAILBROKENWIDGET_H

#ifdef __linux__
#include "core/services/avahi/avahi_service.h"
#else
#include "core/services/dnssd/dnssd_service.h"
#endif

#include "iDescriptor.h"
#include "opensshterminalwidget.h"
#include <QAbstractButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class ClickableWidget;

// Define the struct here so it's available to the class declaration
struct JailbreakToolInfo {
    QString title;
    QString description;
    QString iconPath;
    bool enabled = true;
};

class JailbrokenWidget : public QWidget
{
    Q_OBJECT

public:
    JailbrokenWidget(QWidget *parent = nullptr);
    ~JailbrokenWidget();

private slots:
private:
    // Helper function to create a tool widget
    ClickableWidget *createJailbreakTool(const JailbreakToolInfo &info);
    OpenSSHTerminalWidget *m_sshTerminalWidget = nullptr;
};

#endif // JAILBROKENWIDGET_H