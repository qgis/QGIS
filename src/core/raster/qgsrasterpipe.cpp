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
{
}

QgsRasterPipe::~QgsRasterPipe()
{
  foreach ( QgsRasterInterface* filter, mFilters )
  {
    //delete filter; // TODO enable
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

/*
bool QgsRasterPipe::addFilter ( QgsRasterInterface * theFilter )
{
  mFilters.append ( theFilter );
  if ( mFilters.size() < 2 ) { 
    return true; 
  }
  return theFilter->setInput ( mFilters[ mFilters.size()-2 ] );
}
*/
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
    QgsDebugMsg( "replaced ok" );
  }

  // Connect or reconnect (after the test) filters
  connectFilters ( mFilters );
  return success; 
}

bool QgsRasterPipe::insertOrReplace( QgsRasterInterface* theFilter )
{
  QgsDebugMsg( QString ( "%1" ).arg( typeid(*theFilter).name() ) );
  if ( !theFilter ) return false;
  int idx = indexOf ( theFilter->role() );
  if ( idx >= 0 )
  {
    return replace ( idx, theFilter ); // replace may still fail and return false
  }

  // Not found, find the best default position for this kind of filter
  // The default order is:
  //   QgsRasterDataProvider  - ProviderRole
  //   QgsRasterRenderer      - RendererRole
  //   QgsRasterResampler     - ResamplerRole
  //   QgsRasterProjector     - ProjectorRole
  int providerIdx = indexOf ( QgsRasterInterface::ProviderRole );
  int rendererIdx = indexOf ( QgsRasterInterface::RendererRole );
  int resamplerIdx = indexOf ( QgsRasterInterface::ResamplerRole );
  if ( theFilter->role() == QgsRasterInterface::ProviderRole )
  {
    idx = 0;
  }
  else if ( theFilter->role() == QgsRasterInterface::RendererRole )
  {
    idx =  providerIdx + 1;
  }
  else if ( theFilter->role() == QgsRasterInterface::ResamplerRole )
  {
    idx =  qMax ( providerIdx, rendererIdx ) + 1;
  }
  else if ( theFilter->role() == QgsRasterInterface::ProjectorRole )
  {
    idx =  qMax ( qMax( providerIdx,rendererIdx ), resamplerIdx ) + 1;
  }
  return insert ( idx, theFilter ); // insert may still fail and return false
}

int QgsRasterPipe::indexOf ( QgsRasterInterface::Role theRole ) const
{
  QgsDebugMsg( QString ( "role = %1" ).arg( theRole ) );
  //foreach ( QgsRasterInterface * filter, mFilters )
  
  for ( int i = 0; i < mFilters.size(); i++ )
  {
    if ( !mFilters[i] ) continue;

    if ( mFilters[i]->role() == theRole ) 
    {
      return i;
    }
    /*
    if ( typeid ( *theFilter ) == typeid ( *(mFilters[i]) ) )
    {
      
      QgsDebugMsg( QString ( "%1 found at %2" ).arg( typeid(*(mFilters[i])).name() ).arg(i) );
      return i;
    }

    // known ancestor
    if ( ( dynamic_cast<QgsRasterRenderer*>( theFilter ) && dynamic_cast<QgsRasterRenderer*>( mFilters[i] ) ) 
         || ( dynamic_cast<QgsRasterDataProvider*>( theFilter ) && dynamic_cast<QgsRasterDataProvider*>( mFilters[i] ) ) ) 
    {
      QgsDebugMsg( QString ( "%1 found at %2" ).arg( typeid(*(mFilters[i])).name() ).arg(i) );
      return i;
    }
    */
  }
  QgsDebugMsg( "role not found");
  return -1;
}
QgsRasterInterface * QgsRasterPipe::filter ( QgsRasterInterface::Role role ) const
{
  QgsDebugMsg( QString ( "role = %1" ).arg ( role ) );
  int idx = indexOf ( role );
  if ( idx >= 0 ) 
  {
    return mFilters[idx];
  }
  return 0;
}

