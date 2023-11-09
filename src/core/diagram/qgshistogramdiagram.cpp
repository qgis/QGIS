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
#include "qgsrendercontext.h"
#include "qgsexpression.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbol.h"

#include <QPainter>

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
  if ( feature.attributes().isEmpty() )
  {
    return size; //zero size if no attributes
  }

  if ( qgsDoubleNear( is.upperValue, is.lowerValue ) )
    return size; // invalid value range => zero size

  double maxValue = 0;

  QgsExpressionContext expressionContext = c.expressionContext();
  expressionContext.setFeature( feature );
  if ( !feature.fields().isEmpty() )
    expressionContext.setFields( feature.fields() );

  for ( const QString &cat : std::as_const( s.categoryAttributes ) )
  {
    QgsExpression *expression = getExpression( cat, expressionContext );
    maxValue = std::max( expression->evaluate( &expressionContext ).toDouble(), maxValue );
  }

  // Scale, if extension is smaller than the specified minimum
  if ( maxValue < s.minimumSize )
  {
    maxValue = s.minimumSize;
  }

  // eh - this method returns size in unknown units ...! We'll have to fake it and use a rough estimation of
  // a conversion factor to painter units...
  // TODO QGIS 4.0 -- these methods should all use painter units, dependent on the render context scaling...
  double painterUnitConversionScale = c.convertToPainterUnits( 1, s.sizeType );

  const double spacing = c.convertToPainterUnits( s.spacing(), s.spacingUnit(), s.spacingMapUnitScale() ) / painterUnitConversionScale;

  switch ( s.diagramOrientation )
  {
    case QgsDiagramSettings::Up:
    case QgsDiagramSettings::Down:
      mScaleFactor = ( ( is.upperSize.width() - is.lowerSize.height() ) / ( is.upperValue - is.lowerValue ) );
      size.scale( s.barWidth * s.categoryAttributes.size() + spacing * std::max( 0, static_cast<int>( s.categoryAttributes.size() ) - 1 ), maxValue * mScaleFactor, Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
      mScaleFactor = ( ( is.upperSize.width() - is.lowerSize.width() ) / ( is.upperValue - is.lowerValue ) );
      size.scale( maxValue * mScaleFactor, s.barWidth * s.categoryAttributes.size() + spacing * std::max( 0, static_cast<int>( s.categoryAttributes.size() ) - 1 ), Qt::IgnoreAspectRatio );
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
  return DIAGRAM_NAME_HISTOGRAM;
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s )
{
  Q_UNUSED( c )
  QSizeF size;

  if ( attributes.isEmpty() )
  {
    return QSizeF(); //zero size if no attributes
  }

  double maxValue = attributes.at( 0 ).toDouble();

  for ( int i = 0; i < attributes.count(); ++i )
  {
    maxValue = std::max( attributes.at( i ).toDouble(), maxValue );
  }

  // eh - this method returns size in unknown units ...! We'll have to fake it and use a rough estimation of
  // a conversion factor to painter units...
  // TODO QGIS 4.0 -- these methods should all use painter units, dependent on the render context scaling...
  double painterUnitConversionScale = c.convertToPainterUnits( 1, s.sizeType );

  const double spacing = c.convertToPainterUnits( s.spacing(), s.spacingUnit(), s.spacingMapUnitScale() ) / painterUnitConversionScale;

  switch ( s.diagramOrientation )
  {
    case QgsDiagramSettings::Up:
    case QgsDiagramSettings::Down:
      mScaleFactor = maxValue / s.size.height();
      size.scale( s.barWidth * s.categoryColors.size() + spacing * std::max( 0, static_cast<int>( s.categoryAttributes.size() ) - 1 ), s.size.height(), Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
      mScaleFactor = maxValue / s.size.width();
      size.scale( s.size.width(), s.barWidth * s.categoryColors.size() + spacing * std::max( 0, static_cast<int>( s.categoryAttributes.size() ) - 1 ), Qt::IgnoreAspectRatio );
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

void QgsHistogramDiagram::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position )
{
  QPainter *p = c.painter();
  if ( !p )
  {
    return;
  }

  QList<double> values;
  double maxValue = 0;

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
  }

  double scaledMaxVal = sizePainterUnits( maxValue * mScaleFactor, s, c );

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
        p->drawRect( QRectF( baseX + currentOffset, baseY, scaledWidth, length * -1 ) );
        break;

      case QgsDiagramSettings::Down:
        p->drawRect( QRectF( baseX + currentOffset, baseY - scaledMaxVal, scaledWidth, length ) );
        break;

      case QgsDiagramSettings::Right:
        p->drawRect( QRectF( baseX, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) + currentOffset, length, scaledWidth ) );
        break;

      case QgsDiagramSettings::Left:
        p->drawRect( QRectF( baseX + scaledMaxVal, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) + currentOffset, 0 - length, scaledWidth ) );
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
        axisPoints << QPointF( baseX, baseY - scaledMaxVal ) << QPointF( baseX, baseY ) << QPointF( baseX + scaledWidth * values.size() + spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ), baseY );
        break;

      case QgsDiagramSettings::Down:
        axisPoints << QPointF( baseX, baseY ) << QPointF( baseX, baseY - scaledMaxVal ) << QPointF( baseX + scaledWidth * values.size() + spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ), baseY - scaledMaxVal );
        break;

      case QgsDiagramSettings::Right:
        axisPoints << QPointF( baseX + scaledMaxVal, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) )
                   << QPointF( baseX, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) )
                   << QPointF( baseX, baseY );
        break;

      case QgsDiagramSettings::Left:
        axisPoints << QPointF( baseX, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) )
                   << QPointF( baseX + scaledMaxVal, baseY - scaledWidth * values.size() - spacing * std::max( 0, static_cast<int>( values.size() ) - 1 ) )
                   << QPointF( baseX + scaledMaxVal, baseY );
        break;
    }

    s.axisLineSymbol()->renderPolyline( axisPoints, nullptr, c );
    s.axisLineSymbol()->stopRender( c );
  }
}
