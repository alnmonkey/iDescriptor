#ifndef HEARTBEATTHREAD_H
#define HEARTBEATTHREAD_H
#include <QDebug>
#include <QThread>
#include <idevice++/bindings.hpp>
#include <idevice++/core_device_proxy.hpp>
#include <idevice++/dvt/remote_server.hpp>
#include <idevice++/dvt/screenshot.hpp>
#include <idevice++/ffi.hpp>
#include <idevice++/heartbeat.hpp>
#include <idevice++/provider.hpp>
#include <idevice++/readwrite.hpp>
#include <idevice++/rsd.hpp>
#include <idevice++/usbmuxd.hpp>

using namespace IdeviceFFI;

class HeartBeatThread : public QThread
{
    Q_OBJECT
public:
    HeartBeatThread(HeartbeatClientHandle *heartbeat, QObject *parent = nullptr)
        : QThread(parent), m_hb(Heartbeat::adopt(heartbeat))
    {
    }

    void run() override
    {
        qDebug() << "Heartbeat thread started";
        try {
            // Start with initial interval (15 seconds as per the tool example)
            u_int64_t interval = 15;

            while (!isInterruptionRequested()) {
                // 1. Wait for Marco with current interval
                Result result = m_hb.get_marco(interval);
                if (result.is_err()) {
                    qDebug()
                        << "Failed to get marco:"
                        << QString::fromStdString(result.unwrap_err().message);
                    break;
                }

                // 2. Get the new interval from device
                interval = result.unwrap();
                qDebug() << "Received marco, new interval:" << interval;

                // 3. Send Polo response
                Result polo_result = m_hb.send_polo();
                if (polo_result.is_err()) {
                    qDebug() << "Failed to send polo:"
                             << QString::fromStdString(
                                    polo_result.unwrap_err().message);
                    break;
                }
                qDebug() << "Sent polo successfully";

                interval += 5;
                m_initialCompleted = true;
            }
        } catch (const std::exception &e) {
            qDebug() << "Heartbeat error:" << e.what();
        }
    }

    bool initialCompleted() const { return m_initialCompleted; }

private:
    Heartbeat m_hb;
    bool m_initialCompleted = false;
};
#endif // HEARTBEATTHREAD_H