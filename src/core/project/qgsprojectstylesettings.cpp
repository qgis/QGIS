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

QgsSymbol *QgsProjectStyleSettings::defaultSymbol( QgsWkbTypes::GeometryType geomType ) const
{
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      return mDefaultMarkerSymbol ? mDefaultMarkerSymbol->clone() : nullptr;

    case QgsWkbTypes::LineGeometry:
      return mDefaultLineSymbol ? mDefaultLineSymbol->clone() : nullptr;

    case QgsWkbTypes::PolygonGeometry:
      return mDefaultFillSymbol ? mDefaultFillSymbol->clone() : nullptr;

    default:
      break;
  }

  return nullptr;
}

void QgsProjectStyleSettings::setDefaultSymbol( QgsWkbTypes::GeometryType geomType, QgsSymbol *symbol )
{
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      mDefaultMarkerSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    case QgsWkbTypes::LineGeometry:
      mDefaultLineSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    case QgsWkbTypes::PolygonGeometry:
      mDefaultFillSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    default:
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
    if ( !symbolElem.isNull() )
    {
      mDefaultMarkerSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, rwContext ) );
    }
  }

  elem = element.firstChildElement( QStringLiteral( "lineSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      mDefaultLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, rwContext ) );
    }
  }

  elem = element.firstChildElement( QStringLiteral( "fillSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      mDefaultFillSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, rwContext ) );
    }
  }

  elem = element.firstChildElement( QStringLiteral( "colorramp" ) );
  if ( !elem.isNull() )
  {
    mDefaultColorRamp.reset( QgsSymbolLayerUtils::loadColorRamp( elem ) );
  }

  elem = element.firstChildElement( QStringLiteral( "text-style" ) );
  if ( !elem.isNull() )
  {
    mDefaultTextFormat.readXml( elem, rwContext );
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
