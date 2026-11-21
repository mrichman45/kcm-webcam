#pragma once

#include "v4l2camera.h"

#include <KQuickConfigModule>
#include <QFileSystemWatcher>
#include <QTimer>

class WebcamKcm : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QStringList cameraNames READ cameraNames NOTIFY camerasChanged)
    Q_PROPERTY(int currentCamera READ currentCamera WRITE setCurrentCamera NOTIFY currentCameraChanged)
    Q_PROPERTY(QVariantList controls READ controls NOTIFY controlsChanged)

public:
    WebcamKcm(QObject *parent, const KPluginMetaData &data);

    QStringList cameraNames() const;
    int currentCamera() const { return m_current; }
    void setCurrentCamera(int index);
    QVariantList controls() const;

    Q_INVOKABLE void setControl(int id, int value);

    void load() override;
    void save() override;
    void defaults() override;

Q_SIGNALS:
    void camerasChanged();
    void currentCameraChanged();
    void controlsChanged();

private:
    void rescan(bool force);
    void reloadControls(bool forceEmit);
    void updateDefaultsState();
    QString currentNode() const;

    QList<CameraInfo> m_cameras;
    int m_current = -1;
    QList<ControlInfo> m_controls;
    QFileSystemWatcher m_watcher;
    QTimer m_rescanTimer;
    int m_retries = 0;
};
