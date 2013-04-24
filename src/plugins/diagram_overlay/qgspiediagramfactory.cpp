/***************************************************************************
                         qgspiediagramfactory.cpp  -  description
                         ------------------------
    begin                : December 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspiediagramfactory.h"
#include "qgsrendercontext.h"

QgsPieDiagramFactory::QgsPieDiagramFactory(): QgsWKNDiagramFactory()
{

}

QgsPieDiagramFactory::~QgsPieDiagramFactory()
{

}

QImage* QgsPieDiagramFactory::createDiagram( int size, const QgsFeature& f, const QgsRenderContext& renderContext ) const
{
  const QgsAttributes& dataValues = f.attributes();
  double sizeScaleFactor = diagramSizeScaleFactor( renderContext );

  //create transparent QImage
  int imageSideLength = size * sizeScaleFactor * renderContext.rasterScaleFactor() + 2 * mMaximumPenWidth + 2 * mMaximumGap;
  QImage* diagramImage = new QImage( QSize( imageSideLength, imageSideLength ), QImage::Format_ARGB32_Premultiplied );
  diagramImage->fill( qRgba( 0, 0, 0, 0 ) ); //transparent background
  QPainter p;
  p.begin( diagramImage );
  p.setRenderHint( QPainter::Antialiasing );
  p.setPen( Qt::NoPen );

  //calculate sum of data values
  double sum = 0;
  QList<double> valueList; //cash the values to use them in drawing later

  QList<QgsDiagramCategory>::const_iterator it = mCategories.constBegin();
  for ( ; it != mCategories.constEnd(); ++it )
  {
    int idx = it->propertyIndex();
    if ( idx >= 0 && idx < dataValues.count() )
    {
      valueList.push_back( dataValues[idx].toDouble() );
      sum += dataValues[idx].toDouble();
    }
  }

  if ( qgsDoubleNear( sum, 0.0 ) )
  {
    p.end();
    delete diagramImage;
    return 0;
  }

  //draw pies

  int totalAngle = 0;
  int currentAngle, currentGap;
  int xGapOffset = 0;
  int yGapOffset = 0;

  QList<QgsDiagramCategory>::const_iterator category_it = mCategories.constBegin();
  QList<double>::const_iterator valueList_it = valueList.constBegin();

  for ( ; category_it != mCategories.constEnd() && valueList_it != valueList.constEnd(); ++category_it, ++valueList_it )
  {
    p.setPen( category_it->pen() );
    currentAngle = ( int )(( *valueList_it ) / sum * 360 * 16 );
    p.setBrush( category_it->brush() );

    xGapOffset = 0;
    yGapOffset = 0;
    currentGap = category_it->gap();
    if ( currentGap != 0 )
    {
      //qt angles are degrees*16
      gapOffsetsForPieSlice( currentGap, totalAngle + currentAngle / 2, xGapOffset, yGapOffset );
    }

    p.drawPie( mMaximumPenWidth * renderContext.rasterScaleFactor() + mMaximumGap + xGapOffset, mMaximumPenWidth * renderContext.rasterScaleFactor() + mMaximumGap - yGapOffset, sizeScaleFactor * renderContext.rasterScaleFactor() * size, sizeScaleFactor * renderContext.rasterScaleFactor() * size, totalAngle, currentAngle );
    totalAngle += currentAngle;
  }
  p.end();

  return diagramImage;
}

int QgsPieDiagramFactory::getDiagramDimensions( int size, const QgsFeature& f, const QgsRenderContext& context, int& width, int& height ) const
{
  Q_UNUSED( f );
  double squareSide = size * diagramSizeScaleFactor( context ) * context.rasterScaleFactor() + 2 * mMaximumPenWidth + 2 * mMaximumGap;
  width = squareSide;
  height = squareSide;
  return 0;
}

int QgsPieDiagramFactory::gapOffsetsForPieSlice( int gap, int angle, int& xOffset, int& yOffset ) const
{
  double rad = angle / 2880.0 * M_PI;
  xOffset = ( int )( cos( rad ) * gap );
  yOffset = ( int )( sin( rad ) * gap );

  return 0;
}
