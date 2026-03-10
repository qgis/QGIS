/***************************************************************************
                             qgssymbolconverterregistry.cpp
                             -----------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbolconverterregistry.h"

#include "qgssymbolconverter.h"
#include "qgssymbolconverteresrirest.h"
#include "qgssymbolconvertermapboxgl.h"
#include "qgssymbolconverterogrstyle.h"
#include "qgssymbolconverterqml.h"
#include "qgssymbolconvertersld.h"

#include "moc_qgssymbolconverterregistry.cpp"

QgsSymbolConverterRegistry::QgsSymbolConverterRegistry( QObject *parent )
  : QObject( parent )
{}

QgsSymbolConverterRegistry::~QgsSymbolConverterRegistry()
{
  qDeleteAll( mConverters );
}

void QgsSymbolConverterRegistry::populate()
{
  addConverter( new QgsSymbolConverterQml() );
  addConverter( new QgsSymbolConverterSld() );
  addConverter( new QgsSymbolConverterMapBoxGl() );
  addConverter( new QgsSymbolConverterOgrStyle() );
  addConverter( new QgsSymbolConverterEsriRest() );
}

bool QgsSymbolConverterRegistry::addConverter( QgsAbstractSymbolConverter *converter )
{
  if ( !converter )
    return false;

  if ( mConverters.contains( converter->name() ) )
    return false;

  mConverters.insert( converter->name(), converter );
  return true;
}

QgsAbstractSymbolConverter *QgsSymbolConverterRegistry::converter( const QString &name ) const
{
  return mConverters.value( name, nullptr );
}

bool QgsSymbolConverterRegistry::removeConverter( const QString &name )
{
  if ( !mConverters.contains( name ) )
    return false;

  delete mConverters.take( name );
  return true;
}

QStringList QgsSymbolConverterRegistry::converterNames() const
{
  return mConverters.keys();
}
