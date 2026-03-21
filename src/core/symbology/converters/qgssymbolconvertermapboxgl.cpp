/***************************************************************************
   qgssymbolconvertermapboxgl.cpp
   ----------------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgssymbolconvertermapboxgl.h"

#include "qgsjsonutils.h"
#include "qgsmapboxglstyleconverter.h"
#include "qgssymbol.h"
#include "qgsvectortilebasicrenderer.h"

#include <QObject>
#include <QString>
#include <QVariant>

using namespace Qt::StringLiterals;

Qgis::SymbolConverterCapabilities QgsSymbolConverterMapBoxGl::capabilities() const
{
  return Qgis::SymbolConverterCapability::ReadSymbol;
}

QString QgsSymbolConverterMapBoxGl::name() const
{
  return u"mapboxgl"_s;
}

QString QgsSymbolConverterMapBoxGl::formatName() const
{
  return QObject::tr( "MapBox GL Style" );
}

QVariant QgsSymbolConverterMapBoxGl::toVariant( const QgsSymbol *, QgsSymbolConverterContext & ) const
{
  throw QgsNotSupportedException( u"This symbol converter does not support serialization of symbols"_s );
}

std::unique_ptr< QgsSymbol > QgsSymbolConverterMapBoxGl::createSymbol( const QVariant &variant, QgsSymbolConverterContext &context ) const
{
  if ( variant.isNull() )
    return nullptr;

  QVariantMap jsonLayer;
  if ( variant.userType() == QMetaType::Type::QVariantMap )
  {
    jsonLayer = variant.toMap();
  }
  else if ( variant.canConvert<QString>() )
  {
    jsonLayer = QgsJsonUtils::parseJson( variant.toString() ).toMap();
  }

  if ( jsonLayer.isEmpty() || !jsonLayer.contains( u"type"_s ) )
  {
    context.pushError( QObject::tr( "Invalid MapBox GL JSON: Missing 'type' property." ) );
    return nullptr;
  }

  const QString layerType = jsonLayer.value( u"type"_s ).toString();

  QgsMapBoxGlStyleConversionContext mbContext;
  QgsVectorTileBasicRendererStyle style;
  bool success = false;

  if ( layerType == "fill"_L1 )
  {
    success = QgsMapBoxGlStyleConverter::parseFillLayer( jsonLayer, style, mbContext );
  }
  else if ( layerType == "line"_L1 )
  {
    success = QgsMapBoxGlStyleConverter::parseLineLayer( jsonLayer, style, mbContext );
  }
  else if ( layerType == "circle"_L1 )
  {
    success = QgsMapBoxGlStyleConverter::parseCircleLayer( jsonLayer, style, mbContext );
  }
  else if ( layerType == "symbol"_L1 )
  {
    success = QgsMapBoxGlStyleConverter::parseSymbolLayerAsRenderer( jsonLayer, style, mbContext );
  }
  else if ( layerType == "background"_L1 )
  {
    success = QgsMapBoxGlStyleConverter::parseFillLayer( jsonLayer, style, mbContext, true );
  }
  else
  {
    context.pushError( QObject::tr( "Unsupported MapBox GL layer type: %1" ).arg( layerType ) );
    return nullptr;
  }

  const QStringList warnings = mbContext.warnings();
  for ( const QString &warning : warnings )
  {
    context.pushWarning( warning );
  }

  if ( success && style.symbol() )
  {
    return std::unique_ptr< QgsSymbol >( style.symbol()->clone() );
  }

  return nullptr;
}
