/***************************************************************************
                        qgsgotolocatorfilters.cpp
                        ----------------------------
   begin                : May 2017
   copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsgotolocatorfilter.h"
#include "qgsfeedback.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgscoordinateutils.h"

#include <QUrl>


QgsGotoLocatorFilter::QgsGotoLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsGotoLocatorFilter *QgsGotoLocatorFilter::clone() const
{
  return new QgsGotoLocatorFilter();
}

void QgsGotoLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  if ( feedback->isCanceled() )
    return;

  const QgsCoordinateReferenceSystem currentCrs = QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs();
  const QgsCoordinateReferenceSystem wgs84Crs( QStringLiteral( "EPSG:4326" ) );

  bool okX = false;
  bool okY = false;
  double posX = 0.0;
  double posY = 0.0;
  bool posIsWgs84 = false;
  const QLocale locale;

  // Coordinates such as 106.8468,-6.3804
  QRegularExpression separatorRx( QStringLiteral( "^([0-9\\-\\%1\\%2]*)[\\s%3]*([0-9\\-\\%1\\%2]*)$" ).arg( locale.decimalPoint(),
                                  locale.groupSeparator(),
                                  locale.decimalPoint() != ',' && locale.groupSeparator() != ',' ? QStringLiteral( "\\," ) : QString() ) );
  QRegularExpressionMatch match = separatorRx.match( string.trimmed() );
  if ( match.hasMatch() )
  {
    posX = locale.toDouble( match.captured( 1 ), &okX );
    posY = locale.toDouble( match.captured( 2 ), &okY );
  }

  if ( !match.hasMatch() || !okX || !okY )
  {
    // Digit detection using user locale failed, use default C decimal separators
    separatorRx = QRegularExpression( QStringLiteral( "^([0-9\\-\\.]*)[\\s\\,]*([0-9\\-\\.]*)$" ) );
    match = separatorRx.match( string.trimmed() );
    if ( match.hasMatch() )
    {
      posX = match.captured( 1 ).toDouble( &okX );
      posY = match.captured( 2 ).toDouble( &okY );
    }
  }

  if ( !match.hasMatch() )
  {
    // Check if the string is a pair of decimal degrees with [N,S,E,W] suffixes
    separatorRx = QRegularExpression( QStringLiteral( "^\\s*([0-9\\-\\.]*\\s*[NSEWnsew])[\\s\\,]*([0-9\\-\\.]*\\s*[NSEWnsew])\\s*$" ) );
    match = separatorRx.match( string.trimmed() );
    if ( match.hasMatch() )
    {
      posIsWgs84 = true;
      bool isEasting = false;
      posX = QgsCoordinateUtils::degreeToDecimal( match.captured( 1 ), &okX, &isEasting );
      posY = QgsCoordinateUtils::degreeToDecimal( match.captured( 2 ), &okY );
      if ( !isEasting )
        std::swap( posX, posY );
    }
  }

  if ( !match.hasMatch() )
  {
    // Check if the string is a pair of degree minute second
    separatorRx = QRegularExpression( QStringLiteral( "^((?:([-+nsew])\\s*)?\\d{1,3}(?:[^0-9.]+[0-5]?\\d)?[^0-9.]+[0-5]?\\d(?:\\.\\d+)?[^0-9.,]*[-+nsew]?)[,\\s]+((?:([-+nsew])\\s*)?\\d{1,3}(?:[^0-9.]+[0-5]?\\d)?[^0-9.]+[0-5]?\\d(?:\\.\\d+)?[^0-9.,]*[-+nsew]?)$" ) );
    match = separatorRx.match( string.trimmed() );
    if ( match.hasMatch() )
    {
      posIsWgs84 = true;
      bool isEasting = false;
      posX = QgsCoordinateUtils::dmsToDecimal( match.captured( 1 ), &okX, &isEasting );
      posY = QgsCoordinateUtils::dmsToDecimal( match.captured( 3 ), &okY );
      if ( !isEasting )
        std::swap( posX, posY );
    }
  }

  if ( okX && okY )
  {
    QVariantMap data;
    const QgsPointXY point( posX, posY );
    data.insert( QStringLiteral( "point" ), point );

    const bool withinWgs84 = wgs84Crs.bounds().contains( point );
    if ( !posIsWgs84 && currentCrs != wgs84Crs )
    {
      QgsLocatorResult result;
      result.filter = this;
      result.displayString = tr( "Go to %1 %2 (Map CRS, %3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), currentCrs.userFriendlyIdentifier() );
      result.userData = data;
      result.score = 0.9;
      emit resultFetched( result );
    }

    if ( withinWgs84 )
    {
      if ( currentCrs != wgs84Crs )
      {
        const QgsCoordinateTransform transform( wgs84Crs, currentCrs, QgsProject::instance()->transformContext() );
        QgsPointXY transformedPoint;
        try
        {
          transformedPoint = transform.transform( point );
        }
        catch ( const QgsException &e )
        {
          Q_UNUSED( e )
          return;
        }
        data[QStringLiteral( "point" )] = transformedPoint;
      }

      QgsLocatorResult result;
      result.filter = this;
      result.displayString = tr( "Go to %1° %2° (%3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), wgs84Crs.userFriendlyIdentifier() );
      result.userData = data;
      result.score = 1.0;
      emit resultFetched( result );
    }
    return;
  }

  QMap<int, double> scales;
  scales[0] = 739571909;
  scales[1] = 369785954;
  scales[2] = 184892977;
  scales[3] = 92446488;
  scales[4] = 46223244;
  scales[5] = 23111622;
  scales[6] = 11555811;
  scales[7] = 5777905;
  scales[8] = 2888952;
  scales[9] = 1444476;
  scales[10] = 722238;
  scales[11] = 361119;
  scales[12] = 180559;
  scales[13] = 90279;
  scales[14] = 45139;
  scales[15] = 22569;
  scales[16] = 11284;
  scales[17] = 5642;
  scales[18] = 2821;
  scales[19] = 1500;
  scales[20] = 1000;
  scales[21] = 282;

  const QUrl url( string );
  if ( url.isValid() )
  {
    double scale = 0.0;
    int meters = 0;
    okX = false;
    okY = false;
    posX = 0.0;
    posY = 0.0;
    if ( url.hasFragment() )
    {
      // Check for OSM/Leaflet/OpenLayers pattern (e.g. http://www.openstreetmap.org/#map=6/46.423/4.746)
      const QStringList fragments = url.fragment().split( '&' );
      for ( const QString &fragment : fragments )
      {
        if ( fragment.startsWith( QLatin1String( "map=" ) ) )
        {
          const QStringList params = fragment.mid( 4 ).split( '/' );
          if ( params.size() >= 3 )
          {
            if ( scales.contains( params.at( 0 ).toInt() ) )
            {
              scale = scales.value( params.at( 0 ).toInt() );
            }
            posX = params.at( 2 ).toDouble( &okX );
            posY = params.at( 1 ).toDouble( &okY );
          }
          break;
        }
      }
    }

    if ( !okX && !okY )
    {
      const QRegularExpression locationRx( QStringLiteral( "google.*\\/@([0-9\\-\\.\\,]*)(z|m|a)" ) );
      match = locationRx.match( string );
      if ( match.hasMatch() )
      {
        const QStringList params = match.captured( 1 ).split( ',' );
        if ( params.size() == 3 )
        {
          posX = params.at( 1 ).toDouble( &okX );
          posY = params.at( 0 ).toDouble( &okY );

          if ( okX && okY )
          {
            if ( match.captured( 2 ) == QChar( 'z' ) && scales.contains( static_cast<int>( params.at( 2 ).toDouble() ) ) )
            {
              scale = scales.value( static_cast<int>( params.at( 2 ).toDouble() ) );
            }
            else if ( match.captured( 2 ) == QChar( 'm' ) )
            {
              // satellite view URL, scale to be derived from canvas height
              meters = params.at( 2 ).toInt();
            }
            else if ( match.captured( 2 ) == QChar( 'a' ) )
            {
              // street view URL, use most zoomed in scale value
              scale = scales.value( 21 );
            }
          }
        }
      }
    }

    if ( okX && okY )
    {
      QVariantMap data;
      const QgsPointXY point( posX, posY );
      QgsPointXY dataPoint = point;
      const bool withinWgs84 = wgs84Crs.bounds().contains( point );
      if ( withinWgs84 && currentCrs != wgs84Crs )
      {
        const QgsCoordinateTransform transform( wgs84Crs, currentCrs, QgsProject::instance()->transformContext() );
        dataPoint = transform.transform( point );
      }
      data.insert( QStringLiteral( "point" ), dataPoint );

      if ( meters > 0 )
      {
        const QSize outputSize = QgisApp::instance()->mapCanvas()->mapSettings().outputSize();
        QgsDistanceArea da;
        da.setSourceCrs( currentCrs, QgsProject::instance()->transformContext() );
        da.setEllipsoid( QgsProject::instance()->ellipsoid() );
        const double height = da.measureLineProjected( dataPoint, meters );
        const double width = outputSize.width() * ( height / outputSize.height() );

        QgsRectangle extent;
        extent.setYMinimum( dataPoint.y() -  height / 2.0 );
        extent.setYMaximum( dataPoint.y() +  height / 2.0 );
        extent.setXMinimum( dataPoint.x() -  width / 2.0 );
        extent.setXMaximum( dataPoint.x() +  width / 2.0 );

        QgsScaleCalculator calculator;
        calculator.setMapUnits( currentCrs.mapUnits() );
        calculator.setDpi( QgisApp::instance()->mapCanvas()->mapSettings().outputDpi() );
        scale = calculator.calculate( extent, outputSize.width() );
      }

      if ( scale > 0.0 )
      {
        data.insert( QStringLiteral( "scale" ), scale );
      }

      QgsLocatorResult result;
      result.filter = this;
      result.displayString = tr( "Go to %1° %2° %3(%4)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ),
                             scale > 0.0 ? tr( "at scale 1:%1 " ).arg( scale ) : QString(),
                             wgs84Crs.userFriendlyIdentifier() );
      result.userData = data;
      result.score = 1.0;
      emit resultFetched( result );
    }
  }
}

void QgsGotoLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QgsMapCanvas *mapCanvas = QgisApp::instance()->mapCanvas();

  QVariantMap data = result.userData.toMap();
  const QgsPointXY point = data[QStringLiteral( "point" )].value<QgsPointXY>();
  mapCanvas->setCenter( point );
  if ( data.contains( QStringLiteral( "scale" ) ) )
  {
    mapCanvas->zoomScale( data[QStringLiteral( "scale" )].toDouble() );
  }
  else
  {
    mapCanvas->refresh();
  }

  mapCanvas->flashGeometries( QList< QgsGeometry >() << QgsGeometry::fromPointXY( point ) );
}
