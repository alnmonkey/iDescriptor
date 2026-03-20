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

#ifndef SSHTERMINALWIDGET_H
#define SSHTERMINALWIDGET_H

#include "qprocessindicator.h"
#include "settingsmanager.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include <libssh/libssh.h>
#include <qtermwidget6/qtermwidget.h>
#include <unistd.h>

class QTermWidget;
class QProcessIndicator;

enum class ConnectionType { Wired, Wireless };

struct ConnectionInfo {
    ConnectionType type;
    QString deviceName;
    QString deviceUdid;  // For wired devices
    QString hostAddress; // For wireless devices
    uint16_t port;
};

enum class TerminalState { Loading, Error, Connected };

class SSHTerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SSHTerminalWidget(const ConnectionInfo &connectionInfo,
                               QWidget *parent = nullptr);
    ~SSHTerminalWidget();

private slots:
    void onRetryClicked();
    void checkSshData();

private:
    void setupUI();
    void setupLoadingState();
    void setupErrorState();
    void setupActionState();

    void setState(TerminalState state);
    void showError(const QString &errorMessage);

    void initializeConnection();
    void initWiredDevice();
    void initWirelessDevice();
    void startSSH(const QString &host, uint16_t port);
    void disconnectSSH();
    void connectLibsshToTerminal();
    void cleanup();

    // UI Components
    QStackedWidget *m_stackedWidget;

    // Loading state
    QWidget *m_loadingWidget;
    QProcessIndicator *m_loadingIndicator;
    QLabel *m_loadingLabel;

    // Error state
    QWidget *m_errorWidget;
    QLabel *m_errorLabel;
    QPushButton *m_retryButton;

    // Action state (connected terminal)
    QWidget *m_actionWidget;
    QTermWidget *m_terminal;

    // Connection data
    ConnectionInfo m_connectionInfo;

    // SSH components
    ssh_session m_sshSession;
    ssh_channel m_sshChannel;
    QTimer *m_sshTimer;
    QProcess *m_iproxyProcess;

    // State tracking
    bool m_sshConnected;
    bool m_isInitialized;
    TerminalState m_currentState;
};

#endif // SSHTERMINALWIDGET_H