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
#include "qgslogger.h"

#include "moc_qgsprofilesourceregistry.cpp"

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

bool QgsProfileSourceRegistry::registerProfileSource( QgsAbstractProfileSource *profileSource )
{

  if ( mSources.contains( profileSource ) )
  {
    return false;
  }

  for ( const auto &source : std::as_const( mSources ) )
  {
    if ( source->profileSourceId() == profileSource->profileSourceId() )
    {
      QgsDebugError( u"A profile source with the same ID (%1) already exists"_s.arg( profileSource->profileSourceId() ) );
      return false;
    }
  }

  mSources.append( profileSource );
  emit profileSourceRegistered( profileSource->profileSourceId(), profileSource->profileSourceName() );
  return true;
}

bool QgsProfileSourceRegistry::unregisterProfileSource( QgsAbstractProfileSource *profileSource )
{
  const int index = mSources.indexOf( profileSource );
  if ( index >= 0 )
  {
    const QString id = profileSource->profileSourceId();
    delete mSources.takeAt( index );
    emit profileSourceUnregistered( id );
    return true;
  }
  return false;
}

bool QgsProfileSourceRegistry::unregisterProfileSource( const QString &sourceId )
{
  for ( auto it = mSources.begin(); it != mSources.end(); ++it )
  {
    if ( ( *it )->profileSourceId() == sourceId )
    {
      const int index = mSources.indexOf( *it );
      delete mSources.takeAt( index );
      emit profileSourceUnregistered( sourceId );
      return true;
    }
  }
  return false;
}

QgsAbstractProfileSource *QgsProfileSourceRegistry::findSourceById( const QString &sourceId ) const
{
  for ( QgsAbstractProfileSource *source : mSources )
  {
    if ( source->profileSourceId() == sourceId )
      return source;
  }

  return nullptr;
}
