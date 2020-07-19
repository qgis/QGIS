/***************************************************************************
    qgsstackedbardiagram.cpp
    ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstackedbardiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsrendercontext.h"
#include "qgsexpression.h"
#include "qgssymbollayerutils.h"

#include <QPainter>

QgsStackedBarDiagram::QgsStackedBarDiagram()
{
  mCategoryBrush.setStyle( Qt::SolidPattern );
  mPen.setStyle( Qt::SolidLine );
}

QgsStackedBarDiagram *QgsStackedBarDiagram::clone() const
{
  return new QgsStackedBarDiagram( *this );
}

QSizeF QgsStackedBarDiagram::diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is )
{
  if ( qgsDoubleNear( is.upperValue, is.lowerValue ) )
    return QSizeF(); // invalid value range => zero size

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
  double value = attrVal.toDouble( &ok );
  if ( !ok )
  {
    return QSizeF(); //zero size if attribute is missing
  }

  QSizeF size = sizeForValue( value, s, is );

  // eh - this method returns size in unknown units ...! We'll have to fake it and use a rough estimation of
  // a conversion factor to painter units...
  // TODO QGIS 4.0 -- these methods should all use painter units, dependent on the render context scaling...
  double painterUnitConversionScale = c.convertToPainterUnits( 1, s.sizeType );

  const double spacing = c.convertToPainterUnits( s.spacing(), s.spacingUnit(), s.spacingMapUnitScale() ) / painterUnitConversionScale;
  mApplySpacingAdjust = true;

  switch ( s.diagramOrientation )
  {
    case QgsDiagramSettings::Up:
    case QgsDiagramSettings::Down:
    {
      const double totalBarLength = size.height() + spacing * std::max( 0, s.categoryAttributes.size() - 1 );
      size = QSizeF( s.barWidth, totalBarLength );
      break;
    }

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
    {
      const double totalBarLength = size.width() + spacing * std::max( 0, s.categoryAttributes.size() - 1 );
      size = QSizeF( totalBarLength, s.barWidth );
      break;
    }
  }

  if ( s.showAxis() && s.axisLineSymbol() )
  {
    const double maxBleed = QgsSymbolLayerUtils::estimateMaxSymbolBleed( s.axisLineSymbol(), c ) / painterUnitConversionScale;
    size.setWidth( size.width() + 2 * maxBleed );
    size.setHeight( size.height() + 2 * maxBleed );
  }

  return size;
}

double QgsStackedBarDiagram::legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const
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

QString QgsStackedBarDiagram::diagramName() const
{
  return DIAGRAM_NAME_STACKED;
}

QSizeF QgsStackedBarDiagram::diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s )
{
  Q_UNUSED( c )
  QSizeF size;

  if ( attributes.isEmpty() )
  {
    return QSizeF(); //zero size if no attributes
  }

  double totalSum = 0;
  for ( int i = 0; i < attributes.count(); ++i )
  {
    totalSum += attributes.at( i ).toDouble();
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
      size.scale( s.barWidth, s.size.height() + spacing * std::max( 0, s.categoryAttributes.size() - 1 ), Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
      size.scale( s.size.width() + spacing * std::max( 0, s.categoryAttributes.size() - 1 ), s.barWidth, Qt::IgnoreAspectRatio );
      break;
  }

  return size;
}

void QgsStackedBarDiagram::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position )
{
  QPainter *p = c.painter();
  if ( !p )
  {
    return;
  }

  QList<double> values;

  QgsExpressionContext expressionContext = c.expressionContext();
  expressionContext.setFeature( feature );
  if ( !feature.fields().isEmpty() )
    expressionContext.setFields( feature.fields() );

  values.reserve( s.categoryAttributes.size() );
  double total = 0;
  for ( const QString &cat : qgis::as_const( s.categoryAttributes ) )
  {
    QgsExpression *expression = getExpression( cat, expressionContext );
    double currentVal = expression->evaluate( &expressionContext ).toDouble();
    values.push_back( currentVal );
    total += currentVal;
  }

  const double spacing = c.convertToPainterUnits( s.spacing(), s.spacingUnit(), s.spacingMapUnitScale() );
  const double totalSpacing = std::max( 0, s.categoryAttributes.size() - 1 ) * spacing;

  double scaledMaxVal = 0;
  switch ( s.diagramOrientation )
  {
    case QgsDiagramSettings::Up:
    case QgsDiagramSettings::Down:
      scaledMaxVal = sizePainterUnits( s.size.height(), s, c );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
      scaledMaxVal = sizePainterUnits( s.size.width(), s, c );
      break;
  }
  if ( mApplySpacingAdjust )
    scaledMaxVal -= totalSpacing;

  double currentOffset = 0;
  double scaledWidth = sizePainterUnits( s.barWidth, s, c );


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
    double length = *valIt / total * scaledMaxVal;

    mCategoryBrush.setColor( *colIt );
    p->setBrush( mCategoryBrush );

    switch ( s.diagramOrientation )
    {
      case QgsDiagramSettings::Up:
        p->drawRect( QRectF( baseX, baseY - currentOffset, scaledWidth, length * -1 ) );
        break;

      case QgsDiagramSettings::Down:
        p->drawRect( QRectF( baseX, baseY + currentOffset - scaledMaxVal - spacing * std::max( 0, values.size() - 1 ), scaledWidth, length ) );
        break;

      case QgsDiagramSettings::Right:
        p->drawRect( QRectF( baseX + currentOffset, baseY - scaledWidth, length, scaledWidth ) );
        break;

      case QgsDiagramSettings::Left:
        p->drawRect( QRectF( baseX + scaledMaxVal - currentOffset + spacing * std::max( 0, values.size() - 1 ), baseY - scaledWidth, 0 - length, scaledWidth ) );
        break;
    }

    currentOffset += length + spacing;
  }

  if ( s.showAxis() && s.axisLineSymbol() )
  {
    s.axisLineSymbol()->startRender( c );
    QPolygonF axisPoints;
    switch ( s.diagramOrientation )
    {
      case QgsDiagramSettings::Up:
        axisPoints << QPointF( baseX, baseY - scaledMaxVal - spacing * std::max( 0, values.size() - 1 ) ) << QPointF( baseX, baseY ) << QPointF( baseX + scaledWidth, baseY );
        break;

      case QgsDiagramSettings::Down:
        axisPoints << QPointF( baseX, baseY ) << QPointF( baseX, baseY - scaledMaxVal - spacing * std::max( 0, values.size() - 1 ) ) << QPointF( baseX + scaledWidth, baseY - scaledMaxVal - spacing * std::max( 0, values.size() - 1 ) );
        break;

      case QgsDiagramSettings::Right:
        axisPoints << QPointF( baseX + scaledMaxVal + spacing * std::max( 0, values.size() - 1 ), baseY - scaledWidth )
                   << QPointF( baseX, baseY - scaledWidth )
                   << QPointF( baseX, baseY );
        break;

      case QgsDiagramSettings::Left:
        axisPoints << QPointF( baseX, baseY - scaledWidth )
                   << QPointF( baseX + scaledMaxVal + spacing * std::max( 0, values.size() - 1 ), baseY - scaledWidth )
                   << QPointF( baseX + scaledMaxVal + spacing * std::max( 0, values.size() - 1 ), baseY );
        break;
    }

    s.axisLineSymbol()->renderPolyline( axisPoints, nullptr, c );
    s.axisLineSymbol()->stopRender( c );
  }
}
