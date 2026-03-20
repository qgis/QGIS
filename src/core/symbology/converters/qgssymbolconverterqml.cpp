/***************************************************************************
   qgssymbolconverterqml.cpp
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

#include "qgssymbolconverterqml.h"

#include "qgsreadwritecontext.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <QDomElement>
#include <QObject>
#include <QString>
#include <QVariant>

using namespace Qt::StringLiterals;

Qgis::SymbolConverterCapabilities QgsSymbolConverterQml::capabilities() const
{
  return Qgis::SymbolConverterCapability::ReadSymbol | Qgis::SymbolConverterCapability::WriteSymbol;
}

QString QgsSymbolConverterQml::name() const
{
  return u"qml"_s;
}

QString QgsSymbolConverterQml::formatName() const
{
  return QObject::tr( "QGIS QML Style" );
}

QVariant QgsSymbolConverterQml::toVariant( const QgsSymbol *symbol, QgsSymbolConverterContext &context ) const
{
  if ( !symbol )
    return QVariant();

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType( u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s );
  QDomDocument doc( documentType );

  QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( u"symbol"_s, symbol, doc, context.readWriteContext() );

  if ( symbolElem.isNull() )
    return QVariant();

  doc.appendChild( symbolElem );
  return doc.toString();
}

std::unique_ptr< QgsSymbol > QgsSymbolConverterQml::createSymbol( const QVariant &variant, QgsSymbolConverterContext &context ) const
{
  const QString xmlString = variant.toString();
  if ( xmlString.isEmpty() )
    return nullptr;

  QDomDocument doc;
  QString errorMsg;
  int errorLine, errorColumn;
  if ( !doc.setContent( xmlString, &errorMsg, &errorLine, &errorColumn ) )
  {
    context.pushError( QObject::tr( "Error parsing QML content: %1" ).arg( errorMsg ) );
    return nullptr;
  }

  QDomElement root = doc.documentElement();
  if ( root.isNull() )
  {
    context.pushError( QObject::tr( "QML document is empty" ) );
    return nullptr;
  }

  return QgsSymbolLayerUtils::loadSymbol( root, context.readWriteContext() );
}
