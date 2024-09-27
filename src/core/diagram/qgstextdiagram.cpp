/***************************************************************************
    qgstextdiagram.cpp
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
#include "qgstextdiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsrendercontext.h"
#include "qgsexpression.h"

#include <QPainter>

const QString QgsTextDiagram::DIAGRAM_NAME_TEXT = QStringLiteral( "Text" );

QgsTextDiagram::QgsTextDiagram()
{
  mPen.setWidthF( 2.0 );
  mPen.setColor( QColor( 0, 0, 0 ) );
  mPen.setCapStyle( Qt::FlatCap );
  mBrush.setStyle( Qt::SolidPattern );
}

QgsTextDiagram *QgsTextDiagram::clone() const
{
  return new QgsTextDiagram( *this );
}

QSizeF QgsTextDiagram::diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is )
{
  QgsExpressionContext expressionContext = c.expressionContext();
  expressionContext.setFeature( feature );
  if ( !feature.fields().isEmpty() )
    expressionContext.setFields( feature.fields() );

  QVariant attrVal;
  if ( is.classificationAttributeIsExpression )
  {
    QgsExpression *expression = getExpression( is.classificationAttributeExpression, expressionContext );
    attrVal = expression->evaluate( &expressionContext );
  }
  else
  {
    attrVal = feature.attribute( is.classificationField );
  }

  bool ok = false;
  const double val = attrVal.toDouble( &ok );
  if ( !ok )
  {
    return QSizeF(); //zero size if attribute is missing
  }

  return sizeForValue( val, s, is );
}

double QgsTextDiagram::legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const
{
  const QSizeF size = sizeForValue( value, s, is );
  return std::max( size.width(), size.height() );
}

QString QgsTextDiagram::diagramName() const
{
  return QgsTextDiagram::DIAGRAM_NAME_TEXT;
}

QSizeF QgsTextDiagram::diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s )
{
  Q_UNUSED( c )
  Q_UNUSED( attributes )

  return s.size;
}

void QgsTextDiagram::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position )
{
  QPainter *p = c.painter();
  if ( !p )
  {
    return;
  }

  //convert from mm / map units to painter units
  const QSizeF spu = sizePainterUnits( s.size, s, c );
  const double w = spu.width();
  const double h = spu.height();

  const double baseX = position.x();
  const double baseY = position.y() - h;

  QVector<QPointF> textPositions; //midpoints for text placement
  const int nCategories = s.categoryAttributes.size();
  for ( int i = 0; i < nCategories; ++i )
  {
    if ( mOrientation == Horizontal )
    {
      textPositions.push_back( QPointF( baseX + ( w / nCategories ) * i + w / nCategories / 2.0, baseY + h / 2.0 ) );
    }
    else //vertical
    {
      textPositions.push_back( QPointF( baseX + w / 2.0, baseY + h / nCategories * i + w / nCategories / 2.0 ) );
    }
  }

  mPen.setColor( s.penColor );
  setPenWidth( mPen, s, c );
  p->setPen( mPen );
  mBrush.setColor( s.backgroundColor );
  p->setBrush( mBrush );

  //draw shapes and separator lines first
  if ( mShape == Circle )
  {
    p->drawEllipse( baseX, baseY, w, h );

    //draw separator lines
    QList<QPointF> intersect; //intersections between shape and separation lines
    const QPointF center( baseX + w / 2.0, baseY + h / 2.0 );
    const double r1 = w / 2.0;
    const double r2 = h / 2.0;

    for ( int i = 1; i < nCategories; ++i )
    {
      if ( mOrientation == Horizontal )
      {
        lineEllipseIntersection( QPointF( baseX + w / nCategories * i, baseY ), QPointF( baseX + w / nCategories * i, baseY + h ), center, r1, r2, intersect );
      }
      else //vertical
      {
        lineEllipseIntersection( QPointF( baseX, baseY + h / nCategories * i ), QPointF( baseX + w, baseY + h / nCategories * i ), center, r1, r2, intersect );
      }
      if ( intersect.size() > 1 )
      {
        p->drawLine( intersect.at( 0 ), intersect.at( 1 ) );
      }
    }
  }
  else if ( mShape == Rectangle )
  {
    p->drawRect( QRectF( baseX, baseY, w, h ) );
    for ( int i = 1; i < nCategories; ++i )
    {
      if ( mOrientation == Horizontal )
      {
        p->drawLine( QPointF( baseX + w / nCategories * i, baseY ), QPointF( baseX + w / nCategories * i, baseY + h ) );
      }
      else
      {
        p->drawLine( QPointF( baseX, baseY + h / nCategories * i ), QPointF( baseX + w, baseY + h / nCategories * i ) );
      }
    }
  }
  else //triangle
  {
    QPolygonF triangle;
    triangle << QPointF( baseX, baseY + h ) << QPointF( baseX + w, baseY + h ) << QPointF( baseX + w / 2.0, baseY );
    p->drawPolygon( triangle );

    const QLineF triangleEdgeLeft( baseX + w / 2.0, baseY, baseX, baseY + h );
    const QLineF triangleEdgeRight( baseX + w, baseY + h, baseX + w / 2.0, baseY );
    QPointF intersectionPoint1, intersectionPoint2;

    for ( int i = 1; i < nCategories; ++i )
    {
      if ( mOrientation == Horizontal )
      {
        const QLineF verticalLine( baseX + w / nCategories * i, baseY + h, baseX + w / nCategories * i, baseY );
        if ( baseX + w / nCategories * i < baseX + w / 2.0 )
        {
          verticalLine.intersects( triangleEdgeLeft, &intersectionPoint1 );
        }
        else
        {
          verticalLine.intersects( triangleEdgeRight, &intersectionPoint1 );
        }
        p->drawLine( QPointF( baseX + w / nCategories * i, baseY + h ), intersectionPoint1 );
      }
      else //vertical
      {
        const QLineF horizontalLine( baseX, baseY + h / nCategories * i, baseX + w, baseY + h / nCategories * i );
        horizontalLine.intersects( triangleEdgeLeft, &intersectionPoint1 );
        horizontalLine.intersects( triangleEdgeRight, &intersectionPoint2 );
        p->drawLine( intersectionPoint1, intersectionPoint2 );
      }
    }
  }

  //draw text
  const QFont sFont = scaledFont( s, c );
  const QFontMetricsF fontMetrics( sFont );
  p->setFont( sFont );

  QgsExpressionContext expressionContext = c.expressionContext();
  expressionContext.setFeature( feature );
  if ( !feature.fields().isEmpty() )
    expressionContext.setFields( feature.fields() );

  for ( int i = 0; i < textPositions.size(); ++i )
  {
    QgsExpression *expression = getExpression( s.categoryAttributes.at( i ), expressionContext );
    const QString val = expression->evaluate( &expressionContext ).toString();

    //find out dimensions
    const double textWidth = fontMetrics.horizontalAdvance( val );
    const double textHeight = fontMetrics.height();

    mPen.setColor( s.categoryColors.at( i ) );
    p->setPen( mPen );
    const QPointF position = textPositions.at( i );

    // Calculate vertical placement
    double xOffset = 0;

    switch ( s.labelPlacementMethod )
    {
      case QgsDiagramSettings::Height:
        xOffset = textHeight / 2.0;
        break;

      case QgsDiagramSettings::XHeight:
        xOffset = fontMetrics.xHeight();
        break;
    }
    p->drawText( QPointF( position.x() - textWidth / 2.0, position.y() + xOffset ), val );
  }
}

void QgsTextDiagram::lineEllipseIntersection( QPointF lineStart, QPointF lineEnd, QPointF ellipseMid, double r1, double r2, QList<QPointF> &result ) const
{
  result.clear();

  const double rrx = r1 * r1;
  const double rry = r2 * r2;
  const double x21 = lineEnd.x() - lineStart.x();
  const double y21 = lineEnd.y() - lineStart.y();
  const double x10 = lineStart.x() - ellipseMid.x();
  const double y10 = lineStart.y() - ellipseMid.y();
  const double a = x21 * x21 / rrx + y21 * y21 / rry;
  const double b = x21 * x10 / rrx + y21 * y10 / rry;
  const double c = x10 * x10 / rrx + y10 * y10 / rry;
  const double d = b * b - a * ( c - 1 );
  if ( d > 0 )
  {
    const double e = std::sqrt( d );
    const double u1 = ( -b - e ) / a;
    const double u2 = ( -b + e ) / a;
    //work with a tolerance of 0.00001 because of limited numerical precision
    if ( -0.00001 <= u1 && u1 < 1.00001 )
    {
      result.push_back( QPointF( lineStart.x() + x21 * u1, lineStart.y() + y21 * u1 ) );
    }
    if ( -0.00001 <= u2 && u2 <= 1.00001 )
    {
      result.push_back( QPointF( lineStart.x() + x21 * u2, lineStart.y() + y21 * u2 ) );
    }
  }
}
