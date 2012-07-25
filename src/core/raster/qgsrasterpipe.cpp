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
  foreach( QgsRasterInterface* interface, mInterfaces )
  {
    delete interface;
  }
}

bool QgsRasterPipe::connect( QVector<QgsRasterInterface*> theInterfaces )
{
  QgsDebugMsg( "Entered" );
  for ( int i = 1; i < theInterfaces.size(); i++ )
  {
    if ( ! theInterfaces[i]->setInput( theInterfaces[i-1] ) )
    {
      QgsDebugMsg( QString( "cannot connect %1 to %2" ).arg( typeid( *( theInterfaces[i] ) ).name() ).arg( typeid( *( theInterfaces[i-1] ) ).name() ) );
      return false;
    }
  }
  return true;
}

bool QgsRasterPipe::insert( int idx, QgsRasterInterface* theInterface )
{
  QgsDebugMsg( QString( "insert %1 at %2" ).arg( typeid( *theInterface ).name() ).arg( idx ) );
  if ( idx > mInterfaces.size() )
  {
    idx = mInterfaces.size();
  }
  // make a copy of pipe to test connection, we test the connections
  // of the whole pipe, because the types and band numbers may change
  QVector<QgsRasterInterface*> interfaces = mInterfaces;

  interfaces.insert( idx, theInterface );
  bool success = false;
  if ( connect( interfaces ) )
  {
    success = true;
    mInterfaces.insert( idx, theInterface );
    setRole( theInterface, idx );
    QgsDebugMsg( "inserted ok" );
  }

  // Connect or reconnect (after the test) interfaces
  connect( mInterfaces );
  return success;
}

bool QgsRasterPipe::replace( int idx, QgsRasterInterface* theInterface )
{
  QgsDebugMsg( QString( "replace by %1 at %2" ).arg( typeid( *theInterface ).name() ).arg( idx ) );
  if ( !checkBounds( idx ) ) return false;
  if ( !theInterface ) return false;

  // make a copy of pipe to test connection, we test the connections
  // of the whole pipe, because the types and band numbers may change
  QVector<QgsRasterInterface*> interfaces = mInterfaces;

  interfaces[idx] = theInterface;
  bool success = false;
  if ( connect( interfaces ) )
  {
    success = true;
    delete mInterfaces[idx];
    mInterfaces[idx] = theInterface;
    setRole( theInterface, idx );
    QgsDebugMsg( "replaced ok" );
  }

  // Connect or reconnect (after the test) interfaces
  connect( mInterfaces );
  return success;
}

QgsRasterPipe::Role QgsRasterPipe::interfaceRole( QgsRasterInterface * interface ) const
{
  if ( dynamic_cast<QgsRasterDataProvider *>( interface ) ) return ProviderRole;
  if ( dynamic_cast<QgsRasterRenderer *>( interface ) ) return RendererRole;
  if ( dynamic_cast<QgsRasterResampleFilter *>( interface ) ) return ResamplerRole;
  if ( dynamic_cast<QgsRasterProjector *>( interface ) ) return ProjectorRole;
  return UnknownRole;
}

void QgsRasterPipe::setRole( QgsRasterInterface * theInterface, int idx )
{
  Role role = interfaceRole( theInterface );
  if ( role == UnknownRole ) return;
  mRoleMap.insert( role, idx );
}

void QgsRasterPipe::unsetRole( QgsRasterInterface * theInterface )
{
  Role role = interfaceRole( theInterface );
  if ( role == UnknownRole ) return;
  mRoleMap.remove( role );
}

bool QgsRasterPipe::set( QgsRasterInterface* theInterface )
{
  QgsDebugMsg( QString( "%1" ).arg( typeid( *theInterface ).name() ) );

  if ( !theInterface ) return false;

  Role role = interfaceRole( theInterface );

  // We dont know where to place unknown interface
  if ( role == UnknownRole ) return false;

  //if ( mInterfacesMap.value ( role ) )
  if ( mRoleMap.contains( role ) )
  {
    // An old interface of the same role exists -> replace
    // replace may still fail and return false
    return replace( mRoleMap.value( role ), theInterface );
  }

  int idx = 0;

  // Not found, find the best default position for this kind of interface
  //   QgsRasterDataProvider  - ProviderRole
  //   QgsRasterRenderer      - RendererRole
  //   QgsRasterResampler     - ResamplerRole
  //   QgsRasterProjector     - ProjectorRole

  int providerIdx = mRoleMap.value( ProviderRole, -1 );
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
    idx =  qMax( providerIdx, rendererIdx ) + 1;
  }
  else if ( role == ProjectorRole )
  {
    idx =  qMax( qMax( providerIdx, rendererIdx ), resamplerIdx ) + 1;
  }

  return insert( idx, theInterface );  // insert may still fail and return false
}

QgsRasterInterface * QgsRasterPipe::iface( Role role ) const
{
  QgsDebugMsg( QString( "role = %1" ).arg( role ) );
  if ( mRoleMap.contains( role ) )
  {
    return mInterfaces.value( mRoleMap.value( role ) );
  }
  return 0;
}

QgsRasterDataProvider * QgsRasterPipe::provider() const
{
  return dynamic_cast<QgsRasterDataProvider *>( iface( ProviderRole ) );
}

QgsRasterRenderer * QgsRasterPipe::renderer() const
{
  return dynamic_cast<QgsRasterRenderer *>( iface( RendererRole ) );
}

QgsRasterResampleFilter * QgsRasterPipe::resampleFilter() const
{
  return dynamic_cast<QgsRasterResampleFilter *>( iface( ResamplerRole ) );
}

QgsRasterProjector * QgsRasterPipe::projector() const
{
  return dynamic_cast<QgsRasterProjector*>( iface( ProjectorRole ) );
}

bool QgsRasterPipe::remove( int idx )
{
  QgsDebugMsg( QString( "remove at %1" ).arg( idx ) );

  if ( !checkBounds( idx ) ) return false;

  // make a copy of pipe to test connection, we test the connections
  // of the whole pipe, because the types and band numbers may change
  QVector<QgsRasterInterface*> interfaces = mInterfaces;

  interfaces.remove( idx );
  bool success = false;
  if ( connect( interfaces ) )
  {
    success = true;
    unsetRole( mInterfaces[idx] );
    delete mInterfaces[idx];
    mInterfaces.remove( idx );
    QgsDebugMsg( "removed ok" );
  }

  // Connect or reconnect (after the test) interfaces
  connect( mInterfaces );
  return success;
}

bool QgsRasterPipe::remove( QgsRasterInterface * theInterface )
{
  if ( !theInterface ) return false;

  return remove( mInterfaces.indexOf( theInterface ) );
}

bool QgsRasterPipe::canSetOn( int idx, bool on )
{
  QgsDebugMsg( QString( "idx = %1 on = %2" ).arg( idx ).arg( on ) );
  if ( !checkBounds( idx ) ) return false;

  // Because setting interface on/off may change its output we must check if
  // connection is OK after such switch
  bool onOrig =  mInterfaces[idx]->on();

  if ( onOrig == on ) return true;

  mInterfaces[idx]->setOn( on );

  bool success = connect( mInterfaces );

  mInterfaces[idx]->setOn( onOrig );
  connect( mInterfaces );
  return success;
}

bool QgsRasterPipe::setOn( int idx, bool on )
{
  QgsDebugMsg( QString( "idx = %1 on = %2" ).arg( idx ).arg( on ) );
  if ( !checkBounds( idx ) ) return false;

  bool onOrig =  mInterfaces[idx]->on();

  if ( onOrig == on ) return true;

  mInterfaces[idx]->setOn( on );

  if ( connect( mInterfaces ) ) return true;

  mInterfaces[idx]->setOn( onOrig );
  connect( mInterfaces );

  return false;
}

bool QgsRasterPipe::checkBounds( int idx ) const
{
  if ( idx < 0 || idx >= mInterfaces.size() ) return false;
  return true;
}
