/***************************************************************************
  qgsfieldformatterregistry.cpp - QgsFieldFormatterRegistry

 ---------------------
 begin                : 2.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"

#include "qgsvaluerelationfieldformatter.h"
#include "qgsvaluemapfieldformatter.h"
#include "qgsdatetimefieldformatter.h"
#include "qgsrelationreferencefieldformatter.h"
#include "qgskeyvaluefieldformatter.h"
#include "qgslistfieldformatter.h"
#include "qgsfallbackfieldformatter.h"


QgsFieldFormatterRegistry::QgsFieldFormatterRegistry( QObject* parent )
    : QObject( parent )
{
  addFieldFormatter( new QgsValueRelationFieldFormatter() );
  addFieldFormatter( new QgsValueMapFieldFormatter() );
  addFieldFormatter( new QgsRelationReferenceFieldFormatter() );
  addFieldFormatter( new QgsKeyValueFieldFormatter() );
  addFieldFormatter( new QgsListFieldFormatter() );
  addFieldFormatter( new QgsDateTimeFieldFormatter() );

  mFallbackFieldFormatter = new QgsFallbackFieldFormatter();
}

QgsFieldFormatterRegistry::~QgsFieldFormatterRegistry()
{
  qDeleteAll( mFieldFormatters );
  delete mFallbackFieldFormatter;
}

void QgsFieldFormatterRegistry::addFieldFormatter( QgsFieldFormatter* kit )
{
  mFieldFormatters.insert( kit->id(), kit );
  emit fieldKitAdded( kit );
}

void QgsFieldFormatterRegistry::removeFieldFormatter( QgsFieldFormatter* kit )
{
  if ( mFieldFormatters.remove( kit->id() ) )
  {
    emit fieldKitRemoved( kit );
    delete kit;
  }
}

QgsFieldFormatter* QgsFieldFormatterRegistry::fieldKit( const QString& id ) const
{
  return mFieldFormatters.value( id, mFallbackFieldFormatter );
}
