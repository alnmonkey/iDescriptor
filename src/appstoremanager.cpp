#include "appstoremanager.h"
#include "libipatool-go.h"
#include <QApplication>
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLineEdit>
#include <QtConcurrent/QtConcurrent>

// 2FA callback for login
static char *getAuthCodeCallback()
{
    static QByteArray buffer;
    QString code;
    QMetaObject::invokeMethod(
        qApp,
        [&]() {
            bool ok;
            code = QInputDialog::getText(
                nullptr, "Two-Factor Authentication",
                "Enter the 2FA code:", QLineEdit::Normal, QString(), &ok);
        },
        Qt::BlockingQueuedConnection);

    if (code.isEmpty()) {
        return nullptr;
    }
    buffer = code.toUtf8();
    return buffer.data();
}

AppStoreManager *AppStoreManager::sharedInstance()
{
    static AppStoreManager instance;
    return instance.m_initialized ? &instance : nullptr;
}

AppStoreManager::AppStoreManager(QObject *parent)
    : QObject(parent), m_initialized(false)
{
    m_initialized = initialize();
}

bool AppStoreManager::initialize()
{
    int result = IpaToolInitialize();
    if (result != 0) {
        qDebug() << "IpaToolInitialize failed with error code:" << result;
        return false;
    }
    qDebug() << "IpaToolInitialize succeeded";
    return true;
}

QJsonObject AppStoreManager::getAccountInfo()
{
    if (!m_initialized) {
        return QJsonObject();
    }

    char *accountInfoCStr = IpaToolGetAccountInfo();
    if (!accountInfoCStr) {
        return QJsonObject();
    }

    QString jsonAccountInfo(accountInfoCStr);
    free(accountInfoCStr);

    QJsonParseError parseError;
    QJsonDocument doc =
        QJsonDocument::fromJson(jsonAccountInfo.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qDebug() << "JSON parse error:" << parseError.errorString();
        return QJsonObject();
    }

    return doc.object();
}

void AppStoreManager::loginWithCallback(
    const QString &email, const QString &password,
    std::function<void(bool success, const QJsonObject &accountInfo)> callback)
{
    if (!m_initialized) {
        callback(false, QJsonObject());
        return;
    }

    QFuture<int> future = QtConcurrent::run([email, password]() {
        return IpaToolLoginWithCallback(email.toUtf8().data(),
                                        password.toUtf8().data(),
                                        getAuthCodeCallback);
    });

    QFutureWatcher<int> *watcher = new QFutureWatcher<int>(this);
    connect(watcher, &QFutureWatcher<int>::finished, this,
            [this, watcher, callback]() {
                int result = watcher->result();
                watcher->deleteLater();

                bool success = (result == 0);
                QJsonObject accountInfo;

                if (success) {
                    accountInfo = getAccountInfo();
                    emit loginSuccessful(accountInfo);
                }

                callback(success, accountInfo);
            });
    watcher->setFuture(future);
}

void AppStoreManager::revokeCredentials()
{
    if (!m_initialized) {
        return;
    }

    IpaToolRevokeCredentials();
    // todo: should we ?
    // could be problematic if user logs in using ipatool
    // emit loggedOut(getAccountInfo());
    emit loggedOut(QJsonObject());
}

void AppStoreManager::searchApps(
    const QString &searchTerm, int limit,
    std::function<void(bool success, const QString &results)> callback)
{
    if (!m_initialized) {
        callback(false, QString());
        return;
    }

    QFuture<QString> future =
        QtConcurrent::run([searchTerm, limit]() -> QString {
            char *resultsCStr =
                IpaToolSearch(searchTerm.toUtf8().data(), limit);
            if (!resultsCStr) {
                return QString();
            }
            QString results(resultsCStr);
            free(resultsCStr);
            return results;
        });

    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this,
            [watcher, callback]() {
                QString results = watcher->result();
                watcher->deleteLater();
                callback(!results.isEmpty(), results);
            });
    watcher->setFuture(future);
}

void AppStoreManager::downloadApp(
    const QString &bundleId, const QString &outputDir,
    const QString &externalVersionId, bool acquireLicense,
    std::function<void(int result)> callback,
    std::function<void(long long current, long long total)> progressCallback)
{
    if (!m_initialized) {
        callback(-1);
        return;
    }

    // Create a wrapper for the progress callback
    void *progressUserData = nullptr;
    void (*cProgressCallback)(long long, long long, void *) = nullptr;

    if (progressCallback) {
        // Store the callback in a way that can be accessed from C
        auto *callbackPtr =
            new std::function<void(long long, long long)>(progressCallback);
        progressUserData = callbackPtr;

        cProgressCallback = [](long long current, long long total,
                               void *userData) {
            auto *cb = static_cast<std::function<void(long long, long long)> *>(
                userData);
            QMetaObject::invokeMethod(
                qApp, [cb, current, total]() { (*cb)(current, total); },
                Qt::QueuedConnection);
        };
    }

    QFuture<int> future = QtConcurrent::run(
        [bundleId, outputDir, externalVersionId, acquireLicense,
         cProgressCallback, progressUserData]() {
            int result = IpaToolDownloadApp(
                bundleId.toUtf8().data(), outputDir.toUtf8().data(),
                externalVersionId.toUtf8().data(), acquireLicense,
                cProgressCallback, progressUserData);
            return result;
        });

    QFutureWatcher<int> *watcher = new QFutureWatcher<int>(this);
    connect(
        watcher, &QFutureWatcher<int>::finished, this,
        [watcher, callback, progressUserData]() {
            int result = watcher->result();
            watcher->deleteLater();

            // Clean up progress callback if it was allocated
            if (progressUserData) {
                delete static_cast<std::function<void(long long, long long)> *>(
                    progressUserData);
            }

            callback(result);
        });
    watcher->setFuture(future);
}