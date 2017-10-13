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

#include <QByteArray>

#include "qgslogger.h"
#include "qgsrasterpipe.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterresamplefilter.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgshuesaturationfilter.h"
#include "qgsrasterprojector.h"
#include "qgsrasternuller.h"

QgsRasterPipe::QgsRasterPipe( const QgsRasterPipe &pipe )
{
  for ( int i = 0; i < pipe.size(); i++ )
  {
    QgsRasterInterface *interface = pipe.at( i );
    QgsRasterInterface *clone = interface->clone();

    Role role = interfaceRole( clone );
    QgsDebugMsgLevel( QString( "cloned interface with role %1" ).arg( role ), 4 );
    if ( i > 0 )
    {
      clone->setInput( mInterfaces.at( i - 1 ) );
    }
    mInterfaces.append( clone );
    if ( role != UnknownRole )
    {
      mRoleMap.insert( role, i );
    }
  }
}

QgsRasterPipe::~QgsRasterPipe()
{
  Q_FOREACH ( QgsRasterInterface *interface, mInterfaces )
  {
    delete interface;
  }
}

bool QgsRasterPipe::connect( QVector<QgsRasterInterface *> interfaces )
{
  QgsDebugMsgLevel( "Entered", 4 );
  for ( int i = 1; i < interfaces.size(); i++ )
  {
    if ( ! interfaces[i]->setInput( interfaces[i - 1] ) )
    {
#ifdef QGISDEBUG
      const QgsRasterInterface &a = *interfaces[i];
      const QgsRasterInterface &b = *interfaces[i - 1];
      QgsDebugMsg( QString( "cannot connect %1 to %2" ).arg( typeid( a ).name(), typeid( b ).name() ) );
#endif
      return false;
    }
  }
  return true;
}

bool QgsRasterPipe::insert( int idx, QgsRasterInterface *interface )
{
  QgsDebugMsgLevel( QString( "insert %1 at %2" ).arg( typeid( *interface ).name() ).arg( idx ), 4 );
  if ( idx > mInterfaces.size() )
  {
    idx = mInterfaces.size();
  }
  // make a copy of pipe to test connection, we test the connections
  // of the whole pipe, because the types and band numbers may change
  QVector<QgsRasterInterface *> interfaces = mInterfaces;

  interfaces.insert( idx, interface );
  bool success = false;
  if ( connect( interfaces ) )
  {
    success = true;
    mInterfaces.insert( idx, interface );
    setRole( interface, idx );
    QgsDebugMsgLevel( "inserted OK", 4 );
  }

  // Connect or reconnect (after the test) interfaces
  connect( mInterfaces );
  return success;
}

bool QgsRasterPipe::replace( int idx, QgsRasterInterface *interface )
{
  if ( !interface ) return false;

  QgsDebugMsgLevel( QString( "replace by %1 at %2" ).arg( typeid( *interface ).name() ).arg( idx ), 4 );
  if ( !checkBounds( idx ) ) return false;

  // make a copy of pipe to test connection, we test the connections
  // of the whole pipe, because the types and band numbers may change
  QVector<QgsRasterInterface *> interfaces = mInterfaces;

  interfaces[idx] = interface;
  bool success = false;
  if ( connect( interfaces ) )
  {
    success = true;
    delete mInterfaces.at( idx );
    mInterfaces[idx] = interface;
    setRole( interface, idx );
    QgsDebugMsgLevel( "replaced OK", 4 );
  }

  // Connect or reconnect (after the test) interfaces
  connect( mInterfaces );
  return success;
}

QgsRasterPipe::Role QgsRasterPipe::interfaceRole( QgsRasterInterface *interface ) const
{
  Role role = UnknownRole;
  if ( dynamic_cast<QgsRasterDataProvider *>( interface ) ) role = ProviderRole;
  else if ( dynamic_cast<QgsRasterRenderer *>( interface ) ) role = RendererRole;
  else if ( dynamic_cast<QgsRasterResampleFilter *>( interface ) ) role = ResamplerRole;
  else if ( dynamic_cast<QgsBrightnessContrastFilter *>( interface ) ) role = BrightnessRole;
  else if ( dynamic_cast<QgsHueSaturationFilter *>( interface ) ) role = HueSaturationRole;
  else if ( dynamic_cast<QgsRasterProjector *>( interface ) ) role = ProjectorRole;
  else if ( dynamic_cast<QgsRasterNuller *>( interface ) ) role = NullerRole;

  QgsDebugMsgLevel( QString( "%1 role = %2" ).arg( typeid( *interface ).name() ).arg( role ), 4 );
  return role;
}

void QgsRasterPipe::setRole( QgsRasterInterface *interface, int idx )
{
  Role role = interfaceRole( interface );
  if ( role == UnknownRole ) return;
  mRoleMap.insert( role, idx );
}

void QgsRasterPipe::unsetRole( QgsRasterInterface *interface )
{
  Role role = interfaceRole( interface );
  if ( role == UnknownRole ) return;
  mRoleMap.remove( role );
}

bool QgsRasterPipe::set( QgsRasterInterface *interface )
{
  if ( !interface ) return false;

  QgsDebugMsgLevel( QString( "%1" ).arg( typeid( *interface ).name() ), 4 );
  Role role = interfaceRole( interface );

  // We don't know where to place unknown interface
  if ( role == UnknownRole ) return false;

  //if ( mInterfacesMap.value ( role ) )
  if ( mRoleMap.contains( role ) )
  {
    // An old interface of the same role exists -> replace
    // replace may still fail and return false
    return replace( mRoleMap.value( role ), interface );
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
  int brightnessIdx = mRoleMap.value( BrightnessRole, -1 );
  int hueSaturationIdx = mRoleMap.value( HueSaturationRole, -1 );

  if ( role == ProviderRole )
  {
    idx = 0;
  }
  else if ( role == RendererRole )
  {
    idx = providerIdx + 1;
  }
  else if ( role == BrightnessRole )
  {
    idx = std::max( providerIdx, rendererIdx ) + 1;
  }
  else if ( role == HueSaturationRole )
  {
    idx = std::max( std::max( providerIdx, rendererIdx ), brightnessIdx ) + 1;
  }
  else if ( role == ResamplerRole )
  {
    idx = std::max( std::max( std::max( providerIdx, rendererIdx ), brightnessIdx ), hueSaturationIdx ) + 1;
  }
  else if ( role == ProjectorRole )
  {
    idx = std::max( std::max( std::max( std::max( providerIdx, rendererIdx ), brightnessIdx ), hueSaturationIdx ), resamplerIdx )  + 1;
  }

  return insert( idx, interface );  // insert may still fail and return false
}

QgsRasterInterface *QgsRasterPipe::interface( Role role ) const
{
  QgsDebugMsgLevel( QString( "role = %1" ).arg( role ), 4 );
  if ( mRoleMap.contains( role ) )
  {
    return mInterfaces.value( mRoleMap.value( role ) );
  }
  return nullptr;
}

QgsRasterDataProvider *QgsRasterPipe::provider() const
{
  return dynamic_cast<QgsRasterDataProvider *>( interface( ProviderRole ) );
}

QgsRasterRenderer *QgsRasterPipe::renderer() const
{
  return dynamic_cast<QgsRasterRenderer *>( interface( RendererRole ) );
}

QgsRasterResampleFilter *QgsRasterPipe::resampleFilter() const
{
  return dynamic_cast<QgsRasterResampleFilter *>( interface( ResamplerRole ) );
}

QgsBrightnessContrastFilter *QgsRasterPipe::brightnessFilter() const
{
  return dynamic_cast<QgsBrightnessContrastFilter *>( interface( BrightnessRole ) );
}

QgsHueSaturationFilter *QgsRasterPipe::hueSaturationFilter() const
{
  return dynamic_cast<QgsHueSaturationFilter *>( interface( HueSaturationRole ) );
}

QgsRasterProjector *QgsRasterPipe::projector() const
{
  return dynamic_cast<QgsRasterProjector *>( interface( ProjectorRole ) );
}

QgsRasterNuller *QgsRasterPipe::nuller() const
{
  return dynamic_cast<QgsRasterNuller *>( interface( NullerRole ) );
}

bool QgsRasterPipe::remove( int idx )
{
  QgsDebugMsgLevel( QString( "remove at %1" ).arg( idx ), 4 );

  if ( !checkBounds( idx ) ) return false;

  // make a copy of pipe to test connection, we test the connections
  // of the whole pipe, because the types and band numbers may change
  QVector<QgsRasterInterface *> interfaces = mInterfaces;

  interfaces.remove( idx );
  bool success = false;
  if ( connect( interfaces ) )
  {
    success = true;
    unsetRole( mInterfaces.at( idx ) );
    delete mInterfaces.at( idx );
    mInterfaces.remove( idx );
    QgsDebugMsgLevel( "removed OK", 4 );
  }

  // Connect or reconnect (after the test) interfaces
  connect( mInterfaces );
  return success;
}

bool QgsRasterPipe::remove( QgsRasterInterface *interface )
{
  if ( !interface ) return false;

  return remove( mInterfaces.indexOf( interface ) );
}

bool QgsRasterPipe::canSetOn( int idx, bool on )
{
  QgsDebugMsgLevel( QString( "idx = %1 on = %2" ).arg( idx ).arg( on ), 4 );
  if ( !checkBounds( idx ) ) return false;

  // Because setting interface on/off may change its output we must check if
  // connection is OK after such switch
  bool onOrig = mInterfaces.at( idx )->on();

  if ( onOrig == on ) return true;

  mInterfaces.at( idx )->setOn( on );

  bool success = connect( mInterfaces );

  mInterfaces.at( idx )->setOn( onOrig );
  connect( mInterfaces );
  return success;
}

bool QgsRasterPipe::setOn( int idx, bool on )
{
  QgsDebugMsgLevel( QString( "idx = %1 on = %2" ).arg( idx ).arg( on ), 4 );
  if ( !checkBounds( idx ) ) return false;

  bool onOrig = mInterfaces.at( idx )->on();

  if ( onOrig == on ) return true;

  mInterfaces.at( idx )->setOn( on );

  if ( connect( mInterfaces ) ) return true;

  mInterfaces.at( idx )->setOn( onOrig );
  connect( mInterfaces );

  return false;
}

bool QgsRasterPipe::checkBounds( int idx ) const
{
  return !( idx < 0 || idx >= mInterfaces.size() );
}
