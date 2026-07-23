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

#include "qgs3dsymbolregistry.h"
#include "qgsabstract3dsymbol.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsapplication.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsline3dsymbol.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmetalroughmaterialsettings.h"
#include "qgsmetalroughtexturedmaterialsettings.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QString>

using namespace Qt::StringLiterals;

QColor Qgs3DSymbolUtils::vectorSymbolAverageColor( const QgsAbstract3DSymbol *symbol )
{
  QColor color = QColor();

  if ( !symbol )
  {
    return color;
  }

  QgsAbstractMaterialSettings *materialSettings = symbol->materialSettings();
  if ( materialSettings )
  {
    color = materialSettings->averageColor();
  }
  else
  {
    QgsDebugError( u"Qgs3DSymbolUtils::vectorMaterialAverageColor: unable to retrieve material from symbol"_s );
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
  if ( !symbol )
  {
    return false;
  }

  bool colorSet = false;
  QgsAbstractMaterialSettings *materialSettings = symbol->materialSettings();
  if ( materialSettings )
  {
    materialSettings->setColorsFromBase( baseColor );
    colorSet = true;
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

  if ( fromSymbol && toSymbol )
  {
    toSymbol->setMaterialSettings( fromSymbol->materialSettings()->clone() );
    copied = true;
  }
  else
  {
    QgsDebugError( u"Qgs3DSymbolUtils::copyVectorSymbolMaterial: unable to retrieve material from symbols"_s );
  }

  return copied;
}

std::unique_ptr<QgsAbstract3DSymbol> Qgs3DSymbolUtils::create3DSymbolFrom2D( const QgsVectorLayer *vLayer, const QgsSymbol *symbol2D, const QgsRenderContext &context )
{
  if ( !symbol2D || !vLayer )
  {
    return nullptr;
  }

  auto symbol3D = std::unique_ptr<QgsAbstract3DSymbol>( QgsApplication::symbol3DRegistry()->defaultSymbolForGeometryType( vLayer->geometryType() ) );
  symbol3D->setDefaultPropertiesFromLayer( vLayer );

  // set the main color
  Qgs3DSymbolUtils::setVectorSymbolBaseColor( symbol3D.get(), symbol2D->color() );
  if ( symbol3D->type() == "line"_L1 )
  {
    QgsLine3DSymbol *lineSymbol3D = dynamic_cast<QgsLine3DSymbol *>( symbol3D.get() );
    // lines geometry type - retrieve its width
    if ( const QgsLineSymbol *lineSymbol = dynamic_cast<const QgsLineSymbol *>( symbol2D ) )
    {
      if ( lineSymbol->symbolLayerCount() > 0 )
      {
        const QgsSymbolLayer *symbolLayer = lineSymbol->symbolLayer( 0 );
        if ( const QgsSimpleLineSymbolLayer *simpleLineLayer = dynamic_cast<const QgsSimpleLineSymbolLayer *>( symbolLayer ) )
        {
          const double lineWidthPixels = std::max( 1.0, context.convertToPainterUnits( simpleLineLayer->width(), simpleLineLayer->widthUnit() ) );
          lineSymbol3D->setWidth( static_cast<float>( lineWidthPixels ) );
        }
      }
    }
  }
  else if ( symbol3D->type() == "point"_L1 )
  {
    QgsPoint3DSymbol *pointSymbol3D = dynamic_cast<QgsPoint3DSymbol *>( symbol3D.get() );
    if ( const QgsMarkerSymbol *markerSymbol = dynamic_cast<const QgsMarkerSymbol *>( symbol2D ) )
    {
      if ( markerSymbol->symbolLayerCount() > 0 )
      {
        const QgsSymbolLayer *symbolLayer = markerSymbol->symbolLayer( 0 );
        if ( const QgsSimpleMarkerSymbolLayer *simpleMarkerLayer = dynamic_cast<const QgsSimpleMarkerSymbolLayer *>( symbolLayer ) )
        {
          const double sizeMapUnits = std::max( 1.0, context.convertToMapUnits( markerSymbol->size(), markerSymbol->sizeUnit() ) );

          switch ( simpleMarkerLayer->shape() )
          {
            case Qgis::MarkerShape::Circle:
            {
              pointSymbol3D->setShape( Qgis::Point3DShape::Sphere );
              QVariantMap vmSphere;
              vmSphere[u"radius"_s] = sizeMapUnits / 2.;
              pointSymbol3D->setShapeProperties( vmSphere );
              break;
            }
            case Qgis::MarkerShape::Square:
            {
              pointSymbol3D->setShape( Qgis::Point3DShape::Cube );
              QVariantMap vmCube;
              vmCube[u"size"_s] = sizeMapUnits;
              pointSymbol3D->setShapeProperties( vmCube );
              break;
            }

            case Qgis::MarkerShape::Triangle:
            case Qgis::MarkerShape::EquilateralTriangle:
            {
              QVariantMap vmCone;
              vmCone[u"length"_s] = sizeMapUnits;
              vmCone[u"topRadius"_s] = sizeMapUnits / 10.;
              vmCone[u"bottomRadius"_s] = sizeMapUnits / 2.;
              pointSymbol3D->setShapeProperties( vmCone );
              pointSymbol3D->setShape( Qgis::Point3DShape::Cone );
              break;
            }

            default:
              break;
          }
        }
      }
    }
  }

  // handle opacity
  QgsAbstractMaterialSettings *materialSettings = symbol3D->materialSettings();
  if ( materialSettings->type() == "phong"_L1 )
  {
    QgsPhongMaterialSettings *phongSettings = dynamic_cast<QgsPhongMaterialSettings *>( materialSettings );
    phongSettings->setOpacity( symbol2D->opacity() );
  }
  else if ( materialSettings->type() == "phongtextured"_L1 )
  {
    QgsPhongTexturedMaterialSettings *phongTexturedSettings = dynamic_cast<QgsPhongTexturedMaterialSettings *>( materialSettings );
    phongTexturedSettings->setOpacity( symbol2D->opacity() );
  }
  else if ( materialSettings->type() == "metalroughtextured"_L1 )
  {
    QgsMetalRoughTexturedMaterialSettings *metalTexturedSettings = dynamic_cast<QgsMetalRoughTexturedMaterialSettings *>( materialSettings );
    metalTexturedSettings->setOpacity( symbol2D->opacity() );
  }
  else if ( materialSettings->type() == "metalrough"_L1 )
  {
    QgsMetalRoughMaterialSettings *metalSettings = dynamic_cast<QgsMetalRoughMaterialSettings *>( materialSettings );
    metalSettings->setOpacity( symbol2D->opacity() );
  }

  return symbol3D;
}
