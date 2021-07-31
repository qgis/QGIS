/***************************************************************************
  qgsbabelformatregistry.cpp
   -------------------
  begin                : July 2021
  copyright            : (C) 2021 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbabelformatregistry.h"
#include "qgsbabelformat.h"
#include "qgsbabelgpsdevice.h"
#include "qgssettings.h"
#include <QString>

QgsBabelFormatRegistry::QgsBabelFormatRegistry()
{
  mImporters[QStringLiteral( "CoPilot Flight Planner" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "copilot" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Delorme GPS Log" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "gpl" ), Qgis::BabelFormatCapability::Tracks );
  mImporters[QStringLiteral( "Delorme Routes" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "saroute" ), Qgis::BabelFormatCapability::Tracks );
  mImporters[QStringLiteral( "Delorme Street Atlas 2004 Plus" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "saplus" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "DNA" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "dna" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "EasyGPS Binary Format" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "easygps" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Fugawi" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "fugawi" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Garmin Mapsource" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "mapsource" ), Qgis::BabelFormatCapability::Waypoints | Qgis::BabelFormatCapability::Routes | Qgis::BabelFormatCapability::Tracks );
  mImporters[QStringLiteral( "Garmin PCX5" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "pcx" ), Qgis::BabelFormatCapability::Waypoints | Qgis::BabelFormatCapability::Tracks );
  mImporters[QStringLiteral( "Geocaching.com .loc" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "geo" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "GeocachingDB" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "gcdb" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "GPSDrive" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "gpsdrive" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "GPSman" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "gpsman" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "GPSUtil" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "gpsutil" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Holux" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "holux" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Magellan Mapsend" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "mapsend" ), Qgis::BabelFormatCapability::Waypoints | Qgis::BabelFormatCapability::Routes | Qgis::BabelFormatCapability::Tracks );
  mImporters[QStringLiteral( "Magellan Navigator Companion" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "magnav" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Mapopolis.com Mapconverter Application" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "mapconverter" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Maptech" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "mxf" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Microsoft Streets and Trips" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "s_and_t" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Navicache" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "navicache" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "NIMA/GNIS Geographic Names" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "nima" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "NMEA Sentences" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "nmea" ), Qgis::BabelFormatCapability::Waypoints | Qgis::BabelFormatCapability::Tracks );
  mImporters[QStringLiteral( "OziExplorer" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "ozi" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "PocketStreets 2002/2003 Pushpin" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "psp" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "PSITrex" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "psitrex" ), Qgis::BabelFormatCapability::Waypoints | Qgis::BabelFormatCapability::Routes | Qgis::BabelFormatCapability::Tracks );
  mImporters[QStringLiteral( "Shapefile" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "shape" ), Qgis::BabelFormatCapability::Waypoints | Qgis::BabelFormatCapability::Routes | Qgis::BabelFormatCapability::Tracks );
  mImporters[QStringLiteral( "Tiger" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "tiger" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "Topo by National Geographic" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "tpg" ), Qgis::BabelFormatCapability::Waypoints );
  mImporters[QStringLiteral( "TopoMapPro" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "tmpro" ), Qgis::BabelFormatCapability::Waypoints );

  reloadFromSettings();
}

QgsBabelFormatRegistry::~QgsBabelFormatRegistry()
{
  qDeleteAll( mImporters );
  qDeleteAll( mDevices );
}

QStringList QgsBabelFormatRegistry::importFormatNames() const
{
  return mImporters.keys();
}

QgsAbstractBabelFormat *QgsBabelFormatRegistry::importFormat( const QString &name )
{
  return mImporters.value( name );
}

QStringList QgsBabelFormatRegistry::deviceNames() const
{
  return mDevices.keys();
}

QgsBabelGpsDeviceFormat *QgsBabelFormatRegistry::deviceFormat( const QString &name )
{
  return mDevices.value( name );
}

QMap<QString, QgsBabelGpsDeviceFormat *> QgsBabelFormatRegistry::devices() const
{
  return mDevices;
}

void QgsBabelFormatRegistry::reloadFromSettings()
{
  qDeleteAll( mDevices );
  mDevices.clear();

  mDevices[QStringLiteral( "Garmin serial" )] =
    new QgsBabelGpsDeviceFormat( QStringLiteral( "%babel -w -i garmin -o gpx %in %out" ),
                                 QStringLiteral( "%babel -w -i gpx -o garmin %in %out" ),
                                 QStringLiteral( "%babel -r -i garmin -o gpx %in %out" ),
                                 QStringLiteral( "%babel -r -i gpx -o garmin %in %out" ),
                                 QStringLiteral( "%babel -t -i garmin -o gpx %in %out" ),
                                 QStringLiteral( "%babel -t -i gpx -o garmin %in %out" ) );

  QgsSettings settings;
  const QStringList deviceNames = settings.value( QStringLiteral( "Plugin-GPS/devicelist" ) ).toStringList();
  for ( const QString &device : deviceNames )
  {
    const QString baseKey = QStringLiteral( "/Plugin-GPS/devices/%1" ).arg( device );
    const QString wptDownload = settings.value( QStringLiteral( "%1/wptdownload" ).arg( baseKey ) ).toString();
    const QString wptUpload = settings.value( QStringLiteral( "%1/wptupload" ).arg( baseKey ) ).toString();
    const QString rteDownload = settings.value( QStringLiteral( "%1/rtedownload" ).arg( baseKey ) ).toString();
    const QString rteUpload = settings.value( QStringLiteral( "%1/rteupload" ).arg( baseKey ) ).toString();
    const QString trkDownload = settings.value( QStringLiteral( "%1/trkdownload" ).arg( baseKey ) ).toString();
    const QString trkUpload = settings.value( QStringLiteral( "%1/trkupload" ).arg( baseKey ) ).toString();

    mDevices[device] = new QgsBabelGpsDeviceFormat( wptDownload,
        wptUpload,
        rteDownload,
        rteUpload,
        trkDownload,
        trkUpload );
  }
}

