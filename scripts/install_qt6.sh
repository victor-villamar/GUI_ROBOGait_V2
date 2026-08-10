#!/usr/bin/env bash

install_qt6_dependencies()
{
  log "Installing Qt6 dependencies"
  apt_install \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-declarative-dev \
    qt6-declarative-dev-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libgl1-mesa-dev \
    libqt6opengl6-dev \
    libqt6svg6 \
    libqt6virtualkeyboard6 \
    qt6-virtualkeyboard-plugin \
    qml6-module-qtquick \
    qml6-module-qtquick-window \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtqml \
    qml6-module-qtqml-workerscript \
    qml6-module-qtquick-virtualkeyboard \
    qml6-module-qtquick-templates \
    qml6-module-qt-labs-folderlistmodel \
    qml6-module-qt-labs-settings \
    libqt6sql6-sqlite \
    hunspell-es
}
