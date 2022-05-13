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
#include "qgsstyle.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
#include "qgscombinedstylemodel.h"
#endif

#include <QDomElement>

QgsProjectStyleSettings::QgsProjectStyleSettings( QgsProject *project )
  : QObject( project )
  , mProject( project )
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  mCombinedStyleModel = new QgsCombinedStyleModel( this );
#endif
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

void QgsProjectStyleSettings::setDefaultTextFormat( const QgsTextFormat &textFormat )
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

  mStyleDatabases.clear();
  qDeleteAll( mStyles );
  mStyles.clear();

  emit styleDatabasesChanged();
}

bool QgsProjectStyleSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mRandomizeDefaultSymbolColor = element.attribute( QStringLiteral( "RandomizeDefaultSymbolColor" ), QStringLiteral( "0" ) ).toInt();
  mDefaultSymbolOpacity = element.attribute( QStringLiteral( "DefaultSymbolOpacity" ), QStringLiteral( "1.0" ) ).toDouble();

  QDomElement elem = element.firstChildElement( QStringLiteral( "markerSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultMarkerSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context ) : nullptr );
  }
  else
  {
    mDefaultMarkerSymbol.reset();
  }

  elem = element.firstChildElement( QStringLiteral( "lineSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultLineSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) : nullptr );
  }
  else
  {
    mDefaultLineSymbol.reset();
  }

  elem = element.firstChildElement( QStringLiteral( "fillSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultFillSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context ) : nullptr );
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
    mDefaultTextFormat.readXml( elem, context );
  }
  else
  {
    mDefaultTextFormat = QgsTextFormat();
  }

  {
    qDeleteAll( mStyles );
    mStyles.clear();
    mStyleDatabases.clear();
    const QDomElement styleDatabases = element.firstChildElement( QStringLiteral( "databases" ) );
    if ( !styleDatabases.isNull() )
    {
      const QDomNodeList styleEntries = styleDatabases.childNodes();
      for ( int i = 0; i < styleEntries.count(); ++i )
      {
        const QDomElement styleElement = styleEntries.at( i ).toElement();
        const QString path = styleElement.attribute( QStringLiteral( "path" ) );
        const QString fullPath = context.pathResolver().readPath( path );
        mStyleDatabases.append( fullPath );

        QgsStyle *style = new QgsStyle( this );
        style->load( fullPath );
        style->setName( QFileInfo( fullPath ).completeBaseName() );
        mStyles.append( style );
      }
    }
  }
  emit styleDatabasesChanged();

  return true;
}

QDomElement QgsProjectStyleSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectStyleSettings" ) );

  element.setAttribute( QStringLiteral( "RandomizeDefaultSymbolColor" ), mRandomizeDefaultSymbolColor ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "DefaultSymbolOpacity" ), QString::number( mDefaultSymbolOpacity ) );

  if ( mDefaultMarkerSymbol )
  {
    QDomElement markerSymbolElem = doc.createElement( QStringLiteral( "markerSymbol" ) );
    markerSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultMarkerSymbol.get(), doc, context ) );
    element.appendChild( markerSymbolElem );
  }

  if ( mDefaultLineSymbol )
  {
    QDomElement lineSymbolElem = doc.createElement( QStringLiteral( "lineSymbol" ) );
    lineSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultLineSymbol.get(), doc, context ) );
    element.appendChild( lineSymbolElem );
  }

  if ( mDefaultFillSymbol )
  {
    QDomElement fillSymbolElem = doc.createElement( QStringLiteral( "fillSymbol" ) );
    fillSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultFillSymbol.get(), doc, context ) );
    element.appendChild( fillSymbolElem );
  }

  if ( mDefaultColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QString(), mDefaultColorRamp.get(), doc );
    element.appendChild( colorRampElem );
  }

  if ( mDefaultTextFormat.isValid() )
  {
    QDomElement textFormatElem = mDefaultTextFormat.writeXml( doc, context );
    element.appendChild( textFormatElem );
  }

  {
    QDomElement styleDatabases = doc.createElement( QStringLiteral( "databases" ) );
    for ( const QString &db : mStyleDatabases )
    {
      QDomElement dbElement = doc.createElement( QStringLiteral( "db" ) );
      dbElement.setAttribute( QStringLiteral( "path" ), context.pathResolver().writePath( db ) );
      styleDatabases.appendChild( dbElement );
    }
    element.appendChild( styleDatabases );
  }

  return element;
}

QList<QgsStyle *> QgsProjectStyleSettings::styles() const
{
  QList< QgsStyle * > res;
  res.reserve( mStyles.size() );
  for ( QgsStyle *style : mStyles )
  {
    if ( style )
      res.append( style );
  }
  return res;
}

void QgsProjectStyleSettings::addStyleDatabasePath( const QString &path )
{
  if ( mStyleDatabases.contains( path ) )
    return;

  mStyleDatabases.append( path );

  QgsStyle *style = new QgsStyle( this );
  style->load( path );
  style->setName( QFileInfo( path ).completeBaseName() );
  mStyles.append( style );

  emit styleDatabasesChanged();
}

void QgsProjectStyleSettings::setStyleDatabasePaths( const QStringList &paths )
{
  if ( paths == mStyleDatabases )
    return;

  qDeleteAll( mStyles );
  mStyles.clear();

  for ( const QString &path : paths )
  {
    QgsStyle *style = new QgsStyle( this );
    style->load( path );
    style->setName( QFileInfo( path ).completeBaseName() );
    mStyles.append( style );
  }

  mStyleDatabases = paths;
  emit styleDatabasesChanged();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
QgsCombinedStyleModel *QgsProjectStyleSettings::combinedStyleModel()
{
  return mCombinedStyleModel;
}
#endif
