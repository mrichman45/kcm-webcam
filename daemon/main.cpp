// Reapplies saved webcam settings at login and whenever a camera appears
// (for example after flipping the laptop's camera switch back on).
//
//   webcam-settings-apply           apply once and exit
//   webcam-settings-apply --watch   keep running and watch for cameras appearing

#include "v4l2camera.h"

#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <QSet>
#include <QTimer>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("webcam-settings-apply"));
    const bool watch = app.arguments().contains(QStringLiteral("--watch"));

    QSet<QString> known;
    int retries = 0;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(1500); // give udev time to grant access to a new camera

    const auto scan = [&] {
        int unreadable = 0;
        const QList<CameraInfo> cams = V4L2::listCameras(&unreadable);
        QSet<QString> present;
        for (const CameraInfo &cam : cams) {
            const QString id = cam.node + QLatin1Char('|') + cam.key();
            present.insert(id);
            if (!known.contains(id)) {
                V4L2::applySaved(cam);
            }
        }
        known = present;
        if (unreadable > 0 && retries < 5) {
            ++retries;
            timer.start();
        }
    };

    scan();
    if (!watch) {
        return 0;
    }

    QFileSystemWatcher watcher({QStringLiteral("/dev")});
    QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged, &timer, [&] {
        retries = 0;
        timer.start();
    });
    QObject::connect(&timer, &QTimer::timeout, scan);
    return app.exec();
}
