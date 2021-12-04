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
#include "qgsprocessingparametertypeimpl.h"
#include "qgsprocessingparametermeshdataset.h"
#include "qgsprocessingparametervectortilewriterlayers.h"
#include "qgsprocessingparametertininputlayers.h"
#include "qgsprocessingparameterfieldmap.h"
#include "qgsprocessingparameteraggregate.h"
#include "qgsprocessingparameterdxflayers.h"

QgsProcessingRegistry::QgsProcessingRegistry( QObject *parent SIP_TRANSFERTHIS )
  : QObject( parent )
{
  addParameterType( new QgsProcessingParameterTypeRasterLayer() );
  addParameterType( new QgsProcessingParameterTypeVectorLayer() );
  addParameterType( new QgsProcessingParameterTypeMeshLayer() );
  addParameterType( new QgsProcessingParameterTypeMapLayer() );
  addParameterType( new QgsProcessingParameterTypeBoolean() );
  addParameterType( new QgsProcessingParameterTypeExpression() );
  addParameterType( new QgsProcessingParameterTypeCrs() );
  addParameterType( new QgsProcessingParameterTypeRange() );
  addParameterType( new QgsProcessingParameterTypePoint() );
  addParameterType( new QgsProcessingParameterTypeGeometry() );
  addParameterType( new QgsProcessingParameterTypeEnum() );
  addParameterType( new QgsProcessingParameterTypeExtent() );
  addParameterType( new QgsProcessingParameterTypeMatrix() );
  addParameterType( new QgsProcessingParameterTypeFile() ) ;
  addParameterType( new QgsProcessingParameterTypeField() );
  addParameterType( new QgsProcessingParameterTypeVectorDestination() );
  addParameterType( new QgsProcessingParameterTypeRasterDestination() );
  addParameterType( new QgsProcessingParameterTypeFileDestination() );
  addParameterType( new QgsProcessingParameterTypeFolderDestination() );
  addParameterType( new QgsProcessingParameterTypeString() );
  addParameterType( new QgsProcessingParameterTypeAuthConfig() );
  addParameterType( new QgsProcessingParameterTypeMultipleLayers() );
  addParameterType( new QgsProcessingParameterTypeFeatureSource() );
  addParameterType( new QgsProcessingParameterTypeNumber() );
  addParameterType( new QgsProcessingParameterTypeDistance() );
  addParameterType( new QgsProcessingParameterTypeDuration() );
  addParameterType( new QgsProcessingParameterTypeScale() );
  addParameterType( new QgsProcessingParameterTypeBand() );
  addParameterType( new QgsProcessingParameterTypeFeatureSink() );
  addParameterType( new QgsProcessingParameterTypeLayout() );
  addParameterType( new QgsProcessingParameterTypeLayoutItem() );
  addParameterType( new QgsProcessingParameterTypeColor() );
  addParameterType( new QgsProcessingParameterTypeCoordinateOperation() );
  addParameterType( new QgsProcessingParameterTypeMapTheme() );
  addParameterType( new QgsProcessingParameterTypeDateTime() );
  addParameterType( new QgsProcessingParameterTypeProviderConnection() );
  addParameterType( new QgsProcessingParameterTypeDatabaseSchema() );
  addParameterType( new QgsProcessingParameterTypeDatabaseTable() );
  addParameterType( new QgsProcessingParameterTypeVectorTileWriterLayers() );
  addParameterType( new QgsProcessingParameterTypeFieldMapping() );
  addParameterType( new QgsProcessingParameterTypeAggregate() );
  addParameterType( new QgsProcessingParameterTypeTinInputLayers() );
  addParameterType( new QgsProcessingParameterTypeDxfLayers() );
  addParameterType( new QgsProcessingParameterTypeMeshDatasetGroups() );
  addParameterType( new QgsProcessingParameterTypeMeshDatasetTime() );
  addParameterType( new QgsProcessingParameterTypePointCloudLayer() );
  addParameterType( new QgsProcessingParameterTypeAnnotationLayer() );
  addParameterType( new QgsProcessingParameterTypePointCloudDestination() );
}

QgsProcessingRegistry::~QgsProcessingRegistry()
{
  const auto constMProviders = mProviders;
  for ( QgsProcessingProvider *p : constMProviders )
  {
    removeProvider( p );
  }

  const auto parameterTypes = mParameterTypes.values();

  for ( QgsProcessingParameterType *type : parameterTypes )
  {
    removeParameterType( type );
  }
}

bool QgsProcessingRegistry::addProvider( QgsProcessingProvider *provider )
{
  if ( !provider )
    return false;

  if ( mProviders.contains( provider->id() ) )
  {
    QgsLogger::warning( QStringLiteral( "Duplicate provider %1 registered" ).arg( provider->id() ) );
    delete provider;
    return false;
  }

  if ( !provider->load() )
  {
    QgsLogger::warning( QStringLiteral( "Provider %1 cannot load" ).arg( provider->id() ) );
    delete provider;
    return false;
  }

  provider->setParent( this );
  mProviders[ provider->id()] = provider;
  emit providerAdded( provider->id() );
  return true;
}

bool QgsProcessingRegistry::removeProvider( QgsProcessingProvider *provider )
{
  if ( !provider )
    return false;

  const QString id = provider->id();

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

const QgsProcessingAlgorithm *QgsProcessingRegistry::algorithmById( const QString &constId ) const
{
  // allow mapping of algorithm via registered algorithm aliases
  const QString id = mAlgorithmAliases.value( constId, constId );

  QMap<QString, QgsProcessingProvider *>::const_iterator it = mProviders.constBegin();
  for ( ; it != mProviders.constEnd(); ++it )
  {
    const auto constAlgorithms = it.value()->algorithms();
    for ( const QgsProcessingAlgorithm *alg : constAlgorithms )
      if ( alg->id() == id )
        return alg;
  }

  // try mapping 'qgis' algs to 'native' algs - this allows us to freely move algorithms
  // from the python 'qgis' provider to the c++ 'native' provider without breaking API
  // or existing models
  if ( id.startsWith( QLatin1String( "qgis:" ) ) )
  {
    const QString newId = QStringLiteral( "native:" ) + id.mid( 5 );
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

void QgsProcessingRegistry::addAlgorithmAlias( const QString &aliasId, const QString &actualId )
{
  mAlgorithmAliases.insert( aliasId, actualId );
}

bool QgsProcessingRegistry::addParameterType( QgsProcessingParameterType *type )
{
  if ( !mParameterTypes.contains( type->id() ) )
  {
    mParameterTypes.insert( type->id(), type );
    emit parameterTypeAdded( type );
    return true;
  }
  else
  {
    QgsLogger::warning( QStringLiteral( "Duplicate parameter type %1 (\"%2\") registered" ).arg( type->id(), type->name() ) );

    if ( mParameterTypes.value( type->id() ) != type )
      delete type;

    return false;
  }
}

void QgsProcessingRegistry::removeParameterType( QgsProcessingParameterType *type )
{
  mParameterTypes.remove( type->id() );
  emit parameterTypeRemoved( type );
  delete type;
}

QgsProcessingParameterType *QgsProcessingRegistry::parameterType( const QString &id ) const
{
  return mParameterTypes.value( id );
}

QList<QgsProcessingParameterType *> QgsProcessingRegistry::parameterTypes() const
{
  return mParameterTypes.values();
}
