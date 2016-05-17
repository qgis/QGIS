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
#include "qgsdiagram.h"
#include "qgsdiagramrendererv2.h"
#include "qgsrendercontext.h"
#include "qgsexpression.h"

#include <QPainter>



QgsDiagram::QgsDiagram()
{

}

QgsDiagram::QgsDiagram( const QgsDiagram& other )
{
  Q_UNUSED( other );
  // do not copy the cached expression map - the expressions need to be created and prepared with getExpression(...) call
}


void QgsDiagram::clearCache()
{
  QMapIterator<QString, QgsExpression*> i( mExpressions );
  while ( i.hasNext() )
  {
    i.next();
    delete i.value();
  }
  mExpressions.clear();
}

QgsExpression* QgsDiagram::getExpression( const QString& expression, const QgsFields* fields )
{
  Q_NOWARN_DEPRECATED_PUSH
  if ( !mExpressions.contains( expression ) )
  {
    QgsExpression* expr = new QgsExpression( expression );
    expr->prepare( *fields );
    mExpressions[expression] = expr;
  }
  return mExpressions[expression];
  Q_NOWARN_DEPRECATED_POP
}

QgsExpression *QgsDiagram::getExpression( const QString &expression, const QgsExpressionContext &context )
{
  if ( !mExpressions.contains( expression ) )
  {
    QgsExpression* expr = new QgsExpression( expression );
    expr->prepare( &context );
    mExpressions[expression] = expr;
  }
  return mExpressions[expression];
}

void QgsDiagram::setPenWidth( QPen& pen, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  pen.setWidthF( QgsSymbolLayerV2Utils::convertToPainterUnits( c, s.penWidth, s.lineSizeUnit, s.lineSizeScale ) );
}


QSizeF QgsDiagram::sizePainterUnits( QSizeF size, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  return QSizeF( QgsSymbolLayerV2Utils::convertToPainterUnits( c, size.width(), s.sizeType, s.sizeScale ), QgsSymbolLayerV2Utils::convertToPainterUnits( c, size.height(), s.sizeType, s.sizeScale ) );
}

float QgsDiagram::sizePainterUnits( float l, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  return QgsSymbolLayerV2Utils::convertToPainterUnits( c, l, s.sizeType, s.sizeScale );
}

QFont QgsDiagram::scaledFont( const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  QFont f = s.font;
  if ( s.sizeType == QgsSymbolV2::MapUnit )
  {
    int pixelsize = s.font.pointSizeF() / c.mapToPixel().mapUnitsPerPixel();
    f.setPixelSize( pixelsize > 0 ? pixelsize : 1 );
  }
  else
  {
    f.setPixelSize( s.font.pointSizeF() * 0.376 * c.scaleFactor() );
  }

  return f;
}

QSizeF QgsDiagram::sizeForValue( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const
{
  double scaledValue = value;
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
    bool p = false; // preserve height == width
    if ( qgsDoubleNear( size.width(), size.height() ) )
      p = true;

    size.scale( s.minimumSize, s.minimumSize, Qt::KeepAspectRatio );

    // If height == width, recover here (overwrite floating point errors)
    if ( p )
      size.setWidth( size.height() );
  }

  return size;
}

void QgsDiagram::renderDiagram( const QgsAttributes& attributes, QgsRenderContext& c, const QgsDiagramSettings& s, QPointF position )
{
  QgsFeature feature;
  feature.setAttributes( attributes );
  renderDiagram( feature, c, s, position );
}

QSizeF QgsDiagram::diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is )
{
  QgsFeature feature;
  feature.setAttributes( attributes );
  return diagramSize( feature, c, s, is );
}
