/***************************************************************************
    qgsrasterpipe.cpp - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <typeinfo>

#include "qgsrasterpipe.h"
#include "qgslogger.h"

#include <QByteArray>

QgsRasterPipe::QgsRasterPipe()
  : mProvider(0)
  , mRenderer(0)
  , mResampleFilter(0)
  , mProjector(0)
{
}

QgsRasterPipe::~QgsRasterPipe()
{
  foreach ( QgsRasterInterface* filter, mFilters )
  {
    delete filter;
  }
}

bool QgsRasterPipe::connectFilters ( QVector<QgsRasterInterface*> theFilters )
{
  QgsDebugMsg( "Entered" );
  for ( int i = 1; i < theFilters.size(); i++ ) 
  {
    if ( ! theFilters[i]->setInput ( theFilters[i-1] ) )
    { 
      QgsDebugMsg( QString ( "cannot connect %1 to %2" ).arg( typeid(*(theFilters[i])).name() ).arg ( typeid(*(theFilters[i-1])).name() ) );
      return false;
    }
  }
  return true;
}

bool QgsRasterPipe::insert ( int idx, QgsRasterInterface* theFilter )
{
  QgsDebugMsg( QString ( "insert %1 at %2" ).arg( typeid(*theFilter).name() ).arg ( idx ) );
  if ( idx > mFilters.size() )
  {
    idx = mFilters.size();
  }
  // make a copy of pipe to test connection, we test the connections
  // of the whole pipe, because the types and band numbers may change
  QVector<QgsRasterInterface*> filters = mFilters;

  filters.insert ( idx, theFilter );
  bool success = false;
  if ( connectFilters ( filters ) )
  {
    success = true;
    mFilters.insert ( idx, theFilter );
    setRole ( theFilter, idx );
    QgsDebugMsg( "inserted ok" );
  }

  // Connect or reconnect (after the test) filters
  connectFilters ( mFilters );
  return success; 
}

bool QgsRasterPipe::replace ( int idx, QgsRasterInterface* theFilter )
{
  QgsDebugMsg( QString ( "replace by %1 at %2" ).arg( typeid(*theFilter).name() ).arg ( idx ) );
  if ( idx < 0 || idx >= mFilters.size() )
  {
    return false;
  }
  if ( !theFilter ) return false;

  // make a copy of pipe to test connection, we test the connections
  // of the whole pipe, because the types and band numbers may change
  QVector<QgsRasterInterface*> filters = mFilters;

  filters[idx] = theFilter;
  bool success = false;
  if ( connectFilters ( filters ) )
  {
    success = true;
    delete mFilters[idx];
    mFilters[idx] = theFilter;
    setRole ( theFilter, idx );
    QgsDebugMsg( "replaced ok" );
  }

  // Connect or reconnect (after the test) filters
  connectFilters ( mFilters );
  return success; 
}

QgsRasterPipe::Role QgsRasterPipe::filterRole ( QgsRasterInterface * filter ) const
{
  if ( dynamic_cast<QgsRasterDataProvider *>( filter ) ) return ProviderRole;
  if ( dynamic_cast<QgsRasterRenderer *>( filter ) ) return RendererRole;
  if ( dynamic_cast<QgsRasterResampleFilter *>( filter ) ) return ResamplerRole;
  if ( dynamic_cast<QgsRasterProjector *>( filter ) ) return ProjectorRole;
  return UnknownRole;
}

void QgsRasterPipe::setRole ( QgsRasterInterface * theFilter, int idx )
{
  Role role = filterRole ( theFilter );
  if ( role == UnknownRole ) return;
  mRoleMap.insert ( role, idx );
}

bool QgsRasterPipe::setFilter( QgsRasterInterface* theFilter )
{
  QgsDebugMsg( QString ( "%1" ).arg( typeid(*theFilter).name() ) );

  if ( !theFilter ) return false;

  QgsRasterDataProvider * provider = dynamic_cast<QgsRasterDataProvider *>( theFilter );
  QgsRasterRenderer * renderer;
  QgsRasterResampleFilter * resampleFilter;
  QgsRasterProjector * projector;

  Role role = filterRole ( theFilter );

  // We dont know where to place unknown filter
  if ( role == UnknownRole ) return false;
 
  //if ( mFiltersMap.value ( role ) )
  if ( mRoleMap.contains ( role ) )
  {
    // An old filter of the same role exists -> replace
    // replace may still fail and return false
    return replace ( mRoleMap.value(role), theFilter ); 
  }

  int idx = 0; 

  // Not found, find the best default position for this kind of filter
  //   QgsRasterDataProvider  - ProviderRole
  //   QgsRasterRenderer      - RendererRole
  //   QgsRasterResampler     - ResamplerRole
  //   QgsRasterProjector     - ProjectorRole

  int providerIdx = mRoleMap.value( ProviderRole,-1);
  int rendererIdx = mRoleMap.value( RendererRole, -1 );
  int resamplerIdx = mRoleMap.value( ResamplerRole, -1 );

  if ( role == ProviderRole )
  {
    idx = 0;
  }
  else if ( role == RendererRole )
  {
    idx =  providerIdx + 1;
  }
  else if ( role == ResamplerRole )
  {
    idx =  qMax ( providerIdx, rendererIdx ) + 1;
  }
  else if ( role == ProjectorRole )
  {
    idx =  qMax ( qMax( providerIdx,rendererIdx ), resamplerIdx ) + 1;
  }

  return insert ( idx, theFilter ); // insert may still fail and return false
}

QgsRasterInterface * QgsRasterPipe::filter ( Role role ) const
{
  QgsDebugMsg( QString ( "role = %1" ).arg ( role ) );
  if ( mRoleMap.contains ( role ) )
  {
    return mFilters.value( mRoleMap.value( role)  );
  }
  return 0;
}

QgsRasterDataProvider * QgsRasterPipe::provider() const 
{ 
  return dynamic_cast<QgsRasterDataProvider *>(filter( ProviderRole)); 
}

QgsRasterRenderer * QgsRasterPipe::renderer() const 
{ 
  return dynamic_cast<QgsRasterRenderer *>(filter( RendererRole )); 
}

QgsRasterResampleFilter * QgsRasterPipe::resampleFilter() const 
{ 
  return dynamic_cast<QgsRasterResampleFilter *>(filter( ResamplerRole )); 
}   

QgsRasterProjector * QgsRasterPipe::projector() const 
{ 
  return dynamic_cast<QgsRasterProjector*>(filter ( ProjectorRole )); 
}

