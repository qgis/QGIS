/***************************************************************************
    qgspiediagram.cpp
    ---------------------
    begin                : March 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspiediagram.h"
#include "qgsdiagramrendererv2.h"
#include "qgsrendercontext.h"

#include <QPainter>


QgsPieDiagram::QgsPieDiagram()
{
  mCategoryBrush.setStyle( Qt::SolidPattern );
  mPen.setStyle( Qt::SolidLine );
}

QgsPieDiagram::~QgsPieDiagram()
{
}

QSizeF QgsPieDiagram::diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is )
{
  QgsAttributeMap::const_iterator attIt = attributes.find( is.classificationAttribute );
  if ( attIt == attributes.constEnd() )
  {
    return QSizeF(); //zero size if attribute is missing
  }
  
  double scaledValue = attIt.value().toDouble();
  double scaledLowerValue = is.lowerValue;
  double scaledUpperValue = is.upperValue;
  double scaledLowerSizeWidth = is.lowerSize.width();
  double scaledLowerSizeHeight = is.lowerSize.height();
  double scaledUpperSizeWidth = is.upperSize.width();
  double scaledUpperSizeHeight = is.upperSize.height();

  // interpolate the squared value if scale by area
  if ( s.scaleByArea )
  {
    scaledValue = sqrt( scaledValue );
    scaledLowerValue = sqrt( scaledLowerValue );
    scaledUpperValue = sqrt( scaledUpperValue );
    scaledLowerSizeWidth = sqrt( scaledLowerSizeWidth );
    scaledLowerSizeHeight = sqrt( scaledLowerSizeHeight );
    scaledUpperSizeWidth = sqrt( scaledUpperSizeWidth );
    scaledUpperSizeHeight = sqrt( scaledUpperSizeHeight );
  }

  //interpolate size
  double scaledRatio = ( scaledValue - scaledLowerValue ) / ( scaledUpperValue - scaledLowerValue );

  QSizeF size = QSizeF( is.upperSize.width() * scaledRatio + is.lowerSize.width() * ( 1 - scaledRatio ),
                        is.upperSize.height() * scaledRatio + is.lowerSize.height() * ( 1 - scaledRatio ) );

  // Scale, if extension is smaller than the specified minimum
  if ( size.width() <= s.minimumSize && size.height() <= s.minimumSize )
  {
    size.scale( s.minimumSize, s.minimumSize, Qt::KeepAspectRatio );
  }

  return size;
}

QSizeF QgsPieDiagram::diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s )
{
  return s.size;
}

void QgsPieDiagram::renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )
{
  QPainter* p = c.painter();
  if ( !p )
  {
    return;
  }

  //get sum of values
  QList<double> values;
  double currentVal = 0;
  double valSum = 0;
  int valCount = 0;

  QList<int>::const_iterator catIt = s.categoryIndices.constBegin();
  for ( ; catIt != s.categoryIndices.constEnd(); ++catIt )
  {
    currentVal = att[*catIt].toDouble();
    values.push_back( currentVal );
    valSum += currentVal;
    if ( currentVal ) valCount++;
  }

  //draw the slices
  double totalAngle = 0;
  double currentAngle;

  //convert from mm / map units to painter units
  QSizeF spu = sizePainterUnits( s.size, s, c );
  double w = spu.width();
  double h = spu.height();

  double baseX = position.x();
  double baseY = position.y() - h;

  mPen.setColor( s.penColor );
  setPenWidth( mPen, s, c );
  p->setPen( mPen );

  // there are some values > 0 available
  if ( valSum > 0 )
  {
    QList<double>::const_iterator valIt = values.constBegin();
    QList< QColor >::const_iterator colIt = s.categoryColors.constBegin();
    for ( ; valIt != values.constEnd(); ++valIt, ++colIt )
    {
      currentAngle =  *valIt / valSum * 360 * 16;
      mCategoryBrush.setColor( *colIt );
      p->setBrush( mCategoryBrush );
      // if only 1 value is > 0, draw a circle
      if ( valCount == 1 )
      {
        p->drawEllipse( baseX, baseY, w, h );
      }
      else
      {
        p->drawPie( baseX, baseY, w, h, totalAngle, currentAngle );
      }
      totalAngle += currentAngle;
    }
  }
  else // valSum > 0
  {
    // draw empty circle if no values are defined at all
    mCategoryBrush.setColor( Qt::transparent );
    p->setBrush( mCategoryBrush );
    p->drawEllipse( baseX, baseY, w, h );
  }
}
