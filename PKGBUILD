# Local package: build with `makepkg -si` from this folder.
pkgname=kcm-webcam
pkgver=0.2.1
pkgrel=1
pkgdesc="Webcam settings page for KDE System Settings"
arch=('x86_64')
license=('GPL-2.0-or-later')
depends=('kcmutils' 'kconfig' 'kcoreaddons' 'ki18n' 'kirigami' 'qt6-base' 'qt6-declarative' 'qt6-multimedia' 'qt6-multimedia-ffmpeg')
makedepends=('cmake' 'extra-cmake-modules')
install=kcm-webcam.install
source=()

build() {
  cmake -B build -S "$startdir" \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    -DKDE_INSTALL_USE_QT_SYS_PATHS=ON \
    -DBUILD_TESTING=OFF
  cmake --build build
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}
