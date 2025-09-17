/***************************************************************************
    qgslocaleutils.cpp
     -------------
    Date                 : September 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslocaleutils.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsreadwritelocker.h"
#include "qgsfeaturerequest.h"
#include "qgsexpression.h"
#include <QReadWriteLock>
#include <mutex>

QgsRectangle QgsLocaleUtils::countryBounds( const QString &country )
{
  static QReadWriteLock sMutex;
  static QMap< QString, QgsRectangle > sCache;
  QgsReadWriteLocker locker( sMutex, QgsReadWriteLocker::Read );
  {
    auto it = sCache.constFind( country );
    if ( it != sCache.constEnd() )
      return it.value();
  }

  locker.changeMode( QgsReadWriteLocker::Write );
  const QString fileName = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/world_map.gpkg|layername=countries" );
  QgsVectorLayer::LayerOptions options;
  options.forceReadOnly = true;
  auto layer = std::make_unique< QgsVectorLayer >( fileName, QStringLiteral( "World Map" ), QStringLiteral( "ogr" ), options );

  QgsFeatureRequest req = QgsFeatureRequest().setFilterExpression( QgsExpression::createFieldEqualityExpression( QStringLiteral( "Name" ), country ) ).setNoAttributes();
  QgsFeatureIterator featureIt = layer->getFeatures( req );
  QgsFeature f;
  QgsRectangle rect;
  if ( featureIt.nextFeature( f ) )
  {
    rect = f.geometry().boundingBox();
  }

  sCache.insert( country, rect );
  return rect;
}

QgsRectangle QgsLocaleUtils::territoryBounds( const QLocale &locale )
{
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
  const QString territoryName = QLocale::countryToString( locale.country() );
#else
  const QString territoryName = QLocale::territoryToCode( locale.territory() );
#endif

  return countryBounds( territoryName );
}

QgsRectangle QgsLocaleUtils::systemLocaleGeographicBounds()
{
  static std::once_flag initialized;
  static QgsRectangle sRes;
  std::call_once( initialized, []
  {
    sRes = territoryBounds( QLocale::system() );
  } );

  return sRes;
}
