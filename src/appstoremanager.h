#ifndef APPSTOREMANAGER_H
#define APPSTOREMANAGER_H

#include <QJsonObject>
#include <QObject>
#include <functional>

class AppStoreManager : public QObject
{
    Q_OBJECT

public:
    static AppStoreManager *sharedInstance();

    // Account management
    QJsonObject getAccountInfo();
    void loginWithCallback(
        const QString &email, const QString &password,
        std::function<void(bool success, const QJsonObject &accountInfo)>
            callback);
    void revokeCredentials();

    // App operations
    void searchApps(
        const QString &searchTerm, int limit,
        std::function<void(bool success, const QString &results)> callback);
    void downloadApp(const QString &bundleId, const QString &outputDir,
                     const QString &externalVersionId, bool acquireLicense,
                     std::function<void(int result)> callback,
                     std::function<void(long long current, long long total)>
                         progressCallback = nullptr);

signals:
    void loginSuccessful(const QJsonObject &accountInfo);
    void loggedOut(const QJsonObject &accountInfo);

private:
    AppStoreManager(QObject *parent = nullptr);
    ~AppStoreManager() = default;

    AppStoreManager(const AppStoreManager &) = delete;
    AppStoreManager &operator=(const AppStoreManager &) = delete;

    bool m_initialized;
    bool initialize();
};

#endif // APPSTOREMANAGER_H