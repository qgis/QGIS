/***************************************************************************
                        qgswktlocatorfilter.cpp
                        ----------------------------
    begin                : June 2025
    copyright            : (C) 2025 by Johannes Kröger
    email                : qgis at johanneskroeger dot de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswktlocatorfilter.h"
#include "moc_qgswktlocatorfilter.cpp"
#include "qgsproject.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"

#include <QClipboard>


QgsWktLocatorFilter::QgsWktLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{
  setUseWithoutPrefix( false );
}

QgsWktLocatorFilter *QgsWktLocatorFilter::clone() const
{
  return new QgsWktLocatorFilter();
}

void QgsWktLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback * )
{
  QgsLocatorResult result;
  result.filter = this;
  result.displayString = tr( "Load “%1” as layer" ).arg( string );
  result.setUserData( string );
  emit resultFetched( result );
}

void QgsWktLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QString string = result.userData().toString();

  QgsGeometry geometry = QgsGeometry::fromWkt( string );
  if ( geometry.isNull() ) {
    QgisApp::instance()->messageBar()->pushMessage( tr( "WKT could not be turned into a valid geometry." ), Qgis::MessageLevel::Warning );
    return;
  }

  QgsFeature feature = QgsFeature();
  feature.setGeometry( geometry );

  QgsCoordinateReferenceSystem crs = QgsProject::instance()->crs();
  QString geometryTypeString = QgsWkbTypes::displayString( geometry.wkbType() );
  if ( geometryTypeString.isNull() ) {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Unknown geometry type: %1" ).arg( geometryTypeString ), Qgis::MessageLevel::Warning );
    return;
  }
  QString uri = QString( "%1?crs=%2" ).arg( geometryTypeString ).arg( crs.authid() );
  QgsVectorLayer* layer = new QgsVectorLayer(uri , QStringLiteral( "WKT" ), QStringLiteral( "memory" ));
  if ( !layer->isValid() ) {
    QgisApp::instance()->messageBar()->pushMessage( tr( "WKT could not be turned into a valid layer." ), Qgis::MessageLevel::Warning );
    return;
  }

  layer->dataProvider()->addFeature( feature );
  layer->updateExtents();
  QgsProject::instance()->addMapLayer( layer );
}
