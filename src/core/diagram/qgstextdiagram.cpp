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
#include "qgsdiagramrendererv2.h"
#include "qgsrendercontext.h"
#include "qgsexpression.h"

#include <QPainter>

QgsTextDiagram::QgsTextDiagram(): mOrientation( Vertical ), mShape( Circle )
{
  mPen.setWidthF( 2.0 );
  mPen.setColor( QColor( 0, 0, 0 ) );
  mPen.setCapStyle( Qt::FlatCap );
  mBrush.setStyle( Qt::SolidPattern );
}

QgsTextDiagram::~QgsTextDiagram()
{
}

QgsDiagram* QgsTextDiagram::clone() const
{
  return new QgsTextDiagram( *this );
}

QSizeF QgsTextDiagram::diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is )
{
  Q_UNUSED( c );

  QVariant attrVal;
  if ( is.classificationAttributeIsExpression )
  {
    QgsExpression* expression = getExpression( is.classificationAttributeExpression, feature.fields() );
    attrVal = expression->evaluate( feature );
  }
  else
  {
    attrVal = feature.attributes()[is.classificationAttribute];
  }

  if ( !attrVal.isValid() )
  {
    return QSizeF(); //zero size if attribute is missing
  }

  double scaledValue = attrVal.toDouble();
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

QSizeF QgsTextDiagram::diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s )
{
  Q_UNUSED( c );
  Q_UNUSED( attributes );

  return s.size;
}

void QgsTextDiagram::renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )
{
  Q_UNUSED( feature );

  QPainter* p = c.painter();
  if ( !p )
  {
    return;
  }

  //convert from mm / map units to painter units
  QSizeF spu = sizePainterUnits( s.size, s, c );
  double w = spu.width();
  double h = spu.height();

  double baseX = position.x();
  double baseY = position.y() - h;

  QList<QPointF> textPositions; //midpoints for text placement
  int nCategories = s.categoryAttributes.size();
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
    QPointF center( baseX + w / 2.0, baseY + h / 2.0 );
    double r1 = w / 2.0; double r2 = h / 2.0;

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

    QLineF triangleEdgeLeft( baseX + w / 2.0, baseY, baseX, baseY + h );
    QLineF triangleEdgeRight( baseX + w, baseY + h, baseX + w / 2.0, baseY );
    QPointF intersectionPoint1, intersectionPoint2;

    for ( int i = 1; i < nCategories; ++i )
    {
      if ( mOrientation == Horizontal )
      {
        QLineF verticalLine( baseX + w / nCategories * i, baseY + h, baseX + w / nCategories * i, baseY );
        if ( baseX + w / nCategories * i < baseX + w / 2.0 )
        {
          verticalLine.intersect( triangleEdgeLeft, &intersectionPoint1 );
        }
        else
        {
          verticalLine.intersect( triangleEdgeRight, &intersectionPoint1 );
        }
        p->drawLine( QPointF( baseX + w / nCategories * i, baseY + h ), intersectionPoint1 );
      }
      else //vertical
      {
        QLineF horizontalLine( baseX, baseY + h / nCategories * i, baseX + w, baseY + h / nCategories * i );
        horizontalLine.intersect( triangleEdgeLeft, &intersectionPoint1 );
        horizontalLine.intersect( triangleEdgeRight, &intersectionPoint2 );
        p->drawLine( intersectionPoint1, intersectionPoint2 );
      }
    }
  }

  //draw text
  QFont sFont = scaledFont( s, c );
  QFontMetricsF fontMetrics( sFont );
  p->setFont( sFont );

  for ( int i = 0; i < textPositions.size(); ++i )
  {
    QgsExpression* expression = getExpression( s.categoryAttributes.at( i ), feature.fields() );
    QString val = expression->evaluate( feature ).toString();

    //find out dimesions
    double textWidth = fontMetrics.width( val );
    double textHeight = fontMetrics.height();

    mPen.setColor( s.categoryColors.at( i ) );
    p->setPen( mPen );
    QPointF position = textPositions.at( i );

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

void QgsTextDiagram::lineEllipseIntersection( const QPointF& lineStart, const QPointF& lineEnd, const QPointF& ellipseMid, double r1, double r2, QList<QPointF>& result ) const
{
  result.clear();

  double rrx = r1 * r1;
  double rry = r2 * r2;
  double x21 = lineEnd.x() - lineStart.x();
  double y21 = lineEnd.y() - lineStart.y();
  double x10 = lineStart.x() - ellipseMid.x();
  double y10 = lineStart.y() - ellipseMid.y();
  double a = x21 * x21 / rrx + y21 * y21 / rry;
  double b = x21 * x10 / rrx + y21 * y10 / rry;
  double c = x10 * x10 / rrx + y10 * y10 / rry;
  double d = b * b - a * ( c - 1 );
  if ( d > 0 )
  {
    double e = sqrt( d );
    double u1 = ( -b - e ) / a;
    double u2 = ( -b + e ) / a;
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
