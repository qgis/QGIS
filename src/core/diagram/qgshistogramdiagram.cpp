/***************************************************************************
    qgshistogramdiagram.cpp
    ---------------------
    begin                : August 2012
    copyright            : (C) 2012 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgshistogramdiagram.h"

#include "qgsdiagramrenderer.h"
#include "qgsexpression.h"
#include "qgslinesymbol.h"
#include "qgsrendercontext.h"
#include "qgssymbollayerutils.h"

#include <QPainter>
#include <QString>

using namespace Qt::StringLiterals;

const QString QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM = u"Histogram"_s;

QgsHistogramDiagram::QgsHistogramDiagram()
{
  mCategoryBrush.setStyle( Qt::SolidPattern );
  mPen.setStyle( Qt::SolidLine );
  mScaleFactor = 0;
}

QgsHistogramDiagram *QgsHistogramDiagram::clone() const
{
  return new QgsHistogramDiagram( *this );
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is )
{
  QSizeF size;
  if ( feature.attributeCount() == 0 )
  {
    return size; //zero size if no attributes
  }

  if ( qgsDoubleNear( is.upperValue, is.lowerValue ) )
    return size; // invalid value range => zero size

  double maxValue = 0;
  double minValue = 0;
  double tempValue = 0;
  double valueRange = 0;

  QgsExpressionContext expressionContext = c.expressionContext();
  expressionContext.setFeature( feature );
  if ( !feature.fields().isEmpty() )
    expressionContext.setFields( feature.fields() );

  for ( const QString &cat : std::as_const( s.categoryAttributes ) )
  {
    QgsExpression *expression = getExpression( cat, expressionContext );
    tempValue = expression->evaluate( &expressionContext ).toDouble();
    maxValue = std::max( tempValue, maxValue );
    minValue = std::min( tempValue, minValue );
  }

  // Account for negative values
  valueRange = maxValue + ( minValue * -1 );

  // Scale, if extension is smaller than the specified minimum
  if ( valueRange < s.minimumSize )
  {
    valueRange = s.minimumSize;
  }

  // eh - this method returns size in unknown units ...! We'll have to fake it and use a rough estimation of
  // a conversion factor to painter units...
  // TODO QGIS 5.0 -- these methods should all use painter units, dependent on the render context scaling...
  double painterUnitConversionScale = c.convertToPainterUnits( 1, s.sizeType );

  const double spacing = c.convertToPainterUnits( s.spacing(), s.spacingUnit(), s.spacingMapUnitScale() ) / painterUnitConversionScale;

  switch ( s.diagramOrientation )
  {
    case QgsDiagramSettings::Up:
    case QgsDiagramSettings::Down:
      mScaleFactor = ( ( is.upperSize.width() - is.lowerSize.height() ) / ( is.upperValue - is.lowerValue ) );
      size.scale( s.barWidth * s.categoryAttributes.size() + spacing * std::max( 0, static_cast<int>( s.categoryAttributes.size() ) - 1 ), valueRange * mScaleFactor, Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
      mScaleFactor = ( ( is.upperSize.width() - is.lowerSize.width() ) / ( is.upperValue - is.lowerValue ) );
      size.scale( valueRange * mScaleFactor, s.barWidth * s.categoryAttributes.size() + spacing * std::max( 0, static_cast<int>( s.categoryAttributes.size() ) - 1 ), Qt::IgnoreAspectRatio );
      break;
  }

  if ( s.showAxis() && s.axisLineSymbol() )
  {
    const double maxBleed = QgsSymbolLayerUtils::estimateMaxSymbolBleed( s.axisLineSymbol(), c ) / painterUnitConversionScale;
    size.setWidth( size.width() + 2 * maxBleed );
    size.setHeight( size.height() + 2 * maxBleed );
  }

  return size;
}

double QgsHistogramDiagram::legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const
{
  if ( qgsDoubleNear( is.upperValue, is.lowerValue ) )
    return s.minimumSize; // invalid value range => zero size

  // Scale, if extension is smaller than the specified minimum
  if ( value < s.minimumSize )
  {
    value = s.minimumSize;
  }

  double scaleFactor = ( ( is.upperSize.width() - is.lowerSize.width() ) / ( is.upperValue - is.lowerValue ) );
  return value * scaleFactor;
}

QString QgsHistogramDiagram::diagramName() const
{
  return QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM;
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsAttributes &, const QgsRenderContext &, const QgsDiagramSettings & )
{
  // Since histograms only support interpolated size,
  // we only keep this method for compatibility reasons.
  return QSizeF();
}

void QgsHistogramDiagram::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position )
{
  QPainter *p = c.painter();
  if ( !p )
  {
    return;
  }

  QList<double> values;
  double maxValue = 0;
  double minValue = 0;

  QgsExpressionContext expressionContext = c.expressionContext();
  expressionContext.setFeature( feature );
  if ( !feature.fields().isEmpty() )
    expressionContext.setFields( feature.fields() );

  values.reserve( s.categoryAttributes.size() );
  for ( const QString &cat : std::as_const( s.categoryAttributes ) )
  {
    QgsExpression *expression = getExpression( cat, expressionContext );
    double currentVal = expression->evaluate( &expressionContext ).toDouble();
    values.push_back( currentVal );
    maxValue = std::max( currentVal, maxValue );
    minValue = std::min( currentVal, minValue );
  }

  double scaledMaxVal = sizePainterUnits( maxValue * mScaleFactor, s, c );
  double scaledMinVal = sizePainterUnits( minValue * mScaleFactor, s, c ) * -1;

  double currentOffset = 0;
  double scaledWidth = sizePainterUnits( s.barWidth, s, c );

  const double spacing = c.convertToPainterUnits( s.spacing(), s.spacingUnit(), s.spacingMapUnitScale() );

  double baseX = position.x();
  double baseY = position.y();

  if ( s.showAxis() && s.axisLineSymbol() )
  {
    // if showing axis, the diagram position needs shifting from the default base x so that the axis
    // line stroke sits within the desired label engine rect (otherwise we risk overlaps of the axis line stroke)
    const double maxBleed = QgsSymbolLayerUtils::estimateMaxSymbolBleed( s.axisLineSymbol(), c );
    baseX += maxBleed;
    baseY -= maxBleed;
  }

  // Special bases
  double baseYTop = baseY - scaledMinVal;
  double baseYDown = baseY - scaledMaxVal;
  double baseXRight = baseX + scaledMinVal;
  double baseXLeft = baseX + scaledMaxVal;

  mPen.setColor( s.penColor );
  setPenWidth( mPen, s, c );
  p->setPen( mPen );

  QList<double>::const_iterator valIt = values.constBegin();
  QList< QColor >::const_iterator colIt = s.categoryColors.constBegin();
  for ( ; valIt != values.constEnd(); ++valIt, ++colIt )
  {
    double length = sizePainterUnits( *valIt * mScaleFactor, s, c );

    QColor brushColor( *colIt );
    brushColor.setAlphaF( brushColor.alphaF() * s.opacity );
    mCategoryBrush.setColor( brushColor );
    p->setBrush( mCategoryBrush );

    switch ( s.diagramOrientation )
    {
      case QgsDiagramSettings::Up:
        p->drawRect( QRectF( baseX + currentOffset, baseYTop, scaledWidth, length * -1 ) );
        break;

      case QgsDiagramSettings::Down:
        p->drawRect( QRectF( baseX + currentOffset, baseYDown, scaledWidth, length ) );
        break;

      case QgsDiagramSettings::Right:
        p->drawRect( QRectF( baseXRight, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) + currentOffset, length, scaledWidth ) );
        break;

      case QgsDiagramSettings::Left:
        p->drawRect( QRectF( baseXLeft, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) + currentOffset, 0 - length, scaledWidth ) );
        break;
    }

    currentOffset += scaledWidth + spacing;
  }

  if ( s.showAxis() && s.axisLineSymbol() )
  {
    s.axisLineSymbol()->startRender( c );
    QPolygonF axisPoints;
    switch ( s.diagramOrientation )
    {
      case QgsDiagramSettings::Up:
        axisPoints << QPointF( baseX, baseYTop - scaledMaxVal )
                   << QPointF( baseX, baseYTop )
                   << QPointF( baseX + scaledWidth * values.size() + spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ), baseYTop );
        break;

      case QgsDiagramSettings::Down:
        axisPoints << QPointF( baseX, baseYDown + scaledMaxVal )
                   << QPointF( baseX, baseYDown )
                   << QPointF( baseX + scaledWidth * values.size() + spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ), baseYDown );
        break;

      case QgsDiagramSettings::Right:
        axisPoints << QPointF( baseXRight + scaledMaxVal, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) )
                   << QPointF( baseXRight, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) )
                   << QPointF( baseXRight, baseY );
        break;

      case QgsDiagramSettings::Left:
        axisPoints << QPointF( baseXLeft - scaledMaxVal, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) )
                   << QPointF( baseXLeft, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) )
                   << QPointF( baseXLeft, baseY );
        break;
    }

    s.axisLineSymbol()->renderPolyline( axisPoints, nullptr, c );
    s.axisLineSymbol()->stopRender( c );
  }
}
