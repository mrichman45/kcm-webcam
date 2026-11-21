# Webcam settings for KDE System Settings

Adds a **Webcam** page to KDE System Settings (Plasma 6), under **Disks & Cameras**.

- Live camera preview, so you see each change as you make it (with an on/off switch).
- A slider, switch or drop-down for every setting your webcam supports:
  brightness, contrast, exposure, white balance, focus and so on.
- **Apply** saves your settings, and **Defaults** resets the camera.
- Settings come back automatically after a reboot, and when a laptop's camera
  switch (or privacy shutter key) is turned back on.

## Requirements

- An Arch-based system: Arch, CachyOS, EndeavourOS, Manjaro and similar.
- KDE Plasma 6.

On other systems, see "Other distributions" below.

## Install

Open a terminal in this folder and run:

```bash
./install.sh
```

It builds the package, installs it through pacman (it will ask for your password
and to confirm the packages it needs), then turns on the background service that
restores your settings.

Then open **System Settings → Disks & Cameras → Webcam**, or search for "webcam".

**Keep this folder.** You need it to rebuild after a big KDE or Qt update.

## If the page disappears after a big update

Rebuild it from this folder:

```bash
makepkg -sif
```

## Uninstall

```bash
systemctl --user disable --now webcam-settings.service
sudo pacman -R kcm-webcam
rm ~/.config/kcm_webcamrc      # optional: forget your saved settings
```

## Troubleshooting

- **"No webcam found":** the camera is switched off, disabled in the BIOS, or not connected.
  On laptops, look for a camera switch on the side, or a key with a crossed-out camera icon.
- **The preview is black or shows an error:** another app (Zoom, Discord, a browser tab)
  is using the camera. Close it, then turn the preview off and on again.
- **Something else:** run `kcmshell6 kcm_webcam` in a terminal. It opens the page on its own
  and prints any errors.

## Other distributions

You can build it by hand with CMake. You need the development packages for
Qt 6 (Base, Declarative, Multimedia), KDE Frameworks 6 (KCMUtils, Config,
CoreAddons, I18n, Kirigami) and extra-cmake-modules:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
cmake --build build
sudo cmake --install build
systemctl --user enable --now webcam-settings.service
```

Updates from your system's package manager will not track files installed this way.

## What's inside

| Path | What it is |
|---|---|
| `core/` | Talks to the camera through V4L2; saving and restoring settings |
| `kcm/` | The System Settings page (C++ and `ui/main.qml`) |
| `daemon/` | `webcam-settings-apply`, run by the background service |
| `webcam-settings.service` | systemd user service |
| `PKGBUILD` | Builds an Arch package, so pacman tracks it |
| `install.sh` | One-step install |

Settings are stored in `~/.config/kcm_webcamrc`.

License: GPL-2.0-or-later.
