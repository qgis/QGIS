/***************************************************************************
  qgs3dsymbolutils.cpp
  --------------------------------------
  Date                 : January 2026
  Copyright            : (C) 2026 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dsymbolutils.h"

#include "qgsabstract3dsymbol.h"
#include "qgsgoochmaterialsettings.h"
#include "qgsline3dsymbol.h"
#include "qgslogger.h"
#include "qgsmetalroughmaterialsettings.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgssimplelinematerialsettings.h"

#include <QPainter>
#include <QPainterPath>
#include <QString>

using namespace Qt::StringLiterals;

QColor Qgs3DSymbolUtils::vectorSymbolAverageColor( const QgsAbstract3DSymbol *symbol )
{
  QColor color = Qt::black;

  QgsAbstractMaterialSettings *materialSettings = nullptr;
  if ( symbol->type() == "line"_L1 )
  {
    const QgsLine3DSymbol *lineSymbol = dynamic_cast<const QgsLine3DSymbol *>( symbol );
    materialSettings = lineSymbol->materialSettings();
  }
  else if ( symbol->type() == "point"_L1 )
  {
    const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( symbol );
    materialSettings = pointSymbol->materialSettings();
  }
  else if ( symbol->type() == "polygon"_L1 )
  {
    const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( symbol );
    materialSettings = polygonSymbol->materialSettings();
  }

  if ( materialSettings )
  {
    color = materialSettings->averageColor();
  }
  else
  {
    QgsDebugError( u"Qgs3DSymbolUtils::vectorMaterialAverageColor: unable to retrieve material from symbol"_s );
    color = Qt::black;
  }

  return color;
}

QIcon Qgs3DSymbolUtils::vectorSymbolPreviewIcon( const QgsAbstract3DSymbol *symbol, const QSize &size, const QgsScreenProperties &screen, int padding )
{
  if ( !symbol || ( symbol->type() != "line"_L1 && symbol->type() == "point"_L1 && symbol->type() == "polygon"_L1 ) )
  {
    QgsDebugError( u"A vector symbol is expected by the Qgs3DSymbolUtils::vectorSymbolPreviewIcon function"_s );
    return QIcon();
  }

  const double devicePixelRatio = screen.isValid() ? screen.devicePixelRatio() : 1;
  QPixmap pixmap( size * devicePixelRatio );
  pixmap.setDevicePixelRatio( devicePixelRatio );
  pixmap.fill( Qt::transparent );

  QPainter painter( &pixmap );
  painter.setRenderHint( QPainter::Antialiasing );
  painter.setPen( Qt::NoPen );

  const QColor baseColor = Qgs3DSymbolUtils::vectorSymbolAverageColor( symbol );
  painter.setBrush( QBrush( baseColor ) );

  if ( symbol->type() == "line"_L1 )
  {
    // For lines, the icon size depends on the width of the line
    // draw a line
    const QgsLine3DSymbol *lineSymbol = dynamic_cast<const QgsLine3DSymbol *>( symbol );
    const int lineHeight = std::min( static_cast<int>( lineSymbol->width() ), size.height() - 2 * padding );
    const int y = ( size.height() - lineHeight ) / 2;
    painter.drawRect( padding, y, size.width() - 2 * padding, lineHeight );
  }
  else if ( symbol->type() == "point"_L1 )
  {
    // For points, the icon size depends on the shape of the symbol
    const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( symbol );
    switch ( pointSymbol->shape() )
    {
      case Qgis::Point3DShape::Cone:
      {
        // draw a projected cone
        float bottomRadius = pointSymbol->shapeProperty( u"bottomRadius"_s ).toFloat();
        float topRadius = 2 * pointSymbol->shapeProperty( u"topRadius"_s ).toFloat();
        float length = pointSymbol->shapeProperty( u"length"_s ).toFloat();

        const float maxRadius = static_cast<float>( size.width() ) / 2.0f - static_cast<float>( padding );
        const float maxLength = static_cast<float>( size.height() ) - 2.0f * static_cast<float>( padding );

        bottomRadius = std::min( bottomRadius, maxRadius );
        topRadius = std::min( topRadius, maxRadius );
        length = std::min( length, maxLength );

        const float centerX = static_cast<float>( size.width() ) / 2.0f;
        const float availableHeight = static_cast<float>( size.height() - 2 * padding );
        const float topY = static_cast<float>( padding ) + ( availableHeight - length ) / 2.0f;
        const float bottomY = topY + length;

        QPainterPath path;
        const QPointF topLeft( centerX - topRadius, topY );
        const QPointF topRight( centerX + topRadius, topY );
        const QPointF bottomLeft( centerX - bottomRadius, bottomY );
        const QPointF bottomRight( centerX + bottomRadius, bottomY );

        path.moveTo( topLeft );
        path.lineTo( bottomLeft );
        path.lineTo( bottomRight );
        path.lineTo( topRight );
        path.closeSubpath();
        painter.fillPath( path, painter.brush() );
        break;
      }
      case Qgis::Point3DShape::Cube:
      case Qgis::Point3DShape::Plane:
      {
        // draw a rectangle
        const int pointSize = static_cast<int>( pointSymbol->shapeProperty( u"size"_s ).toFloat() );
        const int rectSize = std::min( std::min( pointSize, size.height() ), size.width() ) - 2 * padding;
        const int rectX = ( size.width() - rectSize ) / 2;
        const int rectY = ( size.height() - rectSize ) / 2;
        painter.drawRect( rectX, rectY, rectSize, rectSize );
        break;
      }
      case Qgis::Point3DShape::Cylinder:
      {
        // draw a projected cylinder
        float radius = pointSymbol->shapeProperty( u"radius"_s ).toFloat();
        float length = pointSymbol->shapeProperty( u"length"_s ).toFloat();
        float centerX = static_cast<float>( size.width() ) / 2.0f;

        float maxRadius = static_cast<float>( size.width() ) / 2.0f - static_cast<float>( padding );
        float maxLength = static_cast<float>( size.height() - 2 * padding );
        radius = std::min( radius, maxRadius );
        length = std::min( length, maxLength );

        float availableHeight = static_cast<float>( size.height() - 2 * padding );
        float topY = static_cast<float>( padding ) + ( availableHeight - length ) / 2.0f;
        float bottomY = topY + length;

        QPainterPath path;
        QPointF topLeft( centerX - radius, topY );
        QPointF topRight( centerX + radius, topY );
        QPointF bottomLeft( centerX - radius, bottomY );
        QPointF bottomRight( centerX + radius, bottomY );

        path.moveTo( topLeft );
        path.lineTo( bottomLeft );
        path.lineTo( bottomRight );
        path.lineTo( topRight );
        path.closeSubpath();

        painter.fillPath( path, painter.brush() );
        painter.drawPath( path );
        break;
      }
      case Qgis::Point3DShape::Sphere:
      {
        // draw a circle
        const float pointSize = 2 * pointSymbol->shapeProperty( u"radius"_s ).toFloat();
        const int diameter = std::min( std::min( static_cast<int>( pointSize ), size.width() ), size.height() ) - 2 * padding;
        const int x = ( size.width() - diameter ) / 2;
        const int y = ( size.height() - diameter ) / 2;
        painter.drawEllipse( x, y, diameter, diameter );
        break;
      }
      case Qgis::Point3DShape::Torus:
      {
        // draw a projected torus
        float radius = pointSymbol->shapeProperty( u"radius"_s ).toFloat();
        float minorRadius = pointSymbol->shapeProperty( u"minorRadius"_s ).toFloat();

        float maxRadius = std::min( static_cast<float>( size.width() ), static_cast<float>( size.height() ) ) / 2.0f - static_cast<float>( padding );
        radius = std::min( radius, maxRadius );
        minorRadius = std::min( minorRadius, radius );

        // Exterior ellipse
        const float centerX = static_cast<float>( size.width() ) / 2.0f;
        const float centerY = static_cast<float>( size.height() ) / 2.0f;
        painter.drawEllipse( QPointF( centerX, centerY ), radius, radius );

        // Interior ellipse
        painter.save();
        painter.setCompositionMode( QPainter::CompositionMode_Clear );
        painter.drawEllipse( QPointF( centerX, centerY ), minorRadius, minorRadius );
        painter.restore();
        break;
      }
      case Qgis::Point3DShape::Billboard:
      case Qgis::Point3DShape::ExtrudedText:
      case Qgis::Point3DShape::Model:
      {
        // fallback - draw a rectangle
        const int shapeSize = 10;
        const int rectSize = std::min( std::min( shapeSize, size.height() ), size.width() ) - 2 * padding;
        const int rectX = ( size.width() - rectSize ) / 2;
        const int rectY = ( size.height() - rectSize ) / 2;
        painter.drawRect( rectX, rectY, rectSize, rectSize );
        break;
      }
    }
  }
  else if ( symbol->type() == "polygon"_L1 )
  {
    // draw a rectangle
    const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( symbol );
    if ( polygonSymbol->edgesEnabled() )
    {
      painter.setPen( QPen( polygonSymbol->edgeColor(), polygonSymbol->edgeWidth() ) );
    }
    painter.drawRect( padding, padding, size.width() - 2 * padding, size.height() - 2 * padding );
  }

  painter.end();
  return QIcon( pixmap );
}

bool Qgs3DSymbolUtils::setVectorSymbolBaseColor( QgsAbstract3DSymbol *symbol, const QColor &baseColor )
{
  bool colorSet = false;

  QgsAbstractMaterialSettings *materialSettings = nullptr;
  if ( symbol->type() == "line"_L1 )
  {
    const QgsLine3DSymbol *lineSymbol = dynamic_cast<const QgsLine3DSymbol *>( symbol );
    materialSettings = lineSymbol->materialSettings();
  }
  else if ( symbol->type() == "point"_L1 )
  {
    const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( symbol );
    materialSettings = pointSymbol->materialSettings();
  }
  else if ( symbol->type() == "polygon"_L1 )
  {
    const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( symbol );
    materialSettings = polygonSymbol->materialSettings();
  }
  else
  {
    QgsDebugError( u"Qgs3DSymbolUtils::setVectorSymbolBaseColor does not support '%1' symbol"_s.arg( symbol->type() ) );
  }

  if ( materialSettings )
  {
    materialSettings->setColorsFromBase( baseColor );
  }
  else
  {
    QgsDebugError( u"Qgs3DSymbolUtils::setVectorSymbolBaseColor: unable to retrieve material from symbol"_s );
  }

  return colorSet;
}

bool Qgs3DSymbolUtils::copyVectorSymbolMaterial( const QgsAbstract3DSymbol *fromSymbol, QgsAbstract3DSymbol *toSymbol )
{
  bool copied = false;

  if ( toSymbol->type() == "line"_L1 )
  {
    const QgsLine3DSymbol *fromLineSymbol = dynamic_cast<const QgsLine3DSymbol *>( fromSymbol );
    QgsLine3DSymbol *toLineSymbol = dynamic_cast<QgsLine3DSymbol *>( toSymbol );
    if ( fromLineSymbol && toLineSymbol )
    {
      toLineSymbol->setMaterialSettings( fromLineSymbol->materialSettings()->clone() );
      copied = true;
    }
  }
  else if ( toSymbol->type() == "point"_L1 )
  {
    const QgsPoint3DSymbol *fromPointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( fromSymbol );
    QgsPoint3DSymbol *toPointSymbol = dynamic_cast<QgsPoint3DSymbol *>( toSymbol );
    if ( fromPointSymbol && toPointSymbol )
    {
      toPointSymbol->setMaterialSettings( fromPointSymbol->materialSettings()->clone() );
      copied = true;
    }
  }
  else if ( toSymbol->type() == "polygon"_L1 )
  {
    const QgsPolygon3DSymbol *fromPolygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( fromSymbol );
    QgsPolygon3DSymbol *toPolygonSymbol = dynamic_cast<QgsPolygon3DSymbol *>( toSymbol );
    if ( fromPolygonSymbol && toPolygonSymbol )
    {
      toPolygonSymbol->setMaterialSettings( fromPolygonSymbol->materialSettings()->clone() );
      copied = true;
    }
  }
  else
  {
    QgsDebugError( u"Qgs3DSymbolUtils::copyVectorSymbolMaterial does not support '%1' symbol"_s.arg( toSymbol->type() ) );
  }

  return copied;
}
