/***************************************************************************
                         qgscomposerutils.cpp
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerutils.h"
#include "qgscomposition.h"
#include "qgsdatadefined.h"
#include <QPainter>

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter

#ifndef M_DEG2RAD
#define M_DEG2RAD 0.0174532925
#endif

void QgsComposerUtils::drawArrowHead( QPainter *p, const double x, const double y, const double angle, const double arrowHeadWidth )
{
  if ( !p )
  {
    return;
  }

  double angleRad = angle / 180.0 * M_PI;
  QPointF middlePoint( x, y );
  //rotate both arrow points
  QPointF p1 = QPointF( -arrowHeadWidth / 2.0, arrowHeadWidth );
  QPointF p2 = QPointF( arrowHeadWidth / 2.0, arrowHeadWidth );

  QPointF p1Rotated, p2Rotated;
  p1Rotated.setX( p1.x() * cos( angleRad ) + p1.y() * -sin( angleRad ) );
  p1Rotated.setY( p1.x() * sin( angleRad ) + p1.y() * cos( angleRad ) );
  p2Rotated.setX( p2.x() * cos( angleRad ) + p2.y() * -sin( angleRad ) );
  p2Rotated.setY( p2.x() * sin( angleRad ) + p2.y() * cos( angleRad ) );

  QPolygonF arrowHeadPoly;
  arrowHeadPoly << middlePoint;
  arrowHeadPoly << QPointF( middlePoint.x() + p1Rotated.x(), middlePoint.y() + p1Rotated.y() );
  arrowHeadPoly << QPointF( middlePoint.x() + p2Rotated.x(), middlePoint.y() + p2Rotated.y() );

  p->save();

  QPen arrowPen = p->pen();
  arrowPen.setJoinStyle( Qt::RoundJoin );
  QBrush arrowBrush = p->brush();
  arrowBrush.setStyle( Qt::SolidPattern );
  p->setPen( arrowPen );
  p->setBrush( arrowBrush );
  arrowBrush.setStyle( Qt::SolidPattern );
  p->drawPolygon( arrowHeadPoly );

  p->restore();
}

double QgsComposerUtils::angle( const QPointF &p1, const QPointF &p2 )
{
  double xDiff = p2.x() - p1.x();
  double yDiff = p2.y() - p1.y();
  double length = sqrt( xDiff * xDiff + yDiff * yDiff );
  if ( length <= 0 )
  {
    return 0;
  }

  double angle = acos(( -yDiff * length ) / ( length * length ) ) * 180 / M_PI;
  if ( xDiff < 0 )
  {
    return ( 360 - angle );
  }
  return angle;
}

void QgsComposerUtils::rotate( const double angle, double &x, double &y )
{
  double rotToRad = angle * M_PI / 180.0;
  double xRot, yRot;
  xRot = x * cos( rotToRad ) - y * sin( rotToRad );
  yRot = x * sin( rotToRad ) + y * cos( rotToRad );
  x = xRot;
  y = yRot;
}

double QgsComposerUtils::normalizedAngle( const double angle )
{
  double clippedAngle = angle;
  if ( clippedAngle >= 360.0 || clippedAngle <= -360.0 )
  {
    clippedAngle = fmod( clippedAngle, 360.0 );
  }
  if ( clippedAngle < 0.0 )
  {
    clippedAngle += 360.0;
  }
  return clippedAngle;
}

double QgsComposerUtils::snappedAngle( const double angle )
{
  //normalize angle to 0-360 degrees
  double clippedAngle = normalizedAngle( angle );

  //snap angle to 45 degree
  if ( clippedAngle >= 22.5 && clippedAngle < 67.5 )
  {
    return 45.0;
  }
  else if ( clippedAngle >= 67.5 && clippedAngle < 112.5 )
  {
    return 90.0;
  }
  else if ( clippedAngle >= 112.5 && clippedAngle < 157.5 )
  {
    return 135.0;
  }
  else if ( clippedAngle >= 157.5 && clippedAngle < 202.5 )
  {
    return 180.0;
  }
  else if ( clippedAngle >= 202.5 && clippedAngle < 247.5 )
  {
    return 225.0;
  }
  else if ( clippedAngle >= 247.5 && clippedAngle < 292.5 )
  {
    return 270.0;
  }
  else if ( clippedAngle >= 292.5 && clippedAngle < 337.5 )
  {
    return 315.0;
  }
  else
  {
    return 0.0;
  }
}

QRectF QgsComposerUtils::largestRotatedRectWithinBounds( const QRectF originalRect, const QRectF boundsRect, const double rotation )
{
  double originalWidth = originalRect.width();
  double originalHeight = originalRect.height();
  double boundsWidth = boundsRect.width();
  double boundsHeight = boundsRect.height();
  double ratioBoundsRect = boundsWidth / boundsHeight;

  double clippedRotation = normalizedAngle( rotation );

  //shortcut for some rotation values
  if ( clippedRotation == 0 || clippedRotation == 90 || clippedRotation == 180 || clippedRotation == 270 )
  {
    double rectScale;
    if ( clippedRotation == 0 || clippedRotation == 180 )
    {
      rectScale = (( originalWidth / originalHeight ) > ratioBoundsRect ) ? boundsWidth / originalWidth : boundsHeight / originalHeight;
    }
    else
    {
      rectScale = (( originalHeight / originalWidth ) > ratioBoundsRect ) ? boundsWidth / originalHeight : boundsHeight / originalWidth;
    }
    double rectScaledWidth = rectScale * originalWidth;
    double rectScaledHeight = rectScale * originalHeight;

    if ( clippedRotation == 0 || clippedRotation == 180 )
    {
      return QRectF(( boundsWidth - rectScaledWidth ) / 2.0, ( boundsHeight - rectScaledHeight ) / 2.0, rectScaledWidth, rectScaledHeight );
    }
    else
    {
      return QRectF(( boundsWidth - rectScaledHeight ) / 2.0, ( boundsHeight - rectScaledWidth ) / 2.0, rectScaledWidth, rectScaledHeight );
    }
  }

  //convert angle to radians and flip
  double angleRad = -clippedRotation * M_DEG2RAD;
  double cosAngle = cos( angleRad );
  double sinAngle = sin( angleRad );

  //calculate size of bounds of rotated rectangle
  double widthBoundsRotatedRect = originalWidth * fabs( cosAngle ) + originalHeight * fabs( sinAngle );
  double heightBoundsRotatedRect = originalHeight * fabs( cosAngle ) + originalWidth * fabs( sinAngle );

  //compare ratio of rotated rect with bounds rect and calculate scaling of rotated
  //rect to fit within bounds
  double ratioBoundsRotatedRect = widthBoundsRotatedRect / heightBoundsRotatedRect;
  double rectScale = ratioBoundsRotatedRect > ratioBoundsRect ? boundsWidth / widthBoundsRotatedRect : boundsHeight / heightBoundsRotatedRect;
  double rectScaledWidth = rectScale * originalWidth;
  double rectScaledHeight = rectScale * originalHeight;

  //now calculate offset so that rotated rectangle is centered within bounds
  //first calculate min x and y coordinates
  double currentCornerX = 0;
  double minX = 0;
  currentCornerX += rectScaledWidth * cosAngle;
  minX = minX < currentCornerX ? minX : currentCornerX;
  currentCornerX += rectScaledHeight * sinAngle;
  minX = minX < currentCornerX ? minX : currentCornerX;
  currentCornerX -= rectScaledWidth * cosAngle;
  minX = minX < currentCornerX ? minX : currentCornerX;

  double currentCornerY = 0;
  double minY = 0;
  currentCornerY -= rectScaledWidth * sinAngle;
  minY = minY < currentCornerY ? minY : currentCornerY;
  currentCornerY += rectScaledHeight * cosAngle;
  minY = minY < currentCornerY ? minY : currentCornerY;
  currentCornerY += rectScaledWidth * sinAngle;
  minY = minY < currentCornerY ? minY : currentCornerY;

  //now calculate offset position of rotated rectangle
  double offsetX = ratioBoundsRotatedRect > ratioBoundsRect ? 0 : ( boundsWidth - rectScale * widthBoundsRotatedRect ) / 2.0;
  offsetX += fabs( minX );
  double offsetY = ratioBoundsRotatedRect > ratioBoundsRect ? ( boundsHeight - rectScale * heightBoundsRotatedRect ) / 2.0 : 0;
  offsetY += fabs( minY );

  return QRectF( offsetX, offsetY, rectScaledWidth, rectScaledHeight );
}

double QgsComposerUtils::pointsToMM( const double pointSize )
{
  //conversion to mm based on 1 point = 1/72 inch
  return ( pointSize * 0.3527 );
}

double QgsComposerUtils::mmToPoints( const double mmSize )
{
  //conversion to points based on 1 point = 1/72 inch
  return ( mmSize / 0.3527 );
}

void QgsComposerUtils::relativeResizeRect( QRectF& rectToResize, const QRectF& boundsBefore, const QRectF& boundsAfter )
{
  //linearly scale rectToResize relative to the scaling from boundsBefore to boundsAfter
  double left = relativePosition( rectToResize.left(), boundsBefore.left(), boundsBefore.right(), boundsAfter.left(), boundsAfter.right() );
  double right = relativePosition( rectToResize.right(), boundsBefore.left(), boundsBefore.right(), boundsAfter.left(), boundsAfter.right() );
  double top = relativePosition( rectToResize.top(), boundsBefore.top(), boundsBefore.bottom(), boundsAfter.top(), boundsAfter.bottom() );
  double bottom = relativePosition( rectToResize.bottom(), boundsBefore.top(), boundsBefore.bottom(), boundsAfter.top(), boundsAfter.bottom() );

  rectToResize.setRect( left, top, right - left, bottom - top );
}

double QgsComposerUtils::relativePosition( const double position, const double beforeMin, const double beforeMax, const double afterMin, const double afterMax )
{
  //calculate parameters for linear scale between before and after ranges
  double m = ( afterMax - afterMin ) / ( beforeMax - beforeMin );
  double c = afterMin - ( beforeMin * m );

  //return linearly scaled position
  return m * position + c;
}

QgsComposition::PaperOrientation QgsComposerUtils::decodePaperOrientation( const QString orientationString, bool &ok )
{
  if ( orientationString.compare( "Portrait", Qt::CaseInsensitive ) == 0 )
  {
    ok = true;
    return QgsComposition::Portrait;
  }
  if ( orientationString.compare( "Landscape", Qt::CaseInsensitive ) == 0 )
  {
    ok = true;
    return QgsComposition::Landscape;
  }
  ok = false;
  return QgsComposition::Landscape; // default to landscape
}

bool QgsComposerUtils::decodePresetPaperSize( const QString presetString, double &width, double &height )
{
  QList< QPair< QString, QSizeF > > presets;
  presets << qMakePair( QString( "A5" ), QSizeF( 148, 210 ) );
  presets << qMakePair( QString( "A4" ), QSizeF( 210, 297 ) );
  presets << qMakePair( QString( "A3" ), QSizeF( 297, 420 ) );
  presets << qMakePair( QString( "A2" ), QSizeF( 420, 594 ) );
  presets << qMakePair( QString( "A1" ), QSizeF( 594, 841 ) );
  presets << qMakePair( QString( "A0" ), QSizeF( 841, 1189 ) );
  presets << qMakePair( QString( "B5" ), QSizeF( 176, 250 ) );
  presets << qMakePair( QString( "B4" ), QSizeF( 250, 353 ) );
  presets << qMakePair( QString( "B3" ), QSizeF( 353, 500 ) );
  presets << qMakePair( QString( "B2" ), QSizeF( 500, 707 ) );
  presets << qMakePair( QString( "B1" ), QSizeF( 707, 1000 ) );
  presets << qMakePair( QString( "B0" ), QSizeF( 1000, 1414 ) );
  // North american formats
  presets << qMakePair( QString( "Legal" ), QSizeF( 215.9, 355.6 ) );
  presets << qMakePair( QString( "Letter" ), QSizeF( 215.9, 279.4 ) );
  presets << qMakePair( QString( "ANSI A" ), QSizeF( 215.9, 279.4 ) );
  presets << qMakePair( QString( "ANSI B" ), QSizeF( 279.4, 431.8 ) );
  presets << qMakePair( QString( "ANSI C" ), QSizeF( 431.8, 558.8 ) );
  presets << qMakePair( QString( "ANSI D" ), QSizeF( 558.8, 863.6 ) );
  presets << qMakePair( QString( "ANSI E" ), QSizeF( 863.6, 1117.6 ) );
  presets << qMakePair( QString( "Arch A" ), QSizeF( 228.6, 304.8 ) );
  presets << qMakePair( QString( "Arch B" ), QSizeF( 304.8, 457.2 ) );
  presets << qMakePair( QString( "Arch C" ), QSizeF( 457.2, 609.6 ) );
  presets << qMakePair( QString( "Arch D" ), QSizeF( 609.6, 914.4 ) );
  presets << qMakePair( QString( "Arch E" ), QSizeF( 914.4, 1219.2 ) );
  presets << qMakePair( QString( "Arch E1" ), QSizeF( 762, 1066.8 ) );

  QList< QPair< QString, QSizeF > >::const_iterator presetIt = presets.constBegin();
  for ( ;presetIt != presets.constEnd(); ++presetIt )
  {
    if ( presetString.compare(( *presetIt ).first, Qt::CaseInsensitive ) == 0 )
    {
      width = ( *presetIt ).second.width();
      height = ( *presetIt ).second.height();
      return true;
    }
  }
  return false;
}

void QgsComposerUtils::readDataDefinedPropertyMap( const QDomElement &itemElem, QMap<QgsComposerObject::DataDefinedProperty, QString> *dataDefinedNames, QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties )
{
  QMap<QgsComposerObject::DataDefinedProperty, QString>::const_iterator i = dataDefinedNames->constBegin();
  for ( ; i != dataDefinedNames->constEnd(); ++i )
  {
    QString elemName = i.value();
    QDomNodeList ddNodeList = itemElem.elementsByTagName( elemName );
    if ( ddNodeList.size() > 0 )
    {
      QDomElement ddElem = ddNodeList.at( 0 ).toElement();
      readDataDefinedProperty( i.key(), ddElem, dataDefinedProperties );
    }
  }
}

void QgsComposerUtils::readDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QDomElement &ddElem, QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties )
{
  if ( property == QgsComposerObject::AllProperties || property == QgsComposerObject::NoProperty )
  {
    //invalid property
    return;
  }

  QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >::const_iterator it = dataDefinedProperties->constFind( property );

  QgsDataDefined* dd = 0;
  if ( it != dataDefinedProperties->constEnd() )
  {
    dd = it.value();
  }
  else
  {
    //QgsDataDefined for property doesn't currently exist, need to add new
    dd = new QgsDataDefined();
    dataDefinedProperties->insert( property, dd );
  }

  //set values for QgsDataDefined
  QString active = ddElem.attribute( "active" );
  if ( active.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    dd->setActive( true );
  }
  else
  {
    dd->setActive( false );
  }
  QString useExpr = ddElem.attribute( "useExpr" );
  if ( useExpr.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    dd->setUseExpression( true );
  }
  else
  {
    dd->setUseExpression( false );
  }
  dd->setField( ddElem.attribute( "field" ) );
  dd->setExpressionString( ddElem.attribute( "expr" ) );
}

void QgsComposerUtils::writeDataDefinedPropertyMap( QDomElement &itemElem, QDomDocument &doc, const QMap<QgsComposerObject::DataDefinedProperty, QString> *dataDefinedNames, const QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties )
{
  QMap<QgsComposerObject::DataDefinedProperty, QString >::const_iterator i = dataDefinedNames->constBegin();
  for ( ; i != dataDefinedNames->constEnd(); ++i )
  {
    QString newElemName = i.value();

    QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >::const_iterator it = dataDefinedProperties->find( i.key() );
    if ( it != dataDefinedProperties->constEnd() )
    {
      QgsDataDefined* dd = it.value();
      if ( dd )
      {
        bool active = dd->isActive();
        bool useExpr = dd->useExpression();
        QString expr = dd->expressionString();
        QString field = dd->field();

        bool defaultVals = ( !active && !useExpr && expr.isEmpty() && field.isEmpty() );

        if ( !defaultVals )
        {
          QDomElement ddElem = doc.createElement( newElemName );
          if ( active )
          {
            ddElem.setAttribute( "active", "true" );
          }
          else
          {
            ddElem.setAttribute( "active", "false" );
          }
          if ( useExpr )
          {
            ddElem.setAttribute( "useExpr", "true" );
          }
          else
          {
            ddElem.setAttribute( "useExpr", "false" );
          }
          ddElem.setAttribute( "expr", expr );
          ddElem.setAttribute( "field", field );
          itemElem.appendChild( ddElem );
        }
      }
    }
  }
}

QFont QgsComposerUtils::scaledFontPixelSize( const QFont &font )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont scaledFont = font;
  double pixelSize = pointsToMM( scaledFont.pointSizeF() ) * FONT_WORKAROUND_SCALE + 0.5;
  scaledFont.setPixelSize( pixelSize );
  return scaledFont;
}

double QgsComposerUtils::fontAscentMM( const QFont &font )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.ascent() / FONT_WORKAROUND_SCALE );
}

double QgsComposerUtils::fontDescentMM( const QFont &font )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.descent() / FONT_WORKAROUND_SCALE );
}

double QgsComposerUtils::fontHeightMM( const QFont &font )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.height() / FONT_WORKAROUND_SCALE );
}

double QgsComposerUtils::fontHeightCharacterMM( const QFont &font, const QChar &character )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.boundingRect( character ).height() / FONT_WORKAROUND_SCALE );
}

double QgsComposerUtils::textWidthMM( const QFont &font, const QString &text )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.width( text ) / FONT_WORKAROUND_SCALE );
}

void QgsComposerUtils::drawText( QPainter *painter, const QPointF &pos, const QString &text, const QFont &font, const QColor &color )
{
  if ( !painter )
  {
    return;
  }

  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont textFont = scaledFontPixelSize( font );

  painter->save();
  painter->setFont( textFont );
  if ( color.isValid() )
  {
    painter->setPen( color );
  }
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  painter->scale( scaleFactor, scaleFactor );
  painter->drawText( pos * FONT_WORKAROUND_SCALE, text );
  painter->restore();
}

void QgsComposerUtils::drawText( QPainter *painter, const QRectF &rect, const QString &text, const QFont &font, const QColor &color, const Qt::AlignmentFlag halignment, const Qt::AlignmentFlag valignment, const int flags )
{
  if ( !painter )
  {
    return;
  }

  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  painter->save();
  painter->setFont( textFont );
  if ( color.isValid() )
  {
    painter->setPen( color );
  }
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  painter->scale( scaleFactor, scaleFactor );
  painter->drawText( scaledRect, halignment | valignment | flags, text );
  painter->restore();
}
