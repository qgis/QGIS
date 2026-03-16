/***************************************************************************
   qgssymbolconverterogrstyle.cpp
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

#include "qgssymbolconverterogrstyle.h"

#include "qgsogrutils.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include <QObject>
#include <QString>
#include <QVariant>

using namespace Qt::StringLiterals;

Qgis::SymbolConverterCapabilities QgsSymbolConverterOgrStyle::capabilities() const
{
  return Qgis::SymbolConverterCapability::ReadSymbol;
}

QString QgsSymbolConverterOgrStyle::name() const
{
  return u"ogr"_s;
}

QString QgsSymbolConverterOgrStyle::formatName() const
{
  return QObject::tr( "OGR Style String" );
}

QVariant QgsSymbolConverterOgrStyle::toVariant( const QgsSymbol *, QgsSymbolConverterContext & ) const
{
  throw QgsNotSupportedException( u"This symbol converter does not support serialization of symbols"_s );
}

std::unique_ptr< QgsSymbol > QgsSymbolConverterOgrStyle::createSymbol( const QVariant &variant, QgsSymbolConverterContext &context ) const
{
  const QString styleString = variant.toString();
  if ( styleString.isEmpty() )
    return nullptr;

  const Qgis::SymbolType typeHint = context.typeHint();
  if ( typeHint == Qgis::SymbolType::Hybrid )
  {
    context.pushError( QObject::tr( "Hybrid symbol types are not supported by the OGR converter." ) );
    return nullptr;
  }

  return QgsOgrUtils::symbolFromStyleString( styleString, typeHint );
}
