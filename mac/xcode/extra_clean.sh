#!/bin/sh

rm -f generated/sdksys generated/build
rm -f generated/*.h
rm -f generated/core/*
rm -f generated/ui/*
rm -f generated/gui/*
rm -f generated/app/*
rm -f generated/helpviewer/*
rm -f generated/i18n/*

# providers & plugins
rm -f generated/wmsprovider/*
rm -f generated/delimitedtextprovider/*
rm -f generated/postgresprovider/*
rm -f generated/gpxprovider/*
rm -f generated/wfsprovider/*
rm -f generated/copyrightlabelplugin/*
rm -f generated/coordinatecaptureplugin/*
rm -f generated/delimitedtextplugin/*
rm -f generated/dxf2shpconverterplugin/*
rm -f generated/georeferencerplugin/*
rm -f generated/gpsimporterplugin/*
rm -f generated/grassplugin/*
rm -f generated/interpolationplugin/*
rm -f generated/northarrowplugin/*
rm -f generated/quickprintplugin/*
rm -f generated/scalebarplugin/*
rm -f generated/spitplugin/*
rm -f generated/wfsplugin/*
rm -f generated/ogrconverterplugin/*

# python
rm -f generated/python/*
rm -f generated/python/python/*
rm -f generated/python/python/core/*
rm -f generated/python/python/gui/*
rm -f generated/python/src/core/*
rm -f generated/python/src/gui/*
