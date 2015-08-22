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
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    pen.setWidthF( s.penWidth * c.scaleFactor() );
  }
  else
  {
    pen.setWidthF( s.penWidth / c.mapToPixel().mapUnitsPerPixel() );
  }
}


QSizeF QgsDiagram::sizePainterUnits( const QSizeF& size, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    return QSizeF( size.width() * c.scaleFactor(), size.height() * c.scaleFactor() );
  }
  else
  {
    return QSizeF( size.width() / c.mapToPixel().mapUnitsPerPixel(), size.height() / c.mapToPixel().mapUnitsPerPixel() );
  }
}

float QgsDiagram::sizePainterUnits( float l, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    return l * c.scaleFactor();
  }
  else
  {
    return l / c.mapToPixel().mapUnitsPerPixel();
  }
}

QFont QgsDiagram::scaledFont( const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  QFont f = s.font;
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    f.setPixelSize( s.font.pointSizeF() * 0.376 * c.scaleFactor() );
  }
  else
  {
    f.setPixelSize( s.font.pointSizeF() / c.mapToPixel().mapUnitsPerPixel() );
  }

  return f;
}

void QgsDiagram::renderDiagram( const QgsAttributes& attributes, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )
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
