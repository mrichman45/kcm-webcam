#!/bin/sh
# One-step install for Arch-based systems with KDE Plasma 6.
set -e
cd "$(dirname "$0")"

if [ "$(id -u)" -eq 0 ]; then
    echo "Run this as your normal user, not as root. It will ask for your password when needed."
    exit 1
fi

if ! command -v makepkg >/dev/null 2>&1; then
    echo "This installer needs an Arch-based system (Arch, CachyOS, EndeavourOS, Manjaro)."
    echo "See \"Other distributions\" in README.md for building by hand."
    exit 1
fi

if ! command -v systemsettings >/dev/null 2>&1; then
    echo "KDE System Settings was not found. This page needs KDE Plasma 6."
    exit 1
fi

makepkg -sif

systemctl --user daemon-reload
systemctl --user enable --now webcam-settings.service

echo
echo "Done. Open System Settings -> Disks & Cameras -> Webcam (or search for \"webcam\")."
echo "Keep this folder: run 'makepkg -sif' here if the page ever disappears after a big KDE update."
