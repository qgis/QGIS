/***************************************************************************
                         qgspointclouddataprovider.cpp
                         -----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgspointclouddataprovider.h"
#include "qgspointcloudindex.h"
#include "qgsgeometry.h"
#include <mutex>

QgsPointCloudDataProvider::QgsPointCloudDataProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( uri, options, flags )
{
}

QgsPointCloudDataProvider::~QgsPointCloudDataProvider() = default;

QgsPointCloudDataProvider::Capabilities QgsPointCloudDataProvider::capabilities() const
{
  return QgsPointCloudDataProvider::NoCapabilities;
}

QgsGeometry QgsPointCloudDataProvider::polygonBounds() const
{
  return QgsGeometry::fromRect( extent() );
}

QgsPointCloudRenderer *QgsPointCloudDataProvider::createRenderer( const QVariantMap & ) const
{
  return nullptr;
}

QMap<int, QString> QgsPointCloudDataProvider::lasClassificationCodes()
{
  static QMap< int, QString > sCodes
  {
    {0, QStringLiteral( "Created, Never Classified" )},
    {1, QStringLiteral( "Unclassified" )},
    {2, QStringLiteral( "Ground" )},
    {3, QStringLiteral( "Low Vegetation" )},
    {4, QStringLiteral( "Medium Vegetation" )},
    {5, QStringLiteral( "High Vegetation" )},
    {6, QStringLiteral( "Building" )},
    {7, QStringLiteral( "Low Point (Low Noise)" )},
    {8, QStringLiteral( "Reserved" )},
    {9, QStringLiteral( "Water" )},
    {10, QStringLiteral( "Rail" )},
    {11, QStringLiteral( "Road Surface" )},
    {12, QStringLiteral( "Reserved" )},
    {13, QStringLiteral( "Wire - Guard (Shield)" )},
    {14, QStringLiteral( "Wire - Conductor (Phase)" )},
    {15, QStringLiteral( "Transmission Tower" )},
    {16, QStringLiteral( "Wire-Structure Connector (Insulator)" )},
    {17, QStringLiteral( "Bridge Deck" )},
    {18, QStringLiteral( "High Noise" )},
  };

  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    for ( int i = 19; i <= 63; ++i )
      sCodes.insert( i, QStringLiteral( "Reserved" ) );
    for ( int i = 64; i <= 255; ++i )
      sCodes.insert( i, QStringLiteral( "User Definable" ) );
  } );

  return sCodes;
}

QMap<int, QString> QgsPointCloudDataProvider::translatedLasClassificationCodes()
{
  static QMap< int, QString > sCodes
  {
    {0, QObject::tr( "Created, Never Classified" )},
    {1, QObject::tr( "Unclassified" )},
    {2, QObject::tr( "Ground" )},
    {3, QObject::tr( "Low Vegetation" )},
    {4, QObject::tr( "Medium Vegetation" )},
    {5, QObject::tr( "High Vegetation" )},
    {6, QObject::tr( "Building" )},
    {7, QObject::tr( "Low Point (Noise)" )},
    {8, QObject::tr( "Reserved" )},
    {9, QObject::tr( "Water" )},
    {10, QObject::tr( "Rail" )},
    {11, QObject::tr( "Road Surface" )},
    {12, QObject::tr( "Reserved" )},
    {13, QObject::tr( "Wire - Guard (Shield)" )},
    {14, QObject::tr( "Wire - Conductor (Phase)" )},
    {15, QObject::tr( "Transmission Tower" )},
    {16, QObject::tr( "Wire-Structure Connector (Insulator)" )},
    {17, QObject::tr( "Bridge Deck" )},
    {18, QObject::tr( "High Noise" )},
  };

  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    for ( int i = 19; i <= 63; ++i )
      sCodes.insert( i, QObject::tr( "Reserved" ) );
    for ( int i = 64; i <= 255; ++i )
      sCodes.insert( i, QObject::tr( "User Definable" ) );
  } );

  return sCodes;
}

QVariant QgsPointCloudDataProvider::metadataStatistic( const QString &, QgsStatisticalSummary::Statistic ) const
{
  return QVariant();
}

QVariantList QgsPointCloudDataProvider::metadataClasses( const QString & ) const
{
  return QVariantList();
}

QVariant QgsPointCloudDataProvider::metadataClassStatistic( const QString &, const QVariant &, QgsStatisticalSummary::Statistic ) const
{
  return QVariant();
}
