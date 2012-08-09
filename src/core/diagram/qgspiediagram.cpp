/***************************************************************************
    qgsdiagram.cpp
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
  double value = attIt.value().toDouble();

  //interpolate size
  double ratio = ( value - is.lowerValue ) / ( is.upperValue - is.lowerValue );
  QSizeF size = QSizeF( sqrt( is.upperSize.width() * ratio + is.lowerSize.width() * ( 1 - ratio ) ),
                        sqrt( is.upperSize.height() * ratio + is.lowerSize.height() * ( 1 - ratio ) ) );

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

  QList<int>::const_iterator catIt = s.categoryIndices.constBegin();
  for ( ; catIt != s.categoryIndices.constEnd(); ++catIt )
  {
    currentVal = att[*catIt].toDouble();
    values.push_back( currentVal );
    valSum += currentVal;
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

  QList<double>::const_iterator valIt = values.constBegin();
  QList< QColor >::const_iterator colIt = s.categoryColors.constBegin();
  for ( ; valIt != values.constEnd(); ++valIt, ++colIt )
  {
    currentAngle =  *valIt / valSum * 360 * 16;
    mCategoryBrush.setColor( *colIt );
    p->setBrush( mCategoryBrush );
    p->drawPie( baseX, baseY, w, h, totalAngle, currentAngle );
    totalAngle += currentAngle;
  }
}