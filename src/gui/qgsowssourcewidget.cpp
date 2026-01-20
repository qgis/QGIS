/***************************************************************************
    qgsowssourcewidget.cpp
     --------------------------------------
    Date                 : November 2021
    Copyright            : (C) 2021 by Samweli Mwakisambwe
    Email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsowssourcewidget.h"

#include "qgsmapcanvas.h"
#include "qgsproviderregistry.h"

#include <QNetworkRequest>

#include "moc_qgsowssourcewidget.cpp"

QgsOWSSourceWidget::QgsOWSSourceWidget( const QString &providerKey, QWidget *parent )
  : QgsProviderSourceWidget( parent )
  , mProviderKey( providerKey )
{
  setupUi( this );

  QgsCoordinateReferenceSystem destinationCrs;
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  mSpatialExtentBox->setOutputCrs( crs );
}


void QgsOWSSourceWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  QgsProviderSourceWidget::setMapCanvas( canvas );

  QgsCoordinateReferenceSystem destinationCrs;
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );

  if ( mapCanvas() && mapCanvas()->mapSettings().destinationCrs().isValid() )
    destinationCrs = mapCanvas()->mapSettings().destinationCrs();
  else
    destinationCrs = crs;

  mSpatialExtentBox->setOutputCrs( destinationCrs );
  mSpatialExtentBox->setMapCanvas( mapCanvas() );
}


void QgsOWSSourceWidget::setExtent( const QgsRectangle &extent )
{
  QgsCoordinateReferenceSystem destinationCrs;
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  mSpatialExtentBox->setOutputCrs( crs );

  if ( mapCanvas() && mapCanvas()->mapSettings().destinationCrs().isValid() )
    destinationCrs = mapCanvas()->mapSettings().destinationCrs();
  else
    destinationCrs = crs;
  mSpatialExtentBox->setCurrentExtent( extent, destinationCrs );
  mSpatialExtentBox->setOutputExtentFromCurrent();
  mSpatialExtentBox->setMapCanvas( mapCanvas() );
}

QgsRectangle QgsOWSSourceWidget::extent() const
{
  return mSpatialExtentBox->outputExtent();
}

void QgsOWSSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( mProviderKey, uri );
  bool inverted = mSourceParts.value( u"InvertAxisOrientation"_s ).toBool();

  QString bbox = mSourceParts.value( u"bbox"_s ).toString();
  QgsRectangle extent;
  if ( !bbox.isEmpty() )
  {
    QStringList coords = bbox.split( ',' );
    extent = inverted ? QgsRectangle(
                          coords.at( 1 ).toDouble(),
                          coords.at( 0 ).toDouble(),
                          coords.at( 3 ).toDouble(),
                          coords.at( 2 ).toDouble()
                        )
                      : QgsRectangle(
                          coords.at( 0 ).toDouble(),
                          coords.at( 1 ).toDouble(),
                          coords.at( 2 ).toDouble(),
                          coords.at( 3 ).toDouble()
                        );
  }
  else
  {
    extent = QgsRectangle();
  }

  setExtent( extent );
  mSpatialExtentBox->setChecked( !extent.isNull() );
}

QString QgsOWSSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;

  QgsRectangle spatialExtent = extent();

  if ( mSpatialExtentBox->isChecked() && !spatialExtent.isNull() )
  {
    bool inverted = parts.value( u"InvertAxisOrientation"_s ).toBool();

    QString bbox = QString( inverted ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                     .arg( qgsDoubleToString( spatialExtent.xMinimum() ), qgsDoubleToString( spatialExtent.yMinimum() ), qgsDoubleToString( spatialExtent.xMaximum() ), qgsDoubleToString( spatialExtent.yMaximum() ) );

    parts.insert( u"bbox"_s, bbox );
  }


  return QgsProviderRegistry::instance()->encodeUri( mProviderKey, parts );
}
