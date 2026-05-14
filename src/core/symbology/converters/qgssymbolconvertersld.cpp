/***************************************************************************
   qgssymbolconvertersld.cpp
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

#include "qgssymbolconvertersld.h"

#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsreadwritecontext.h"
#include "qgssldexportcontext.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <QDomElement>
#include <QObject>
#include <QString>
#include <QVariant>

using namespace Qt::StringLiterals;

Qgis::SymbolConverterCapabilities QgsSymbolConverterSld::capabilities() const
{
  return Qgis::SymbolConverterCapability::ReadSymbol | Qgis::SymbolConverterCapability::WriteSymbol;
}

QString QgsSymbolConverterSld::name() const
{
  return u"sld"_s;
}

QString QgsSymbolConverterSld::formatName() const
{
  return QObject::tr( "Styled Layer Descriptor (SLD)" );
}

QVariant QgsSymbolConverterSld::toVariant( const QgsSymbol *symbol, QgsSymbolConverterContext &context ) const
{
  if ( !symbol )
    return QVariant();

  QDomDocument doc;

  QgsSldExportContext sldExportContext;

  QDomElement root = doc.createElement( u"Rule"_s );
  symbol->toSld( doc, root, sldExportContext );
  doc.appendChild( root );

  // copy errors, warnings from SLD export context
  for ( const QString &error : sldExportContext.errors() )
  {
    context.pushError( error );
  }
  for ( const QString &warning : sldExportContext.warnings() )
  {
    context.pushWarning( warning );
  }

  return doc.toString();
}

std::unique_ptr< QgsSymbol > QgsSymbolConverterSld::createSymbol( const QVariant &variant, QgsSymbolConverterContext &context ) const
{
  if ( variant.isNull() || !variant.canConvert<QString>() )
    return nullptr;

  QString xmlString = variant.toString();
  if ( xmlString.isEmpty() )
    return nullptr;

  if ( !xmlString.contains( "xmlns:se="_L1 ) )
  {
    // wrap in a dummy tag to get se xmlns processing
    xmlString = u"<?xml version=\"1.0\" encoding=\"UTF-8\"?><tmp xmlns:se=\"http://www.opengis.net/se\">%1</tmp>"_s.arg( xmlString );
  }

  QDomDocument doc;
  QString errorMsg;
  int errorLine, errorColumn;
  if ( !doc.setContent( xmlString, true, &errorMsg, &errorLine, &errorColumn ) )
  {
    context.pushError( QObject::tr( "Error parsing SLD content: %1" ).arg( errorMsg ) );
    return nullptr;
  }

  const QDomElement ruleElem = doc.documentElement().firstChildElement( u"Rule"_s );
  if ( ruleElem.isNull() )
  {
    context.pushError( QObject::tr( "Error parsing SLD content: no Rule elements found" ) );
    return nullptr;
  }

  QgsSymbolLayerList layers;

  Qgis::GeometryType geomType = Qgis::GeometryType::Unknown;
  switch ( context.typeHint() )
  {
    case Qgis::SymbolType::Marker:
      geomType = Qgis::GeometryType::Point;
      break;
    case Qgis::SymbolType::Line:
      geomType = Qgis::GeometryType::Line;
      break;
    case Qgis::SymbolType::Fill:
      geomType = Qgis::GeometryType::Polygon;
      break;
    case Qgis::SymbolType::Hybrid:
      break;
  }

  // retrieve the Rule element child nodes
  QDomElement childElem = ruleElem.firstChildElement();
  while ( !childElem.isNull() )
  {
    if ( childElem.localName().endsWith( "Symbolizer"_L1 ) )
    {
      QgsSymbolLayerUtils::createSymbolLayerListFromSld( childElem, geomType, layers );
    }

    childElem = childElem.nextSiblingElement();
  }

  if ( layers.isEmpty() )
    return nullptr;

  std::unique_ptr< QgsSymbol > symbol;
  switch ( geomType )
  {
    case Qgis::GeometryType::Line:
      symbol = std::make_unique< QgsLineSymbol >( layers );
      break;

    case Qgis::GeometryType::Polygon:
      symbol = std::make_unique< QgsFillSymbol >( layers );
      break;

    case Qgis::GeometryType::Point:
      symbol = std::make_unique< QgsMarkerSymbol >( layers );
      break;

    default:
      context.pushError( QObject::tr( "Invalid geometry type: found %1" ).arg( qgsEnumValueToKey( geomType ) ) );
      return nullptr;
  }

  return symbol;
}
