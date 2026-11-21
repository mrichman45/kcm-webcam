#include "webcamkcm.h"

#include <KPluginFactory>

#include <algorithm>

K_PLUGIN_CLASS_WITH_JSON(WebcamKcm, "kcm_webcam.json")

WebcamKcm::WebcamKcm(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
{
    setButtons(Apply | Default);

    // The camera switch makes /dev/videoN disappear and reappear, so watch /dev.
    // The delay lets permissions settle and lets the background service reapply saved values first.
    m_rescanTimer.setSingleShot(true);
    m_rescanTimer.setInterval(2000);
    connect(&m_rescanTimer, &QTimer::timeout, this, [this] {
        rescan(false);
    });
    m_watcher.addPath(QStringLiteral("/dev"));
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        m_retries = 0;
        m_rescanTimer.start();
    });
}

QStringList WebcamKcm::cameraNames() const
{
    QStringList names;
    for (const auto &cam : m_cameras) {
        names.append(cam.card);
    }
    return names;
}

void WebcamKcm::setCurrentCamera(int index)
{
    if (index == m_current || index < 0 || index >= m_cameras.size()) {
        return;
    }
    m_current = index;
    Q_EMIT currentCameraChanged();
    reloadControls(true);
}

QVariantList WebcamKcm::controls() const
{
    QVariantList list;
    for (const auto &c : m_controls) {
        list.append(c.toVariant());
    }
    return list;
}

QString WebcamKcm::currentNode() const
{
    return (m_current >= 0 && m_current < m_cameras.size()) ? m_cameras.at(m_current).node : QString();
}

void WebcamKcm::rescan(bool force)
{
    const QString previous = (m_current >= 0 && m_current < m_cameras.size()) ? m_cameras.at(m_current).key() : QString();

    int unreadable = 0;
    const QList<CameraInfo> cams = V4L2::listCameras(&unreadable);
    if (unreadable > 0 && m_retries < 5) {
        ++m_retries;
        m_rescanTimer.start();
    }

    bool changed = cams.size() != m_cameras.size();
    for (qsizetype i = 0; !changed && i < cams.size(); ++i) {
        changed = cams.at(i).node != m_cameras.at(i).node || cams.at(i).key() != m_cameras.at(i).key();
    }
    m_cameras = cams;

    int index = cams.isEmpty() ? -1 : 0;
    for (qsizetype i = 0; i < cams.size(); ++i) {
        if (cams.at(i).key() == previous) {
            index = int(i);
            break;
        }
    }

    if (changed) {
        Q_EMIT camerasChanged();
    }
    const bool indexChanged = index != m_current;
    if (indexChanged) {
        m_current = index;
        Q_EMIT currentCameraChanged();
    }
    if (force || changed || indexChanged) {
        reloadControls(true);
    }
}

void WebcamKcm::reloadControls(bool forceEmit)
{
    const QString node = currentNode();
    const QList<ControlInfo> fresh = node.isEmpty() ? QList<ControlInfo>() : V4L2::listControls(node);

    // Only rebuild the page when the set of controls or their enabled state changed,
    // so dragging a slider is not interrupted.
    bool structural = forceEmit || fresh.size() != m_controls.size();
    for (qsizetype i = 0; !structural && i < fresh.size(); ++i) {
        structural = fresh.at(i).id != m_controls.at(i).id || fresh.at(i).inactive != m_controls.at(i).inactive;
    }
    m_controls = fresh;
    if (structural) {
        Q_EMIT controlsChanged();
    }
    updateDefaultsState();
}

void WebcamKcm::updateDefaultsState()
{
    const bool atDefaults = std::all_of(m_controls.cbegin(), m_controls.cend(), [](const ControlInfo &c) {
        return c.inactive || c.value == c.def;
    });
    setRepresentsDefaults(atDefaults);
}

void WebcamKcm::setControl(int id, int value)
{
    const QString node = currentNode();
    if (node.isEmpty()) {
        return;
    }
    V4L2::setControl(node, quint32(id), value);
    reloadControls(false);
    setNeedsSave(true);
}

void WebcamKcm::load()
{
    KQuickConfigModule::load();
    m_retries = 0;
    rescan(true);
    setNeedsSave(false);
}

void WebcamKcm::save()
{
    KQuickConfigModule::save();
    for (const auto &cam : std::as_const(m_cameras)) {
        V4L2::saveCamera(cam);
    }
    setNeedsSave(false);
}

void WebcamKcm::defaults()
{
    KQuickConfigModule::defaults();
    const QString node = currentNode();
    if (node.isEmpty()) {
        return;
    }
    // Modes first (e.g. auto exposure back on), then plain values that are still adjustable.
    for (const auto &c : V4L2::listControls(node)) {
        if (c.type != ControlInfo::Int && c.value != c.def) {
            V4L2::setControl(node, c.id, c.def);
        }
    }
    for (const auto &c : V4L2::listControls(node)) {
        if (c.type == ControlInfo::Int && !c.inactive && c.value != c.def) {
            V4L2::setControl(node, c.id, c.def);
        }
    }
    reloadControls(true);
    setNeedsSave(true);
}

#include "webcamkcm.moc"
