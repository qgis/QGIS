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

#include <mutex>

QgsRasterPipe::QgsRasterPipe( const QgsRasterPipe &pipe )
{
  for ( int i = 0; i < pipe.size(); i++ )
  {
    QgsRasterInterface *interface = pipe.at( i );
    QgsRasterInterface *clone = interface->clone();

    Qgis::RasterPipeInterfaceRole role = interfaceRole( clone );
    QgsDebugMsgLevel( QStringLiteral( "cloned interface with role %1" ).arg( qgsEnumValueToKey( role ) ), 4 );
    if ( i > 0 )
    {
      clone->setInput( mInterfaces.at( i - 1 ) );
    }
    mInterfaces.append( clone );
    if ( role != Qgis::RasterPipeInterfaceRole::Unknown )
    {
      mRoleMap.insert( role, i );
    }
  }
  setResamplingStage( pipe.resamplingStage() );
  mDataDefinedProperties = pipe.mDataDefinedProperties;
}

QgsRasterPipe::~QgsRasterPipe()
{
  const auto constMInterfaces = mInterfaces;
  for ( QgsRasterInterface *interface : constMInterfaces )
  {
    delete interface;
  }
}

bool QgsRasterPipe::connect( QVector<QgsRasterInterface *> interfaces )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  for ( int i = 1; i < interfaces.size(); i++ )
  {
    if ( ! interfaces[i]->setInput( interfaces[i - 1] ) )
    {
#ifdef QGISDEBUG
      const QgsRasterInterface &a = *interfaces[i];
      const QgsRasterInterface &b = *interfaces[i - 1];
      QgsDebugMsg( QStringLiteral( "cannot connect %1 to %2" ).arg( typeid( a ).name(), typeid( b ).name() ) );
#endif
      return false;
    }
  }
  return true;
}

bool QgsRasterPipe::insert( int idx, QgsRasterInterface *interface )
{
  QgsDebugMsgLevel( QStringLiteral( "insert %1 at %2" ).arg( typeid( *interface ).name() ).arg( idx ), 4 );
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
    QgsDebugMsgLevel( QStringLiteral( "Pipe %1 inserted OK" ).arg( idx ), 4 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Error inserting pipe %1" ).arg( idx ), 4 );
  }

  // Connect or reconnect (after the test) interfaces
  connect( mInterfaces );
  return success;
}

bool QgsRasterPipe::replace( int idx, QgsRasterInterface *interface )
{
  if ( !interface ) return false;

  QgsDebugMsgLevel( QStringLiteral( "replace by %1 at %2" ).arg( typeid( *interface ).name() ).arg( idx ), 4 );
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
    QgsDebugMsgLevel( QStringLiteral( "replaced OK" ), 4 );
  }

  // Connect or reconnect (after the test) interfaces
  connect( mInterfaces );
  return success;
}

Qgis::RasterPipeInterfaceRole QgsRasterPipe::interfaceRole( QgsRasterInterface *interface ) const
{
  Qgis::RasterPipeInterfaceRole role = Qgis::RasterPipeInterfaceRole::Unknown;
  if ( dynamic_cast<QgsRasterDataProvider *>( interface ) )
    role = Qgis::RasterPipeInterfaceRole::Provider;
  else if ( dynamic_cast<QgsRasterRenderer *>( interface ) )
    role = Qgis::RasterPipeInterfaceRole::Renderer;
  else if ( dynamic_cast<QgsRasterResampleFilter *>( interface ) )
    role = Qgis::RasterPipeInterfaceRole::Resampler;
  else if ( dynamic_cast<QgsBrightnessContrastFilter *>( interface ) )
    role = Qgis::RasterPipeInterfaceRole::Brightness;
  else if ( dynamic_cast<QgsHueSaturationFilter *>( interface ) )
    role = Qgis::RasterPipeInterfaceRole::HueSaturation;
  else if ( dynamic_cast<QgsRasterProjector *>( interface ) )
    role = Qgis::RasterPipeInterfaceRole::Projector;
  else if ( dynamic_cast<QgsRasterNuller *>( interface ) )
    role = Qgis::RasterPipeInterfaceRole::Nuller;

  QgsDebugMsgLevel( QStringLiteral( "%1 role = %2" ).arg( typeid( *interface ).name(), qgsEnumValueToKey( role ) ), 4 );
  return role;
}

void QgsRasterPipe::setRole( QgsRasterInterface *interface, int idx )
{
  Qgis::RasterPipeInterfaceRole role = interfaceRole( interface );
  if ( role == Qgis::RasterPipeInterfaceRole::Unknown )
    return;

  mRoleMap.insert( role, idx );
}

void QgsRasterPipe::unsetRole( QgsRasterInterface *interface )
{
  Qgis::RasterPipeInterfaceRole role = interfaceRole( interface );
  if ( role == Qgis::RasterPipeInterfaceRole::Unknown )
    return;

  const int roleIdx{ mRoleMap[role] };
  mRoleMap.remove( role );

  // Decrease all indexes greater than the removed one
  const auto roleMapValues {mRoleMap.values()};
  if ( roleIdx < *std::max_element( roleMapValues.begin(), roleMapValues.end() ) )
  {
    for ( auto it = mRoleMap.cbegin(); it != mRoleMap.cend(); ++it )
    {
      if ( it.value() > roleIdx )
      {
        mRoleMap[it.key()] = it.value() - 1;
      }
    }
  }
}

bool QgsRasterPipe::set( QgsRasterInterface *interface )
{
  if ( !interface )
    return false;

  QgsDebugMsgLevel( typeid( *interface ).name(), 4 );
  Qgis::RasterPipeInterfaceRole role = interfaceRole( interface );

  // We don't know where to place unknown interface
  if ( role == Qgis::RasterPipeInterfaceRole::Unknown )
    return false;

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

  int providerIdx = mRoleMap.value( Qgis::RasterPipeInterfaceRole::Provider, -1 );
  int rendererIdx = mRoleMap.value( Qgis::RasterPipeInterfaceRole::Renderer, -1 );
  int resamplerIdx = mRoleMap.value( Qgis::RasterPipeInterfaceRole::Resampler, -1 );
  int brightnessIdx = mRoleMap.value( Qgis::RasterPipeInterfaceRole::Brightness, -1 );
  int hueSaturationIdx = mRoleMap.value( Qgis::RasterPipeInterfaceRole::HueSaturation, -1 );

  if ( role == Qgis::RasterPipeInterfaceRole::Provider )
  {
    idx = 0;
  }
  else if ( role == Qgis::RasterPipeInterfaceRole::Renderer )
  {
    idx = providerIdx + 1;
  }
  else if ( role == Qgis::RasterPipeInterfaceRole::Brightness )
  {
    idx = std::max( providerIdx, rendererIdx ) + 1;
  }
  else if ( role == Qgis::RasterPipeInterfaceRole::HueSaturation )
  {
    idx = std::max( std::max( providerIdx, rendererIdx ), brightnessIdx ) + 1;
  }
  else if ( role == Qgis::RasterPipeInterfaceRole::Resampler )
  {
    idx = std::max( std::max( std::max( providerIdx, rendererIdx ), brightnessIdx ), hueSaturationIdx ) + 1;
  }
  else if ( role == Qgis::RasterPipeInterfaceRole::Projector )
  {
    idx = std::max( std::max( std::max( std::max( providerIdx, rendererIdx ), brightnessIdx ), hueSaturationIdx ), resamplerIdx )  + 1;
  }

  return insert( idx, interface );  // insert may still fail and return false
}

QgsRasterInterface *QgsRasterPipe::interface( Qgis::RasterPipeInterfaceRole role ) const
{
  QgsDebugMsgLevel( QStringLiteral( "role = %1" ).arg( qgsEnumValueToKey( role ) ), 4 );
  if ( mRoleMap.contains( role ) )
  {
    return mInterfaces.value( mRoleMap.value( role ) );
  }
  return nullptr;
}

QgsRasterDataProvider *QgsRasterPipe::provider() const
{
  return dynamic_cast<QgsRasterDataProvider *>( interface( Qgis::RasterPipeInterfaceRole::Provider ) );
}

QgsRasterRenderer *QgsRasterPipe::renderer() const
{
  return dynamic_cast<QgsRasterRenderer *>( interface( Qgis::RasterPipeInterfaceRole::Renderer ) );
}

QgsRasterResampleFilter *QgsRasterPipe::resampleFilter() const
{
  return dynamic_cast<QgsRasterResampleFilter *>( interface( Qgis::RasterPipeInterfaceRole::Resampler ) );
}

QgsBrightnessContrastFilter *QgsRasterPipe::brightnessFilter() const
{
  return dynamic_cast<QgsBrightnessContrastFilter *>( interface( Qgis::RasterPipeInterfaceRole::Brightness ) );
}

QgsHueSaturationFilter *QgsRasterPipe::hueSaturationFilter() const
{
  return dynamic_cast<QgsHueSaturationFilter *>( interface( Qgis::RasterPipeInterfaceRole::HueSaturation ) );
}

QgsRasterProjector *QgsRasterPipe::projector() const
{
  return dynamic_cast<QgsRasterProjector *>( interface( Qgis::RasterPipeInterfaceRole::Projector ) );
}

QgsRasterNuller *QgsRasterPipe::nuller() const
{
  return dynamic_cast<QgsRasterNuller *>( interface( Qgis::RasterPipeInterfaceRole::Nuller ) );
}

bool QgsRasterPipe::remove( int idx )
{
  QgsDebugMsgLevel( QStringLiteral( "remove at %1" ).arg( idx ), 4 );

  if ( !checkBounds( idx ) )
    return false;

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
    QgsDebugMsgLevel( QStringLiteral( "Pipe %1 removed OK" ).arg( idx ), 4 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Error removing pipe %1" ).arg( idx ), 4 );
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
  QgsDebugMsgLevel( QStringLiteral( "idx = %1 on = %2" ).arg( idx ).arg( on ), 4 );
  if ( !checkBounds( idx ) )
    return false;

  // Because setting interface on/off may change its output we must check if
  // connection is OK after such switch
  bool onOrig = mInterfaces.at( idx )->on();

  if ( onOrig == on )
    return true;

  mInterfaces.at( idx )->setOn( on );

  bool success = connect( mInterfaces );

  mInterfaces.at( idx )->setOn( onOrig );
  connect( mInterfaces );
  return success;
}

bool QgsRasterPipe::setOn( int idx, bool on )
{
  QgsDebugMsgLevel( QStringLiteral( "idx = %1 on = %2" ).arg( idx ).arg( on ), 4 );
  if ( !checkBounds( idx ) )
    return false;

  bool onOrig = mInterfaces.at( idx )->on();

  if ( onOrig == on )
    return true;

  mInterfaces.at( idx )->setOn( on );

  if ( connect( mInterfaces ) )
    return true;

  mInterfaces.at( idx )->setOn( onOrig );
  connect( mInterfaces );

  return false;
}

bool QgsRasterPipe::checkBounds( int idx ) const
{
  return !( idx < 0 || idx >= mInterfaces.size() );
}

void QgsRasterPipe::setResamplingStage( Qgis::RasterResamplingStage stage )
{
  mResamplingStage = stage;

  int resamplerIndex = 0;
  for ( QgsRasterInterface *interface : std::as_const( mInterfaces ) )
  {
    if ( interfaceRole( interface ) == Qgis::RasterPipeInterfaceRole::Resampler )
    {
      setOn( resamplerIndex, stage == Qgis::RasterResamplingStage::ResampleFilter );
      break;
    }
    resamplerIndex ++;
  }

  if ( QgsRasterDataProvider *l_provider = provider() )
  {
    l_provider->enableProviderResampling( stage == Qgis::RasterResamplingStage::Provider );
  }
}

void QgsRasterPipe::evaluateDataDefinedProperties( QgsExpressionContext &context )
{
  if ( !mDataDefinedProperties.hasActiveProperties() )
    return;

  if ( mDataDefinedProperties.isActive( RendererOpacity ) )
  {
    if ( QgsRasterRenderer *r = renderer() )
    {
      const double prevOpacity = r->opacity();
      context.setOriginalValueVariable( prevOpacity * 100 );
      bool ok = false;
      const double opacity = mDataDefinedProperties.valueAsDouble( RendererOpacity, context, prevOpacity, &ok ) / 100;
      if ( ok )
      {
        r->setOpacity( opacity );
      }
    }
  }
}

QgsPropertiesDefinition QgsRasterPipe::sPropertyDefinitions;

void QgsRasterPipe::initPropertyDefinitions()
{
  const QString origin = QStringLiteral( "raster" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsRasterPipe::RendererOpacity, QgsPropertyDefinition( "RendererOpacity", QObject::tr( "Renderer opacity" ), QgsPropertyDefinition::Opacity, origin ) },
  };
}

QgsPropertiesDefinition QgsRasterPipe::propertyDefinitions()
{
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    initPropertyDefinitions();
  } );
  return sPropertyDefinitions;
}
