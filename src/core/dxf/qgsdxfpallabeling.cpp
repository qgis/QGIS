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
#include "qgsmaplayerregistry.h"
#include "qgspalgeometry.h"
#include "pal/pointset.h"
#include "pal/labelposition.h"

using namespace pal;

QgsDxfPalLabeling::QgsDxfPalLabeling( QgsDxfExport* dxf, const QgsRectangle& bbox, double scale ): QgsPalLabeling(), mDxfExport( dxf )
{
  mMapRenderer.setExtent( bbox );

  //todo: adapt to other map units than meters
  int dpi = 96;
  double factor = 1000 * dpi / scale / 25.4;
  mMapRenderer.setOutputSize( QSizeF( bbox.width() * factor, bbox.height() * factor ), dpi );
  mMapRenderer.setScale( scale );
  mMapRenderer.setOutputUnits( QgsMapRenderer::Pixels );

  //mMapRenderer.setLayer necessary?
  init( &mMapRenderer );

  mImage = new QImage( 10, 10, QImage::Format_ARGB32_Premultiplied );
  mImage->setDotsPerMeterX( 96 / 25.4 * 1000 );
  mImage->setDotsPerMeterY( 96 / 25.4 * 1000 );
  mPainter = new QPainter( mImage );
  mRenderContext.setPainter( mPainter );
  mRenderContext.setRendererScale( scale );
  mRenderContext.setExtent( bbox );
  mRenderContext.setScaleFactor( 96.0 / 25.4 );
}

QgsDxfPalLabeling::~QgsDxfPalLabeling()
{
  delete mPainter;
  delete mImage;
}

void QgsDxfPalLabeling::drawLabel( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, DrawLabelType drawType, double dpiRatio )
{
  Q_UNUSED( context );
  Q_UNUSED( drawType );
  Q_UNUSED( dpiRatio );
  //debug: print label infos
  if ( mDxfExport )
  {
    //label text
    QString text = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->text();

    //layer name
    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( QString( label->getLayerName() ) );
    if ( !layer )
    {
      return;
    }
    QString layerName = mDxfExport->dxfLayerName( layer->name() );

    //angle
    double angle = label->getAlpha() * 180 / M_PI;

    //debug: show label rectangle
    /*QgsPolyline line;
    for( int i = 0; i < 4; ++i )
    {
        line.append( QgsPoint( label->getX( i ), label->getY( i ) ) );
    }
    mDxfExport->writePolyline( line, layerName, "CONTINUOUS", 1, 0.01, true );*/

    QStringList textList = text.split( "\n" );
    double textHeight = label->getHeight() / textList.size();
    QFontMetricsF fm( tmpLyr.textFont );
    double textAscent = textHeight * fm.ascent() / fm.height();

    for ( int i = 0; i < textList.size(); ++i )
    {
      mDxfExport->writeText( layerName, textList.at( i ), QgsPoint( label->getX(), label->getY() + ( textList.size() - 1 - i ) * textHeight ), textAscent, angle, mDxfExport->closestColorMatch( tmpLyr.textColor.rgb() ) );
    }
  }
}
