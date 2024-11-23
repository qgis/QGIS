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
#include "qgsdiagramrenderer.h"
#include "qgsrendercontext.h"
#include "qgsexpression.h"

#include <QPainter>

const QString QgsPieDiagram::DIAGRAM_NAME_PIE = QStringLiteral( "Pie" );

QgsPieDiagram::QgsPieDiagram()
{
  mCategoryBrush.setStyle( Qt::SolidPattern );
  mPen.setStyle( Qt::SolidLine );
}

QgsPieDiagram *QgsPieDiagram::clone() const
{
  return new QgsPieDiagram( *this );
}

QSizeF QgsPieDiagram::diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is )
{
  Q_UNUSED( c )

  QVariant attrVal;
  if ( is.classificationAttributeIsExpression )
  {
    QgsExpressionContext expressionContext = c.expressionContext();
    if ( !feature.fields().isEmpty() )
      expressionContext.setFields( feature.fields() );
    expressionContext.setFeature( feature );

    QgsExpression *expression = getExpression( is.classificationAttributeExpression, expressionContext );
    attrVal = expression->evaluate( &expressionContext );
  }
  else
  {
    attrVal = feature.attribute( is.classificationField );
  }

  bool ok = false;
  const double value = attrVal.toDouble( &ok );
  if ( !ok )
  {
    return QSizeF(); //zero size if attribute is missing
  }

  return sizeForValue( value, s, is );
}

double QgsPieDiagram::legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const
{
  const QSizeF size = sizeForValue( value, s, is );
  return std::max( size.width(), size.height() );
}

QString QgsPieDiagram::diagramName() const
{
  return QgsPieDiagram::DIAGRAM_NAME_PIE;
}

QSizeF QgsPieDiagram::diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s )
{
  Q_UNUSED( c )
  Q_UNUSED( attributes )
  return s.size;
}

void QgsPieDiagram::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position )
{
  QPainter *p = c.painter();
  if ( !p )
  {
    return;
  }

  //get sum of values
  QList<double> values;
  double currentVal = 0;
  double valSum = 0;
  int valCount = 0;

  QgsExpressionContext expressionContext = c.expressionContext();
  expressionContext.setFeature( feature );
  if ( !feature.fields().isEmpty() )
    expressionContext.setFields( feature.fields() );

  QList<QString>::const_iterator catIt = s.categoryAttributes.constBegin();
  for ( ; catIt != s.categoryAttributes.constEnd(); ++catIt )
  {
    QgsExpression *expression = getExpression( *catIt, expressionContext );
    currentVal = expression->evaluate( &expressionContext ).toDouble();
    values.push_back( currentVal );
    valSum += currentVal;
    if ( currentVal ) valCount++;
  }

  //draw the slices
  double totalAngle = 0;
  double currentAngle;

  //convert from mm / map units to painter units
  const QSizeF spu = sizePainterUnits( s.size, s, c );
  const double w = spu.width();
  const double h = spu.height();

  const double baseX = position.x();
  const double baseY = position.y() - h;

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
      if ( *valIt )
      {
        currentAngle = ( *valIt / valSum * 360 * 16 ) * ( s.direction() == QgsDiagramSettings::Clockwise ? -1 : 1 );
        QColor brushColor( *colIt );
        brushColor.setAlphaF( brushColor.alphaF() * s.opacity );
        mCategoryBrush.setColor( brushColor );
        p->setBrush( mCategoryBrush );
        // if only 1 value is > 0, draw a circle
        if ( valCount == 1 )
        {
          p->drawEllipse( QRectF( baseX, baseY, w, h ) );
        }
        else
        {
          p->drawPie( QRectF( baseX, baseY, w, h ), totalAngle - s.rotationOffset * 16.0, currentAngle );
        }
        totalAngle += currentAngle;
      }
    }
  }
  else // valSum > 0
  {
    // draw empty circle if no values are defined at all
    mCategoryBrush.setColor( Qt::transparent );
    p->setBrush( mCategoryBrush );
    p->drawEllipse( QRectF( baseX, baseY, w, h ) );
  }
}
