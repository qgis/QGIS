/***************************************************************************
  qgsdataitemguiproviderregistry.cpp
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemguiproviderregistry.h"
#include "qgsdataitemguiprovider.h"

QgsDataItemGuiProviderRegistry::QgsDataItemGuiProviderRegistry()
{
}

QgsDataItemGuiProviderRegistry::~QgsDataItemGuiProviderRegistry()
{
  qDeleteAll( mProviders );
}

void QgsDataItemGuiProviderRegistry::addProvider( QgsDataItemGuiProvider *provider )
{
  mProviders.append( provider );
}

void QgsDataItemGuiProviderRegistry::removeProvider( QgsDataItemGuiProvider *provider )
{
  int index = mProviders.indexOf( provider );
  if ( index >= 0 )
    delete mProviders.takeAt( index );
}
