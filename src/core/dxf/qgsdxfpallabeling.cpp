/***************************************************************************
                         qgsdxfpallabeling.cpp
                         ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfpallabeling.h"
#include "qgsdxfexport.h"
#include "qgspalgeometry.h"
#include "qgsmapsettings.h"

#include "pal/pointset.h"
#include "pal/labelposition.h"

using namespace pal;

QgsDxfPalLabeling::QgsDxfPalLabeling( QgsDxfExport* dxf, const QgsRectangle& bbox, double scale, QGis::UnitType mapUnits )
    : QgsPalLabeling()
    , mDxfExport( dxf )
    , mImage( 0 )
    , mPainter( 0 )
{
  mSettings = new QgsMapSettings;
  mSettings->setMapUnits( mapUnits );
  mSettings->setExtent( bbox );

  int dpi = 96;
  double factor = 1000 * dpi / scale / 25.4 * QGis::fromUnitToUnitFactor( mapUnits, QGis::Meters );
  mSettings->setOutputSize( QSize( bbox.width() * factor, bbox.height() * factor ) );
  mSettings->setOutputDpi( dpi );
  mSettings->setCrsTransformEnabled( false );
  init( *mSettings );

  mImage = new QImage( 10, 10, QImage::Format_ARGB32_Premultiplied );
  mImage->setDotsPerMeterX( 96 / 25.4 * 1000 );
  mImage->setDotsPerMeterY( 96 / 25.4 * 1000 );
  mPainter = new QPainter( mImage );
  mRenderContext.setPainter( mPainter );
  mRenderContext.setRendererScale( scale );
  mRenderContext.setExtent( bbox );
  mRenderContext.setScaleFactor( 96.0 / 25.4 );
  mRenderContext.setMapToPixel( QgsMapToPixel( 1.0 / factor, bbox.xMinimum(), bbox.yMinimum(), bbox.height() * factor ) );
}

QgsDxfPalLabeling::~QgsDxfPalLabeling()
{
  delete mPainter;
  delete mImage;
  delete mSettings;
}

void QgsDxfPalLabeling::drawLabel( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, DrawLabelType drawType, double dpiRatio )
{
  Q_UNUSED( context );
  Q_UNUSED( drawType );
  Q_UNUSED( dpiRatio );

  if ( drawType == QgsPalLabeling::LabelBuffer )
  {
    return;
  }

  //debug: print label infos
  if ( mDxfExport )
  {
    QgsPalGeometry *g = dynamic_cast< QgsPalGeometry* >( label->getFeaturePart()->getUserGeometry() );

    //label text
    QString text = g->text();

    //angle
    double angle = label->getAlpha() * 180 / M_PI;

    //debug: show label rectangle
#if 0
    QgsPolyline line;
    for ( int i = 0; i < 4; ++i )
    {
      line.append( QgsPoint( label->getX( i ), label->getY( i ) ) );
    }
    mDxfExport->writePolyline( line, g->dxfLayer(), "CONTINUOUS", 1, 0.01, true );
#endif
    text = text.replace( tmpLyr.wrapChar.isEmpty() ? "\n" : tmpLyr.wrapChar, "\\P" );

    text.prepend( QString( "\\f%1|i%2|b%3;\\H%4;\\W0.5;" )
                  .arg( tmpLyr.textFont.family() )
                  .arg( tmpLyr.textFont.italic() ? 1 : 0 )
                  .arg( tmpLyr.textFont.bold() ? 1 : 0 )
                  .arg( label->getHeight() / ( 1 + text.count( "\\P" ) ) * 0.75 ) );

    mDxfExport->writeMText( g->dxfLayer(), text, QgsPoint( label->getX(), label->getY() ), label->getWidth() * 1.1, angle, tmpLyr.textColor );
  }
}
