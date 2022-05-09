/***************************************************************************
    qgsprojectstylesettings.cpp
    ---------------------------
    begin                : May 2022
    copyright            : (C) 2022 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectstylesettings.h"
#include "qgis.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgscolorramp.h"
#include "qgstextformat.h"

#include <QDomElement>

QgsProjectStyleSettings::QgsProjectStyleSettings( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

QgsSymbol *QgsProjectStyleSettings::defaultSymbol( Qgis::SymbolType symbolType ) const
{
  switch ( symbolType )
  {
    case Qgis::SymbolType::Marker:
      return mDefaultMarkerSymbol ? mDefaultMarkerSymbol->clone() : nullptr;

    case Qgis::SymbolType::Line:
      return mDefaultLineSymbol ? mDefaultLineSymbol->clone() : nullptr;

    case Qgis::SymbolType::Fill:
      return mDefaultFillSymbol ? mDefaultFillSymbol->clone() : nullptr;

    case Qgis::SymbolType::Hybrid:
      break;
  }

  return nullptr;
}

void QgsProjectStyleSettings::setDefaultSymbol( Qgis::SymbolType symbolType, QgsSymbol *symbol )
{
  switch ( symbolType )
  {
    case Qgis::SymbolType::Marker:
      mDefaultMarkerSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    case Qgis::SymbolType::Line:
      mDefaultLineSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    case Qgis::SymbolType::Fill:
      mDefaultFillSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    case Qgis::SymbolType::Hybrid:
      break;
  }
}

QgsColorRamp *QgsProjectStyleSettings::defaultColorRamp() const
{
  return mDefaultColorRamp ? mDefaultColorRamp->clone() : nullptr;
}

void QgsProjectStyleSettings::setDefaultColorRamp( QgsColorRamp *colorRamp )
{
  mDefaultColorRamp.reset( colorRamp ? colorRamp->clone() : nullptr );
}

QgsTextFormat QgsProjectStyleSettings::defaultTextFormat() const
{
  return mDefaultTextFormat;
}

void QgsProjectStyleSettings::setDefaultTextFormat( QgsTextFormat textFormat )
{
  mDefaultTextFormat = textFormat;
}

void QgsProjectStyleSettings::reset()
{
  mDefaultMarkerSymbol.reset();
  mDefaultLineSymbol.reset();
  mDefaultFillSymbol.reset();
  mDefaultColorRamp.reset();
  mDefaultTextFormat = QgsTextFormat();
  mRandomizeDefaultSymbolColor = true;
  mDefaultSymbolOpacity = 1.0;
}

bool QgsProjectStyleSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( mProject->pathResolver() );

  mRandomizeDefaultSymbolColor = element.attribute( QStringLiteral( "RandomizeDefaultSymbolColor" ), QStringLiteral( "0" ) ).toInt();
  mDefaultSymbolOpacity = element.attribute( QStringLiteral( "DefaultSymbolOpacity" ), QStringLiteral( "1.0" ) ).toDouble();

  QDomElement elem = element.firstChildElement( QStringLiteral( "markerSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultMarkerSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, rwContext ) : nullptr );
  }
  else
  {
    mDefaultMarkerSymbol.reset();
  }

  elem = element.firstChildElement( QStringLiteral( "lineSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultLineSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, rwContext ) : nullptr );
  }
  else
  {
    mDefaultLineSymbol.reset();
  }

  elem = element.firstChildElement( QStringLiteral( "fillSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultFillSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, rwContext ) : nullptr );
  }
  else
  {
    mDefaultFillSymbol.reset();
  }

  elem = element.firstChildElement( QStringLiteral( "colorramp" ) );
  mDefaultColorRamp.reset( !elem.isNull() ? QgsSymbolLayerUtils::loadColorRamp( elem ) : nullptr );

  elem = element.firstChildElement( QStringLiteral( "text-style" ) );
  if ( !elem.isNull() )
  {
    mDefaultTextFormat.readXml( elem, rwContext );
  }
  else
  {
    mDefaultTextFormat = QgsTextFormat();
  }

  return true;
}

QDomElement QgsProjectStyleSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectStyleSettings" ) );
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( mProject->pathResolver() );

  element.setAttribute( QStringLiteral( "RandomizeDefaultSymbolColor" ), mRandomizeDefaultSymbolColor ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "DefaultSymbolOpacity" ), QString::number( mDefaultSymbolOpacity ) );

  if ( mDefaultMarkerSymbol )
  {
    QDomElement markerSymbolElem = doc.createElement( QStringLiteral( "markerSymbol" ) );
    markerSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultMarkerSymbol.get(), doc, rwContext ) );
    element.appendChild( markerSymbolElem );
  }

  if ( mDefaultLineSymbol )
  {
    QDomElement lineSymbolElem = doc.createElement( QStringLiteral( "lineSymbol" ) );
    lineSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultLineSymbol.get(), doc, rwContext ) );
    element.appendChild( lineSymbolElem );
  }

  if ( mDefaultFillSymbol )
  {
    QDomElement fillSymbolElem = doc.createElement( QStringLiteral( "fillSymbol" ) );
    fillSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultFillSymbol.get(), doc, rwContext ) );
    element.appendChild( fillSymbolElem );
  }

  if ( mDefaultColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QString(), mDefaultColorRamp.get(), doc );
    element.appendChild( colorRampElem );
  }

  if ( mDefaultTextFormat.isValid() )
  {
    QDomElement textFormatElem = mDefaultTextFormat.writeXml( doc, rwContext );
    element.appendChild( textFormatElem );
  }

  return element;
}
