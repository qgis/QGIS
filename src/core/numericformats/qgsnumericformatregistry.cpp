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


QgsNumericFormatRegistry::QgsNumericFormatRegistry()
{
  addFormat( new QgsBasicNumericFormat() );
  addFormat( new QgsBearingNumericFormat() );
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

QgsNumericFormat *QgsNumericFormatRegistry::create( const QString &id, const QVariantMap &configuration ) const
{
  if ( mFormats.contains( id ) )
    return mFormats.value( id )->create( configuration );

  return new QgsFallbackNumericFormat();
}

QgsNumericFormat *QgsNumericFormatRegistry::fallbackFormat() const
{
  return new QgsFallbackNumericFormat();
}
