#!/usr/bin/env bash


docker build -v ..:/root/QGIS -t "qgis/qgis3-build-deps:new" ..
