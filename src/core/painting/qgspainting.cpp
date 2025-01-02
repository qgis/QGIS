/***************************************************************************
    qgspainting.cpp
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspainting.h"
#include "qgslogger.h"

#include <QTransform>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

QPainter::CompositionMode QgsPainting::getCompositionMode( Qgis::BlendMode blendMode )
{
  // Map Qgis::BlendMode::Normal to QPainter::CompositionMode
  switch ( blendMode )
  {
    case Qgis::BlendMode::Normal:
      return QPainter::CompositionMode_SourceOver;
    case Qgis::BlendMode::Lighten:
      return QPainter::CompositionMode_Lighten;
    case Qgis::BlendMode::Screen:
      return QPainter::CompositionMode_Screen;
    case Qgis::BlendMode::Dodge:
      return QPainter::CompositionMode_ColorDodge;
    case Qgis::BlendMode::Addition:
      return QPainter::CompositionMode_Plus;
    case Qgis::BlendMode::Darken:
      return QPainter::CompositionMode_Darken;
    case Qgis::BlendMode::Multiply:
      return QPainter::CompositionMode_Multiply;
    case Qgis::BlendMode::Burn:
      return QPainter::CompositionMode_ColorBurn;
    case Qgis::BlendMode::Overlay:
      return QPainter::CompositionMode_Overlay;
    case Qgis::BlendMode::SoftLight:
      return QPainter::CompositionMode_SoftLight;
    case Qgis::BlendMode::HardLight:
      return QPainter::CompositionMode_HardLight;
    case Qgis::BlendMode::Difference:
      return QPainter::CompositionMode_Difference;
    case Qgis::BlendMode::Subtract:
      return QPainter::CompositionMode_Exclusion;
    case Qgis::BlendMode::Source:
      return QPainter::CompositionMode_Source;
    case Qgis::BlendMode::DestinationOver:
      return QPainter::CompositionMode_DestinationOver;
    case Qgis::BlendMode::Clear:
      return QPainter::CompositionMode_Clear;
    case Qgis::BlendMode::Destination:
      return QPainter::CompositionMode_Destination;
    case Qgis::BlendMode::SourceIn:
      return QPainter::CompositionMode_SourceIn;
    case Qgis::BlendMode::DestinationIn:
      return QPainter::CompositionMode_DestinationIn;
    case Qgis::BlendMode::SourceOut:
      return QPainter::CompositionMode_SourceOut;
    case Qgis::BlendMode::DestinationOut:
      return QPainter::CompositionMode_DestinationOut;
    case Qgis::BlendMode::SourceAtop:
      return QPainter::CompositionMode_SourceAtop;
    case Qgis::BlendMode::DestinationAtop:
      return QPainter::CompositionMode_DestinationAtop;
    case Qgis::BlendMode::Xor:
      return QPainter::CompositionMode_Xor;
    default:
      QgsDebugError( QStringLiteral( "Blend mode %1 mapped to SourceOver" ).arg( qgsEnumValueToKey( blendMode ) ) );
      return QPainter::CompositionMode_SourceOver;
  }
}


Qgis::BlendMode QgsPainting::getBlendModeEnum( QPainter::CompositionMode blendMode )
{
  // Map QPainter::CompositionMode to Qgis::BlendMode::Normal
  switch ( blendMode )
  {
    case QPainter::CompositionMode_SourceOver:
      return Qgis::BlendMode::Normal;
    case QPainter::CompositionMode_Lighten:
      return Qgis::BlendMode::Lighten;
    case QPainter::CompositionMode_Screen:
      return Qgis::BlendMode::Screen;
    case QPainter::CompositionMode_ColorDodge:
      return Qgis::BlendMode::Dodge;
    case QPainter::CompositionMode_Plus:
      return Qgis::BlendMode::Addition;
    case QPainter::CompositionMode_Darken:
      return Qgis::BlendMode::Darken;
    case QPainter::CompositionMode_Multiply:
      return Qgis::BlendMode::Multiply;
    case QPainter::CompositionMode_ColorBurn:
      return Qgis::BlendMode::Burn;
    case QPainter::CompositionMode_Overlay:
      return Qgis::BlendMode::Overlay;
    case QPainter::CompositionMode_SoftLight:
      return Qgis::BlendMode::SoftLight;
    case QPainter::CompositionMode_HardLight:
      return Qgis::BlendMode::HardLight;
    case QPainter::CompositionMode_Difference:
      return Qgis::BlendMode::Difference;
    case QPainter::CompositionMode_Exclusion:
      return Qgis::BlendMode::Subtract;
    case QPainter::CompositionMode_Source:
      return Qgis::BlendMode::Source;
    case QPainter::CompositionMode_DestinationOver:
      return Qgis::BlendMode::DestinationOver;
    case QPainter::CompositionMode_Clear:
      return Qgis::BlendMode::Clear;
    case QPainter::CompositionMode_Destination:
      return Qgis::BlendMode::Destination;
    case QPainter::CompositionMode_SourceIn:
      return Qgis::BlendMode::SourceIn;
    case QPainter::CompositionMode_DestinationIn:
      return Qgis::BlendMode::DestinationIn;
    case QPainter::CompositionMode_SourceOut:
      return Qgis::BlendMode::SourceOut;
    case QPainter::CompositionMode_DestinationOut:
      return Qgis::BlendMode::DestinationOut;
    case QPainter::CompositionMode_SourceAtop:
      return Qgis::BlendMode::SourceAtop;
    case QPainter::CompositionMode_DestinationAtop:
      return Qgis::BlendMode::DestinationAtop;
    case QPainter::CompositionMode_Xor:
      return Qgis::BlendMode::Xor;
    default:
      QgsDebugError( QStringLiteral( "Composition mode %1 mapped to Normal" ).arg( blendMode ) );
      return Qgis::BlendMode::Normal;
  }
}

bool QgsPainting::isClippingMode( Qgis::BlendMode mode )
{
  switch ( mode )
  {
    case Qgis::BlendMode::Normal:
    case Qgis::BlendMode::Lighten:
    case Qgis::BlendMode::Screen:
    case Qgis::BlendMode::Dodge:
    case Qgis::BlendMode::Addition:
    case Qgis::BlendMode::Darken:
    case Qgis::BlendMode::Multiply:
    case Qgis::BlendMode::Burn:
    case Qgis::BlendMode::Overlay:
    case Qgis::BlendMode::SoftLight:
    case Qgis::BlendMode::HardLight:
    case Qgis::BlendMode::Difference:
    case Qgis::BlendMode::Subtract:
    case Qgis::BlendMode::Source:
    case Qgis::BlendMode::DestinationOver:
    case Qgis::BlendMode::Clear:
    case Qgis::BlendMode::Destination:
      return false;

    case Qgis::BlendMode::SourceIn:
    case Qgis::BlendMode::DestinationIn:
    case Qgis::BlendMode::SourceOut:
    case Qgis::BlendMode::DestinationOut:
    case Qgis::BlendMode::SourceAtop:
    case Qgis::BlendMode::DestinationAtop:
    case Qgis::BlendMode::Xor:
      return true;
  }
  return false;
}

QTransform QgsPainting::triangleToTriangleTransform( double inX1, double inY1, double inX2, double inY2, double inX3, double inY3, double outX1, double outY1, double outX2, double outY2, double outX3, double outY3, bool &ok )
{
  // QTransform maps points using X' = X * T (not X' = T * X !)
  // So we are trying to solve the equation:  U * T = V, where U = input triangle and V = output triangle
  // Hence T = U^(-1) * V

  const QTransform U(
    inX1, inY1, 1,
    inX2, inY2, 1,
    inX3, inY3, 1 );

  const QTransform V(
    outX1, outY1, 1,
    outX2, outY2, 1,
    outX3, outY3, 1
  );

  return ( U.inverted( &ok ) ) * V;
}

bool QgsPainting::drawTriangleUsingTexture( QPainter *painter, const QPolygonF &triangle, const QImage &textureImage, float textureX1, float textureY1, float textureX2, float textureY2, float textureX3, float textureY3 )
{
  bool ok = false;
  const QTransform brushTransform = triangleToTriangleTransform(
                                      textureX1 * ( textureImage.width() - 1 ), textureY1 * ( textureImage.height() - 1 ),
                                      textureX2 * ( textureImage.width() - 1 ), textureY2 * ( textureImage.height() - 1 ),
                                      textureX3 * ( textureImage.width() - 1 ), textureY3 * ( textureImage.height() - 1 ),
                                      triangle.at( 0 ).x(), triangle.at( 0 ).y(),
                                      triangle.at( 1 ).x(), triangle.at( 1 ).y(),
                                      triangle.at( 2 ).x(), triangle.at( 2 ).y(),
                                      ok
                                    );
  if ( !ok )
    return false;

  // only store/restore the painter's current brush -- this is cheaper than saving/restoring the whole painter state
  const QBrush previousBrush = painter->brush();

  QBrush textureBrush( textureImage );
  textureBrush.setTransform( brushTransform );

  painter->setBrush( textureBrush );
  painter->drawPolygon( triangle );
  painter->setBrush( previousBrush );

  return true;
}

int QgsPainting::qtDefaultDpiX()
{
  return qt_defaultDpiX();
}

int QgsPainting::qtDefaultDpiY()
{
  return qt_defaultDpiY();
}

void QgsPainting::applyScaleFixForQPictureDpi( QPainter *painter )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  painter->scale( static_cast< double >( QgsPainting::qtDefaultDpiX() ) / painter->device()->logicalDpiX(),
                  static_cast< double >( QgsPainting::qtDefaultDpiY() ) / painter->device()->logicalDpiY() );
}

void QgsPainting::drawPicture( QPainter *painter, const QPointF &point, const QPicture &picture )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  const double xScale = static_cast< double >( QgsPainting::qtDefaultDpiX() ) / painter->device()->logicalDpiX();
  const double yScale = static_cast< double >( QgsPainting::qtDefaultDpiY() ) / painter->device()->logicalDpiY();
  painter->scale( xScale, yScale );
  painter->drawPicture( QPointF( point.x() / xScale, point.y() / yScale ), picture );
  painter->scale( 1 / xScale, 1 / yScale );
}
