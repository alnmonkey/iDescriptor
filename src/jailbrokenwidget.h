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

#include "appcontext.h"
#include "responsiveqlabel.h"
#include "sshterminaltool.h"

#include "iDescriptor-ui.h"
#include "iDescriptor.h"
#include <QApplication>
#include <QButtonGroup>
#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class ClickableWidget;

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
    ClickableWidget *createJailbreakTool(const JailbreakToolInfo &info);
    SSHTerminalTool *m_sshTerminalWidget = nullptr;
};

#endif // JAILBROKENWIDGET_H