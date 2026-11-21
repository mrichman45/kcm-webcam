// Shared webcam (V4L2) helpers used by the settings page and the background service.
#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QVariantMap>

struct CameraInfo {
    QString node;    // e.g. /dev/video0
    QString card;    // human-readable name reported by the driver
    QString busInfo; // stable physical location, e.g. usb-0000:00:14.0-8

    // Identifies the same physical camera across reboots and switch on/off.
    QString key() const;
};

struct ControlInfo {
    enum Type { Int, Bool, Menu };

    quint32 id = 0;
    QString name;
    Type type = Int;
    qint32 min = 0;
    qint32 max = 0;
    qint32 step = 1;
    qint32 def = 0;
    qint32 value = 0;
    bool inactive = false; // e.g. exposure time while auto exposure is on
    QList<QPair<qint32, QString>> options; // menu entries

    QVariantMap toVariant() const;
};

namespace V4L2
{
// Video capture devices only (skips metadata nodes such as /dev/video1).
// `unreadable` receives the number of /dev/video* nodes that could not be opened yet.
QList<CameraInfo> listCameras(int *unreadable = nullptr);

QList<ControlInfo> listControls(const QString &node);
bool setControl(const QString &node, quint32 id, qint32 value);

// Persistence in ~/.config/kcm_webcamrc
void saveCamera(const CameraInfo &camera);
void applySaved(const CameraInfo &camera);
}
