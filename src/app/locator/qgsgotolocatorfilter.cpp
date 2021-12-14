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

#include "qgsprojectionselectionwidget.h"

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
  QgsCoordinateReferenceSystem extraCrs( QStringLiteral( "EPSG:4326" ) );

  QgsSettings settings;
  QString extraCrsAuthId = extraCrs.authid();
  if ( settings.value( QStringLiteral( "locator_filters/go_to_coordinate/extra_crs" ), QStringLiteral( "EPSG:4326" ), QgsSettings::App ) != QStringLiteral( "EPSG:4326" ) )
  {
    extraCrsAuthId = settings.value( QStringLiteral( "locator_filters/go_to_coordinate/extra_crs" ), QStringLiteral( "EPSG:4326" ), QgsSettings::App ).toString();
    extraCrs = QgsCoordinateReferenceSystem( extraCrsAuthId );
  }

  bool okX = false;
  bool okY = false;
  double posX = 0.0;
  double posY = 0.0;
  const QLocale locale;

  // Coordinates such as 106.8468,-6.3804 or 200.000,450.000 using locale group- and decimal separators
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
    // Digit detection using user locale failed, use default C decimal separators,
    // no group separators, with or without comma between x,y
    // E.g. 20.1 40.1 or 20.1,40.1
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
    // Check if the string is a set of degree minute seconds (dms)
    // E.g. 48°51'29.5"N 2°17'40.2"E or 2°17'40.2"E 48°51'29.5"N
    separatorRx = QRegularExpression( QStringLiteral( "^((?:([-+nsew])\\s*)?\\d{1,3}(?:[^0-9.]+[0-5]?\\d)?[^0-9.]+[0-5]?\\d(?:\\.\\d+)?[^0-9.,]*[-+nsew]?)[,\\s]+((?:([-+nsew])\\s*)?\\d{1,3}(?:[^0-9.]+[0-5]?\\d)?[^0-9.]+[0-5]?\\d(?:\\.\\d+)?[^0-9.,]*[-+nsew]?)$" ) );
    match = separatorRx.match( string.trimmed() );
    if ( match.hasMatch() )
    {
      bool isEasting = false;
      posX = QgsCoordinateUtils::dmsToDecimal( match.captured( 1 ), &okX, &isEasting );
      posY = QgsCoordinateUtils::dmsToDecimal( match.captured( 3 ), &okY );
      if ( !isEasting )
        std::swap( posX, posY );
    }
  }

  if ( okX && okY )  // Ok found a valid looking x,y coordinate pair, check against Wgs84crs, MapCanvas-crs AND layer-crs's
  {
    // collect all crs's from all layers (plus MapCanvas crs and wgs84)
    QList<QgsCoordinateReferenceSystem> crsList;
    crsList.append( wgs84Crs );
    if ( currentCrs != wgs84Crs )
      crsList.append( currentCrs );
    // append optional extra crs (added via config)
    if ( extraCrs != wgs84Crs )
      crsList.append( extraCrs );
    // append crs's used by mapLayers
    for ( const QgsMapLayer *layer : QgsProject::instance()->mapLayers() )
    {
      if ( !crsList.contains( layer->crs() ) )
      {
        crsList.append( layer->crs() );
      }
    }
    // Now go over all crs's and check if the coordinate COULD be use in that crs
    for ( const QgsCoordinateReferenceSystem &crs : std::as_const( crsList ) )
    {
      QVariantMap data;
      const QgsPointXY point( posX, posY );
      data.insert( QStringLiteral( "point" ), point );

      QgsLocatorResult result;
      result.filter = this;

      // create a transform to be able to check point against bounds
      QgsCoordinateTransform transform2wgs84( crs, wgs84Crs, QgsProject::instance()->transformContext() );
      transform2wgs84.setBallparkTransformsAreAppropriate( true );
      QgsPointXY wgs84Point;
      try
      {
        wgs84Point = transform2wgs84.transform( point );
      }
      catch ( const QgsException &e )
      {
        Q_UNUSED( e )
        // for testing purposes: show when a potential crs coordinate is skipped: show the resultstring
        //result.displayString = tr( "Transform exception: NOT Going to %1 %2 (Map CRS, %3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), crs.userFriendlyIdentifier() );emit resultFetched( result );
        continue;
      }

      if ( crs == currentCrs )
      {
        if ( crs.bounds().contains( wgs84Point ) )
        {
          // data.point is already set to this crs
          if ( crs == wgs84Crs )
            result.displayString = tr( "Go to %1° %2° (Map CRS %3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), crs.userFriendlyIdentifier() );
          else
            result.displayString = tr( "Go to %1 %2 (Map CRS, %3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), crs.userFriendlyIdentifier() );
          result.score = 0.9;
        }
        else   // (potential) currentCrs coordinate is tested outside bounds of currentCrs
        {
          // for testing purposes: show when a potential crs coordinate is skipped: show the resultstring
          //result.displayString = tr( "NOT Going to %1 %2 (Map CRS, %3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), crs.userFriendlyIdentifier() );emit resultFetched( result );
          continue;
        }
      }
      else    // crs != currentCrs ==> transform, BUT only show if within crs bounds
      {
        if ( crs.bounds().contains( wgs84Point ) )
        {
          const QgsCoordinateTransform transform2mapcanvas( crs, currentCrs, QgsProject::instance()->transformContext() );
          QgsPointXY transformedPoint;
          try
          {
            transformedPoint = transform2mapcanvas.transform( point );
          }
          catch ( const QgsException &e )
          {
            Q_UNUSED( e )
            // for testing purposes: show when a potential crs coordinate is skipped: show the resultstring
            //result.displayString = tr( "Transform exception: NOT Going to %1 %2 (%3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), crs.userFriendlyIdentifier() );emit resultFetched( result );
            continue;
          }
          if ( crs == wgs84Crs )
            result.displayString = tr( "Go to %1° %2° (%3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), crs.userFriendlyIdentifier() );
          else
            result.displayString = tr( "Go to %1 %2 (%3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), crs.userFriendlyIdentifier() );
          result.score = 0.85;
          data[QStringLiteral( "point" )] = transformedPoint;
        }
        else
        {
          // for testing purposes: show when a potential crs coordinate is skipped: show the resultstring
          //result.displayString = tr( "NOT Going to %1 %2 (Layer CRS, %3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), crs.userFriendlyIdentifier() );emit resultFetched( result );
          continue;
        }
      }
      result.userData = data;
      emit resultFetched( result );

    }
    return;
  }

  // No valid looking coordinate pair found, going to check for url patterns of (google/osm) tiling services

  // Scales for EPSG:3857 tiling services
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
      // Check for OSM/Leaflet/OpenLayers url/link pattern e.g. Eiffel tower Paris at zoom 18:
      // http://www.openstreetmap.org/#map=18/48.8582/2.2945
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

    if ( !okX && !okY )  // No valid x,y coordinate pair found yet
    {
      // Check for Google maps url/link pattern e.g. Eiffel tower Paris:
      // https://www.google.com/maps/place/48%C2%B051'29.5%22N+2%C2%B017'40.2%22E/@48.8582035,2.2923113,17z/data=
      // Same but streetview:
      // https://www.google.com/maps/place/48%C2%B051'29.5%22N+2%C2%B017'40.2%22E/@48.8582035,2.2923113,916m/data=
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

    if ( okX && okY )  // so X,Y from some kind of tiling service url, check for scale/z part
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


void QgsGotoLocatorFilter::openConfigWidget( QWidget *parent )
{
  QString key = "locator_filters/go_to_coordinate";
  QgsSettings settings;
  std::unique_ptr<QDialog> dlg( new QDialog( parent ) );
  dlg->restoreGeometry( settings.value( QStringLiteral( "Windows/%1/geometry" ).arg( key ) ).toByteArray() );
  dlg->setWindowTitle( tr( "Go To Coordinate Locator" ) );
  QFormLayout *formLayout = new QFormLayout;

  QgsProjectionSelectionWidget *crsSelection = new QgsProjectionSelectionWidget( dlg.get() );
  if ( settings.value( QStringLiteral( "%1/extra_crs" ).arg( key ), QVariant( false ), QgsSettings::App ) == QVariant( false ) )
  {
    crsSelection->setNotSetText( tr( "Not set. Please select one..." ) );
    crsSelection->setOptionVisible( QgsProjectionSelectionWidget::CrsOption::CrsNotSet, true );
  }
  else
  {
    crsSelection->setCrs( QgsCoordinateReferenceSystem( settings.value( QStringLiteral( "%1/extra_crs" ).arg( key ), QStringLiteral( "EPSG:4326" ), QgsSettings::App ).toString() ) );
  }
  crsSelection->setMinimumSize( 350, 5 );

  formLayout->addRow( new QLabel( tr( "Using Project and Layer Crs's" ) ) );
  formLayout->addRow( tr( "&Select Extra Crs:" ), crsSelection );

  QDialogButtonBox *buttonbBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg.get() );
  formLayout->addRow( buttonbBox );
  dlg->setLayout( formLayout );
  connect( buttonbBox, &QDialogButtonBox::accepted, [&]()
  {
    settings.setValue( QStringLiteral( "%1/extra_crs" ).arg( key ), crsSelection->crs().authid(), QgsSettings::App );
    dlg->accept();
  } );
  connect( buttonbBox, &QDialogButtonBox::rejected, dlg.get(), &QDialog::reject );

  dlg->exec();
}
