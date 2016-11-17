#!/bin/bash
wget http://distribute.kde.org/kdeflatpak.gpg
flatpak --user remote-add kde http://distribute.kde.org/flatpak-testing/ --gpg-import=kdeflatpak.gpg
rm kdeflatpak.gpg
flatpak --user install kde org.kde.Platform
flatpak --user install kde org.kde.Sdk

git -C .. archive --format=tar.gz --prefix=qgis-HEAD/ HEAD > qgis.tar.gz
flatpak-builder --ccache --require-changes --force-clean --subject="build of org.qgis, `date`" qgis-app qgis.json
flatpak build-export repo qgis-app
