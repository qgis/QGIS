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
#include "moc_qgsprocessingregistry.cpp"
#include "qgsvectorfilewriter.h"
#include "qgsprocessingparametertypeimpl.h"
#include "qgsprocessingparametermeshdataset.h"
#include "qgsprocessingparametervectortilewriterlayers.h"
#include "qgsprocessingparametertininputlayers.h"
#include "qgsprocessingparameterfieldmap.h"
#include "qgsprocessingparameteraggregate.h"
#include "qgsprocessingparameterdxflayers.h"
#include "qgsprocessingparameteralignrasterlayers.h"

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
  addParameterType( new QgsProcessingParameterTypeArea() );
  addParameterType( new QgsProcessingParameterTypeVolume() );
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
  addParameterType( new QgsProcessingParameterTypePointCloudAttribute() );
  addParameterType( new QgsProcessingParameterTypeVectorTileDestination() );
  addParameterType( new QgsProcessingParameterTypeAlignRasterLayers() );
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

  if ( providerById( provider->id() ) )
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

  mCachedInformation.clear();
  connect( provider, &QgsProcessingProvider::algorithmsLoaded, this, [this]
  {
    mCachedInformation.clear();
  } );

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

  mCachedInformation.clear();

  emit providerRemoved( id );
  return true;
}

bool QgsProcessingRegistry::removeProvider( const QString &providerId )
{
  QgsProcessingProvider *p = providerById( providerId );
  return removeProvider( p );
}

QgsProcessingProvider *QgsProcessingRegistry::providerById( const QString &id ) const
{
  auto it = mProviders.constFind( id );
  if ( it != mProviders.constEnd() )
    return it.value();

  // transparently map old references to "grass7" provider to "grass" provider
  if ( id.compare( QLatin1String( "grass7" ), Qt::CaseInsensitive ) == 0 )
    return providerById( QStringLiteral( "grass" ) );

  return nullptr;
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

QgsProcessingAlgorithmInformation QgsProcessingRegistry::algorithmInformation( const QString &id ) const
{
  const auto it = mCachedInformation.constFind( id );
  if ( it != mCachedInformation.constEnd() )
    return *it;

  QgsProcessingAlgorithmInformation info;
  if ( const QgsProcessingAlgorithm *algorithm = algorithmById( id ) )
  {
    info.displayName = algorithm->displayName();
    info.icon = algorithm->icon();
  }
  mCachedInformation.insert( id, info );
  return info;
}

const QgsProcessingAlgorithm *QgsProcessingRegistry::algorithmById( const QString &constId ) const
{
  if ( constId.isEmpty() )
    return nullptr;

  // allow mapping of algorithm via registered algorithm aliases
  const QString id = mAlgorithmAliases.value( constId, constId );

  // try to match just the one target provider, if we can determine it from the id easily
  static thread_local QRegularExpression reSplitProviderId( QStringLiteral( "^(.*?):(.*)$" ) );
  const QRegularExpressionMatch match = reSplitProviderId.match( id );
  if ( match.hasMatch() )
  {
    if ( QgsProcessingProvider *provider = providerById( match.captured( 1 ) ) )
    {
      if ( const QgsProcessingAlgorithm *algorithm = provider->algorithm( match.captured( 2 ) ) )
        return algorithm;
    }

    // try mapping 'qgis' algs to 'native' algs - this allows us to freely move algorithms
    // from the python 'qgis' provider to the c++ 'native' provider without breaking API
    // or existing models
    if ( match.captured( 1 ) == QLatin1String( "qgis" ) )
    {
      const QString algorithmName = id.mid( 5 );
      if ( QgsProcessingProvider *provider = mProviders.value( QStringLiteral( "native" ) ) )
      {
        if ( const QgsProcessingAlgorithm *algorithm = provider->algorithm( algorithmName ) )
          return algorithm;
      }
    }
  }

  // slow: iterate through ALL providers to find a match
  QMap<QString, QgsProcessingProvider *>::const_iterator it = mProviders.constBegin();
  for ( ; it != mProviders.constEnd(); ++it )
  {
    const QList< const QgsProcessingAlgorithm * > algorithms = it.value()->algorithms();
    for ( const QgsProcessingAlgorithm *alg : algorithms )
      if ( alg->id() == id )
        return alg;
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
