/***************************************************************************
                             qgsnumericformatregistry.cpp
                             ----------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnumericformatregistry.h"
#include "qgsnumericformat.h"

#include "qgsfallbacknumericformat.h"
#include "qgsbasicnumericformat.h"
#include "qgsbearingnumericformat.h"
#include "qgscurrencynumericformat.h"
#include "qgspercentagenumericformat.h"
#include "qgsscientificnumericformat.h"
#include "qgsfractionnumericformat.h"
#include "qgscoordinatenumericformat.h"
#include "qgsxmlutils.h"

QgsNumericFormatRegistry::QgsNumericFormatRegistry()
{
  addFormat( new QgsFallbackNumericFormat() );
  addFormat( new QgsBasicNumericFormat() );
  addFormat( new QgsBearingNumericFormat() );
  addFormat( new QgsGeographicCoordinateNumericFormat() );
  addFormat( new QgsCurrencyNumericFormat() );
  addFormat( new QgsPercentageNumericFormat() );
  addFormat( new QgsScientificNumericFormat() );
  addFormat( new QgsFractionNumericFormat() );
}

QgsNumericFormatRegistry::~QgsNumericFormatRegistry()
{
  qDeleteAll( mFormats );
}

QStringList QgsNumericFormatRegistry::formats() const
{
  return mFormats.keys();
}

void QgsNumericFormatRegistry::addFormat( QgsNumericFormat *format )
{
  if ( !format )
    return;

  mFormats.insert( format->id(), format );
}

void QgsNumericFormatRegistry::removeFormat( const QString &id )
{
  if ( QgsNumericFormat *format = mFormats.take( id ) )
  {
    delete format;
  }
}

QgsNumericFormat *QgsNumericFormatRegistry::format( const QString &id ) const
{
  if ( mFormats.contains( id ) )
    return mFormats.value( id )->clone();

  return new QgsFallbackNumericFormat();
}

QgsNumericFormat *QgsNumericFormatRegistry::create( const QString &id, const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  if ( mFormats.contains( id ) )
    return mFormats.value( id )->create( configuration, context );

  return new QgsFallbackNumericFormat();
}

QgsNumericFormat *QgsNumericFormatRegistry::createFromXml( const QDomElement &element, const QgsReadWriteContext &context ) const
{
  const QVariantMap configuration = QgsXmlUtils::readVariant( element.firstChildElement() ).toMap();
  const QString id = element.attribute( QStringLiteral( "id" ) );

  if ( mFormats.contains( id ) )
    return mFormats.value( id )->create( configuration, context );

  return new QgsFallbackNumericFormat();
}

QgsNumericFormat *QgsNumericFormatRegistry::fallbackFormat() const
{
  return new QgsFallbackNumericFormat();
}

QString QgsNumericFormatRegistry::visibleName( const QString &id ) const
{
  if ( mFormats.contains( id ) )
    return mFormats.value( id )->visibleName();

  return QString();
}

int QgsNumericFormatRegistry::sortKey( const QString &id ) const
{
  if ( mFormats.contains( id ) )
    return mFormats.value( id )->sortKey();

  return 0;
}
