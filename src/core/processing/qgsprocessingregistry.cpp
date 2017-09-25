/***************************************************************************
                         qgsprocessingregistry.cpp
                         --------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingregistry.h"
#include "qgsvectorfilewriter.h"

QgsProcessingRegistry::QgsProcessingRegistry( QObject *parent SIP_TRANSFERTHIS )
  : QObject( parent )
{}

QgsProcessingRegistry::~QgsProcessingRegistry()
{
  Q_FOREACH ( QgsProcessingProvider *p, mProviders )
  {
    removeProvider( p );
  }
}

bool QgsProcessingRegistry::addProvider( QgsProcessingProvider *provider )
{
  if ( !provider )
    return false;

  if ( mProviders.contains( provider->id() ) )
    return false;

  if ( !provider->load() )
    return false;

  provider->setParent( this );
  mProviders[ provider->id()] = provider;
  emit providerAdded( provider->id() );
  return true;
}

bool QgsProcessingRegistry::removeProvider( QgsProcessingProvider *provider )
{
  if ( !provider )
    return false;

  QString id = provider->id();

  if ( !mProviders.contains( id ) )
    return false;

  provider->unload();

  delete mProviders.take( id );
  emit providerRemoved( id );
  return true;
}

bool QgsProcessingRegistry::removeProvider( const QString &providerId )
{
  QgsProcessingProvider *p = providerById( providerId );
  return removeProvider( p );
}

QgsProcessingProvider *QgsProcessingRegistry::providerById( const QString &id )
{
  return mProviders.value( id, nullptr );
}

QList< const QgsProcessingAlgorithm * > QgsProcessingRegistry::algorithms() const
{
  QList< const QgsProcessingAlgorithm * > algs;
  QMap<QString, QgsProcessingProvider *>::const_iterator it = mProviders.constBegin();
  for ( ; it != mProviders.constEnd(); ++it )
  {
    algs.append( it.value()->algorithms() );
  }
  return algs;
}

const QgsProcessingAlgorithm *QgsProcessingRegistry::algorithmById( const QString &id ) const
{
  QMap<QString, QgsProcessingProvider *>::const_iterator it = mProviders.constBegin();
  for ( ; it != mProviders.constEnd(); ++it )
  {
    Q_FOREACH ( const QgsProcessingAlgorithm *alg, it.value()->algorithms() )
      if ( alg->id() == id )
        return alg;
  }

  // try mapping 'qgis' algs to 'native' algs - this allows us to freely move algorithms
  // from the python 'qgis' provider to the c++ 'native' provider without breaking API
  // or existing models
  if ( id.startsWith( QStringLiteral( "qgis:" ) ) )
  {
    QString newId = QStringLiteral( "native:" ) + id.mid( 5 );
    return algorithmById( newId );
  }
  return nullptr;
}

QgsProcessingAlgorithm *QgsProcessingRegistry::createAlgorithmById( const QString &id, const QVariantMap &configuration ) const
{
  const QgsProcessingAlgorithm *alg = algorithmById( id );
  if ( !alg )
    return nullptr;

  std::unique_ptr< QgsProcessingAlgorithm > creation( alg->create( configuration ) );
  return creation.release();
}

