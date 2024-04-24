/***************************************************************************
  qgsprofilesourceregistry.cpp
  --------------------------------------
  Date                 : April 2024
  Copyright            : (C) 2024 by Germán Carrillo
  Email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprofilesourceregistry.h"

#include "qgsabstractprofilesource.h"

QgsProfileSourceRegistry::QgsProfileSourceRegistry()
{

}

QgsProfileSourceRegistry::~QgsProfileSourceRegistry()
{
  qDeleteAll( mSources );
}

QList< QgsAbstractProfileSource * > QgsProfileSourceRegistry::profileSources() const
{
  return mSources;
}

void QgsProfileSourceRegistry::registerProfileSource( QgsAbstractProfileSource *profileSource )
{
  if ( !mSources.contains( profileSource ) )
    mSources.append( profileSource );
}

void QgsProfileSourceRegistry::unregisterProfileSource( QgsAbstractProfileSource *profileSource )
{
  const int index = mSources.indexOf( profileSource );
  if ( index >= 0 )
    delete mSources.takeAt( index );
}
