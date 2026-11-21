#include "v4l2camera.h"

#include <KConfig>
#include <KConfigGroup>
#include <QDir>
#include <QFile>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace
{
const QString s_configName = QStringLiteral("kcm_webcamrc");

int xioctl(int fd, unsigned long request, void *arg)
{
    int r;
    do {
        r = ::ioctl(fd, request, arg);
    } while (r == -1 && errno == EINTR);
    return r;
}

class Fd
{
public:
    explicit Fd(const QString &path)
        : m_fd(::open(QFile::encodeName(path).constData(), O_RDWR | O_NONBLOCK | O_CLOEXEC))
    {
    }
    ~Fd()
    {
        if (m_fd >= 0) {
            ::close(m_fd);
        }
    }
    Fd(const Fd &) = delete;
    Fd &operator=(const Fd &) = delete;

    bool ok() const { return m_fd >= 0; }
    int get() const { return m_fd; }

private:
    int m_fd;
};

QString fromV4l(const __u8 *text, size_t size)
{
    const char *s = reinterpret_cast<const char *>(text);
    return QString::fromUtf8(s, qsizetype(strnlen(s, size))).trimmed();
}
} // namespace

QString CameraInfo::key() const
{
    return busInfo + QStringLiteral(" @ ") + card;
}

QVariantMap ControlInfo::toVariant() const
{
    QVariantList opts;
    for (const auto &o : options) {
        opts.append(QVariantMap{{QStringLiteral("value"), o.first}, {QStringLiteral("text"), o.second}});
    }
    QString typeName = QStringLiteral("int");
    if (type == Bool) {
        typeName = QStringLiteral("bool");
    } else if (type == Menu) {
        typeName = QStringLiteral("menu");
    }
    return QVariantMap{
        {QStringLiteral("id"), int(id)},
        {QStringLiteral("name"), name},
        {QStringLiteral("type"), typeName},
        {QStringLiteral("min"), min},
        {QStringLiteral("max"), max},
        {QStringLiteral("step"), step},
        {QStringLiteral("def"), def},
        {QStringLiteral("value"), value},
        {QStringLiteral("inactive"), inactive},
        {QStringLiteral("options"), opts},
    };
}

QList<CameraInfo> V4L2::listCameras(int *unreadable)
{
    QList<CameraInfo> result;
    int bad = 0;

    QStringList nodes = QDir(QStringLiteral("/dev")).entryList({QStringLiteral("video*")}, QDir::System);
    std::sort(nodes.begin(), nodes.end(), [](const QString &a, const QString &b) {
        return a.mid(5).toInt() < b.mid(5).toInt();
    });

    for (const QString &name : std::as_const(nodes)) {
        const QString path = QStringLiteral("/dev/") + name;
        Fd fd(path);
        if (!fd.ok()) {
            ++bad; // usually permissions not applied yet right after the camera appears
            continue;
        }
        v4l2_capability cap;
        std::memset(&cap, 0, sizeof cap);
        if (xioctl(fd.get(), VIDIOC_QUERYCAP, &cap) < 0) {
            continue;
        }
        const quint32 caps = (cap.capabilities & V4L2_CAP_DEVICE_CAPS) ? cap.device_caps : cap.capabilities;
        if (!(caps & V4L2_CAP_VIDEO_CAPTURE)) {
            continue;
        }
        result.append(CameraInfo{path, fromV4l(cap.card, sizeof cap.card), fromV4l(cap.bus_info, sizeof cap.bus_info)});
    }

    if (unreadable) {
        *unreadable = bad;
    }
    return result;
}

QList<ControlInfo> V4L2::listControls(const QString &node)
{
    QList<ControlInfo> out;
    Fd fd(node);
    if (!fd.ok()) {
        return out;
    }

    v4l2_queryctrl q;
    std::memset(&q, 0, sizeof q);
    q.id = V4L2_CTRL_FLAG_NEXT_CTRL;

    while (xioctl(fd.get(), VIDIOC_QUERYCTRL, &q) == 0) {
        const quint32 id = q.id;
        const bool supportedType =
            q.type == V4L2_CTRL_TYPE_INTEGER || q.type == V4L2_CTRL_TYPE_BOOLEAN || q.type == V4L2_CTRL_TYPE_MENU;
        const bool usable = supportedType && !(q.flags & (V4L2_CTRL_FLAG_DISABLED | V4L2_CTRL_FLAG_READ_ONLY));

        if (usable) {
            ControlInfo c;
            c.id = id;
            c.name = fromV4l(q.name, sizeof q.name);
            c.type = q.type == V4L2_CTRL_TYPE_BOOLEAN ? ControlInfo::Bool
                   : q.type == V4L2_CTRL_TYPE_MENU    ? ControlInfo::Menu
                                                      : ControlInfo::Int;
            c.min = q.minimum;
            c.max = q.maximum;
            c.step = q.step > 0 ? q.step : 1;
            c.def = q.default_value;
            c.inactive = q.flags & V4L2_CTRL_FLAG_INACTIVE;

            v4l2_control v;
            std::memset(&v, 0, sizeof v);
            v.id = id;
            c.value = xioctl(fd.get(), VIDIOC_G_CTRL, &v) == 0 ? v.value : c.def;

            if (c.type == ControlInfo::Menu && c.max - c.min < 64) {
                for (qint32 i = c.min; i <= c.max; ++i) {
                    v4l2_querymenu m;
                    std::memset(&m, 0, sizeof m);
                    m.id = id;
                    m.index = quint32(i);
                    if (xioctl(fd.get(), VIDIOC_QUERYMENU, &m) == 0) {
                        c.options.append({i, fromV4l(m.name, sizeof m.name)});
                    }
                }
            }
            out.append(c);
        }

        std::memset(&q, 0, sizeof q);
        q.id = id | V4L2_CTRL_FLAG_NEXT_CTRL;
    }
    return out;
}

bool V4L2::setControl(const QString &node, quint32 id, qint32 value)
{
    Fd fd(node);
    if (!fd.ok()) {
        return false;
    }
    v4l2_control v;
    std::memset(&v, 0, sizeof v);
    v.id = id;
    v.value = value;
    return xioctl(fd.get(), VIDIOC_S_CTRL, &v) == 0;
}

void V4L2::saveCamera(const CameraInfo &camera)
{
    const auto controls = listControls(camera.node);
    if (controls.isEmpty()) {
        return;
    }
    KConfig config(s_configName);
    KConfigGroup group = config.group(camera.key());
    const QStringList oldKeys = group.keyList();
    for (const QString &k : oldKeys) {
        group.deleteEntry(k);
    }
    group.writeEntry(QStringLiteral("Name"), camera.card);
    for (const auto &c : controls) {
        if (!c.inactive) { // values of inactive controls are managed by the camera's auto modes
            group.writeEntry(QString::number(c.id), c.value);
        }
    }
    config.sync();
}

void V4L2::applySaved(const CameraInfo &camera)
{
    KConfig config(s_configName);
    const KConfigGroup group = config.group(camera.key());
    if (!group.exists()) {
        return;
    }

    // Modes (auto exposure, auto white balance...) first, so manual values are accepted afterwards.
    const auto applyPass = [&](bool integers) {
        const auto controls = listControls(camera.node);
        for (const auto &c : controls) {
            if ((c.type == ControlInfo::Int) != integers) {
                continue;
            }
            const QString key = QString::number(c.id);
            if (!group.hasKey(key)) {
                continue;
            }
            const int wanted = group.readEntry(key, c.value);
            if (wanted != c.value) {
                setControl(camera.node, c.id, wanted);
            }
        }
    };
    applyPass(false);
    applyPass(true);
}
