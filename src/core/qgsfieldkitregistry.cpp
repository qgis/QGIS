/***************************************************************************
  qgsfieldkitregistry.cpp - QgsFieldKitRegistry

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
#include "qgsfieldkitregistry.h"
#include "qgsfieldkit.h"

#include "qgsvaluerelationfieldkit.h"
#include "qgsvaluemapfieldkit.h"
#include "qgsdatetimefieldkit.h"
#include "qgsrelationreferencefieldkit.h"
#include "qgskeyvaluefieldkit.h"
#include "qgslistfieldkit.h"
#include "qgsfallbackfieldkit.h"


QgsFieldKitRegistry::QgsFieldKitRegistry( QObject* parent )
    : QObject( parent )
{
  addFieldKit( new QgsValueRelationFieldKit() );
  addFieldKit( new QgsValueMapFieldKit() );
  addFieldKit( new QgsRelationReferenceFieldKit() );
  addFieldKit( new QgsKeyValueFieldKit() );
  addFieldKit( new QgsListFieldKit() );
  addFieldKit( new QgsDateTimeFieldKit() );

  mFallbackFieldKit = new QgsFallbackFieldKit();
}

QgsFieldKitRegistry::~QgsFieldKitRegistry()
{
  qDeleteAll( mFieldKits );
  delete mFallbackFieldKit;
}

void QgsFieldKitRegistry::addFieldKit( QgsFieldKit* kit )
{
  mFieldKits.insert( kit->id(), kit );
  emit fieldKitAdded( kit );
}

void QgsFieldKitRegistry::removeFieldKit( QgsFieldKit* kit )
{
  if ( mFieldKits.remove( kit->id() ) )
  {
    emit fieldKitRemoved( kit );
    delete kit;
  }
}

QgsFieldKit* QgsFieldKitRegistry::fieldKit( const QString& id ) const
{
  return mFieldKits.value( id, mFallbackFieldKit );
}
