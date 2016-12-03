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

QgsFieldKitRegistry::QgsFieldKitRegistry()
{

}

QgsFieldKitRegistry::~QgsFieldKitRegistry()
{
  qDeleteAll( mFieldKits );
}

void QgsFieldKitRegistry::addFieldKit( QgsFieldKit* kit )
{
  mFieldKits.prepend( kit );
}

void QgsFieldKitRegistry::removeFieldKit( QgsFieldKit* kit )
{
  mFieldKits.removeOne( kit );
  delete kit;
}
