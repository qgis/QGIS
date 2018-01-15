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
#include "qgsproperty.h"
#include "qgsmapsettings.h"
#include "qgscomposermap.h"
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
  p1Rotated.setX( p1.x() * std::cos( angleRad ) + p1.y() * -std::sin( angleRad ) );
  p1Rotated.setY( p1.x() * std::sin( angleRad ) + p1.y() * std::cos( angleRad ) );
  p2Rotated.setX( p2.x() * std::cos( angleRad ) + p2.y() * -std::sin( angleRad ) );
  p2Rotated.setY( p2.x() * std::sin( angleRad ) + p2.y() * std::cos( angleRad ) );

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

double QgsComposerUtils::angle( QPointF p1, QPointF p2 )
{
  double xDiff = p2.x() - p1.x();
  double yDiff = p2.y() - p1.y();
  double length = std::sqrt( xDiff * xDiff + yDiff * yDiff );
  if ( length <= 0 )
  {
    return 0;
  }

  double angle = std::acos( ( -yDiff * length ) / ( length * length ) ) * 180 / M_PI;
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
  xRot = x * std::cos( rotToRad ) - y * std::sin( rotToRad );
  yRot = x * std::sin( rotToRad ) + y * std::cos( rotToRad );
  x = xRot;
  y = yRot;
}

double QgsComposerUtils::normalizedAngle( const double angle )
{
  double clippedAngle = angle;
  if ( clippedAngle >= 360.0 || clippedAngle <= -360.0 )
  {
    clippedAngle = std::fmod( clippedAngle, 360.0 );
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

QRectF QgsComposerUtils::largestRotatedRectWithinBounds( const QRectF &originalRect, const QRectF &boundsRect, const double rotation )
{
  double originalWidth = originalRect.width();
  double originalHeight = originalRect.height();
  double boundsWidth = boundsRect.width();
  double boundsHeight = boundsRect.height();
  double ratioBoundsRect = boundsWidth / boundsHeight;

  double clippedRotation = normalizedAngle( rotation );

  //shortcut for some rotation values
  if ( qgsDoubleNear( clippedRotation, 0.0 ) || qgsDoubleNear( clippedRotation, 90.0 ) || qgsDoubleNear( clippedRotation, 180.0 ) || qgsDoubleNear( clippedRotation, 270.0 ) )
  {
    double rectScale;
    if ( qgsDoubleNear( clippedRotation, 0.0 ) || qgsDoubleNear( clippedRotation, 180.0 ) )
    {
      rectScale = ( ( originalWidth / originalHeight ) > ratioBoundsRect ) ? boundsWidth / originalWidth : boundsHeight / originalHeight;
    }
    else
    {
      rectScale = ( ( originalHeight / originalWidth ) > ratioBoundsRect ) ? boundsWidth / originalHeight : boundsHeight / originalWidth;
    }
    double rectScaledWidth = rectScale * originalWidth;
    double rectScaledHeight = rectScale * originalHeight;

    if ( qgsDoubleNear( clippedRotation, 0.0 ) || qgsDoubleNear( clippedRotation, 180.0 ) )
    {
      return QRectF( ( boundsWidth - rectScaledWidth ) / 2.0, ( boundsHeight - rectScaledHeight ) / 2.0, rectScaledWidth, rectScaledHeight );
    }
    else
    {
      return QRectF( ( boundsWidth - rectScaledHeight ) / 2.0, ( boundsHeight - rectScaledWidth ) / 2.0, rectScaledWidth, rectScaledHeight );
    }
  }

  //convert angle to radians and flip
  double angleRad = -clippedRotation * M_DEG2RAD;
  double cosAngle = std::cos( angleRad );
  double sinAngle = std::sin( angleRad );

  //calculate size of bounds of rotated rectangle
  double widthBoundsRotatedRect = originalWidth * std::fabs( cosAngle ) + originalHeight * std::fabs( sinAngle );
  double heightBoundsRotatedRect = originalHeight * std::fabs( cosAngle ) + originalWidth * std::fabs( sinAngle );

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
  offsetX += std::fabs( minX );
  double offsetY = ratioBoundsRotatedRect > ratioBoundsRect ? ( boundsHeight - rectScale * heightBoundsRotatedRect ) / 2.0 : 0;
  offsetY += std::fabs( minY );

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

void QgsComposerUtils::relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter )
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

QgsComposition::PaperOrientation QgsComposerUtils::decodePaperOrientation( const QString &orientationString, bool &ok )
{
  if ( orientationString.compare( QLatin1String( "Portrait" ), Qt::CaseInsensitive ) == 0 )
  {
    ok = true;
    return QgsComposition::Portrait;
  }
  if ( orientationString.compare( QLatin1String( "Landscape" ), Qt::CaseInsensitive ) == 0 )
  {
    ok = true;
    return QgsComposition::Landscape;
  }
  ok = false;
  return QgsComposition::Landscape; // default to landscape
}

bool QgsComposerUtils::decodePresetPaperSize( const QString &presetString, double &width, double &height )
{
  QList< QPair< QString, QSizeF > > presets;
  presets << qMakePair( QStringLiteral( "A5" ), QSizeF( 148, 210 ) );
  presets << qMakePair( QStringLiteral( "A4" ), QSizeF( 210, 297 ) );
  presets << qMakePair( QStringLiteral( "A3" ), QSizeF( 297, 420 ) );
  presets << qMakePair( QStringLiteral( "A2" ), QSizeF( 420, 594 ) );
  presets << qMakePair( QStringLiteral( "A1" ), QSizeF( 594, 841 ) );
  presets << qMakePair( QStringLiteral( "A0" ), QSizeF( 841, 1189 ) );
  presets << qMakePair( QStringLiteral( "B5" ), QSizeF( 176, 250 ) );
  presets << qMakePair( QStringLiteral( "B4" ), QSizeF( 250, 353 ) );
  presets << qMakePair( QStringLiteral( "B3" ), QSizeF( 353, 500 ) );
  presets << qMakePair( QStringLiteral( "B2" ), QSizeF( 500, 707 ) );
  presets << qMakePair( QStringLiteral( "B1" ), QSizeF( 707, 1000 ) );
  presets << qMakePair( QStringLiteral( "B0" ), QSizeF( 1000, 1414 ) );
  // North american formats
  presets << qMakePair( QStringLiteral( "Legal" ), QSizeF( 215.9, 355.6 ) );
  presets << qMakePair( QStringLiteral( "Letter" ), QSizeF( 215.9, 279.4 ) );
  presets << qMakePair( QStringLiteral( "ANSI A" ), QSizeF( 215.9, 279.4 ) );
  presets << qMakePair( QStringLiteral( "ANSI B" ), QSizeF( 279.4, 431.8 ) );
  presets << qMakePair( QStringLiteral( "ANSI C" ), QSizeF( 431.8, 558.8 ) );
  presets << qMakePair( QStringLiteral( "ANSI D" ), QSizeF( 558.8, 863.6 ) );
  presets << qMakePair( QStringLiteral( "ANSI E" ), QSizeF( 863.6, 1117.6 ) );
  presets << qMakePair( QStringLiteral( "Arch A" ), QSizeF( 228.6, 304.8 ) );
  presets << qMakePair( QStringLiteral( "Arch B" ), QSizeF( 304.8, 457.2 ) );
  presets << qMakePair( QStringLiteral( "Arch C" ), QSizeF( 457.2, 609.6 ) );
  presets << qMakePair( QStringLiteral( "Arch D" ), QSizeF( 609.6, 914.4 ) );
  presets << qMakePair( QStringLiteral( "Arch E" ), QSizeF( 914.4, 1219.2 ) );
  presets << qMakePair( QStringLiteral( "Arch E1" ), QSizeF( 762, 1066.8 ) );

  QList< QPair< QString, QSizeF > >::const_iterator presetIt = presets.constBegin();
  for ( ; presetIt != presets.constEnd(); ++presetIt )
  {
    if ( presetString.compare( ( *presetIt ).first, Qt::CaseInsensitive ) == 0 )
    {
      width = ( *presetIt ).second.width();
      height = ( *presetIt ).second.height();
      return true;
    }
  }
  return false;
}

void QgsComposerUtils::readOldDataDefinedPropertyMap( const QDomElement &itemElem, QgsPropertyCollection &dataDefinedProperties )
{
  QgsPropertiesDefinition::const_iterator i = QgsComposerObject::propertyDefinitions().constBegin();
  for ( ; i != QgsComposerObject::propertyDefinitions().constEnd(); ++i )
  {
    QString elemName = i.value().name();
    QDomNodeList ddNodeList = itemElem.elementsByTagName( elemName );
    if ( !ddNodeList.isEmpty() )
    {
      QDomElement ddElem = ddNodeList.at( 0 ).toElement();
      QgsProperty prop = readOldDataDefinedProperty( static_cast< QgsComposerObject::DataDefinedProperty >( i.key() ), ddElem );
      if ( prop )
        dataDefinedProperties.setProperty( i.key(), prop );
    }
  }
}

QgsProperty QgsComposerUtils::readOldDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QDomElement &ddElem )
{
  if ( property == QgsComposerObject::AllProperties || property == QgsComposerObject::NoProperty )
  {
    //invalid property
    return QgsProperty();
  }

  QString active = ddElem.attribute( QStringLiteral( "active" ) );
  bool isActive = false;
  if ( active.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    isActive = true;
  }
  QString field = ddElem.attribute( QStringLiteral( "field" ) );
  QString expr = ddElem.attribute( QStringLiteral( "expr" ) );

  QString useExpr = ddElem.attribute( QStringLiteral( "useExpr" ) );
  bool isExpression = false;
  if ( useExpr.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    isExpression = true;
  }

  if ( isExpression )
    return QgsProperty::fromExpression( expr, isActive );
  else
    return QgsProperty::fromField( field, isActive );
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

double QgsComposerUtils::fontHeightCharacterMM( const QFont &font, QChar character )
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

double QgsComposerUtils::textHeightMM( const QFont &font, const QString &text, double multiLineHeight )
{
  QStringList multiLineSplit = text.split( '\n' );
  int lines = multiLineSplit.size();

  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );

  double fontHeight = fontMetrics.ascent() + fontMetrics.descent(); // ignore +1 for baseline
  double textHeight = fontMetrics.ascent() + static_cast< double >( ( lines - 1 ) * fontHeight * multiLineHeight );

  return textHeight / FONT_WORKAROUND_SCALE;
}

void QgsComposerUtils::drawText( QPainter *painter, QPointF pos, const QString &text, const QFont &font, const QColor &color )
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

QgsRenderContext QgsComposerUtils::createRenderContextForMap( QgsComposerMap *map, QPainter *painter, double dpi )
{
  if ( !map )
  {
    QgsRenderContext context;
    context.setPainter( painter );
    if ( dpi < 0 && painter && painter->device() )
    {
      context.setScaleFactor( painter->device()->logicalDpiX() / 25.4 );
    }
    else if ( dpi > 0 )
    {
      context.setScaleFactor( dpi / 25.4 );
    }
    else
    {
      context.setScaleFactor( 3.465 ); //assume 88 dpi as standard value
    }
    return context;
  }

  // default to 88 dpi if no painter specified
  if ( dpi < 0 )
  {
    dpi = ( painter && painter->device() ) ? painter->device()->logicalDpiX() : 88;
  }
  double dotsPerMM = dpi / 25.4;

  // get map settings from reference map
  QgsRectangle extent = *( map->currentMapExtent() );
  QSizeF mapSizeMM = map->rect().size();
  QgsMapSettings ms = map->mapSettings( extent, mapSizeMM * dotsPerMM, dpi );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  if ( painter )
    context.setPainter( painter );
  return context;
}

QgsRenderContext QgsComposerUtils::createRenderContextForComposition( QgsComposition *composition, QPainter *painter )
{
  QgsComposerMap *referenceMap = composition ? composition->referenceMap() : nullptr;
  return createRenderContextForMap( referenceMap, painter );
}
