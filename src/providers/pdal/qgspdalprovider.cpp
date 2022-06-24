/***************************************************************************
                         qgspdaldataprovider.cpp
                         -----------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgspdalprovider.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsjsonutils.h"
#include "qgspdalindexingtask.h"
#include "qgseptpointcloudindex.h"
#include "qgstaskmanager.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"

#include <pdal/Options.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/Reader.hpp>

#include <QQueue>
#include <QFileInfo>
#include <QDir>

#define PROVIDER_KEY QStringLiteral( "pdal" )
#define PROVIDER_DESCRIPTION QStringLiteral( "PDAL point cloud data provider" )

QQueue<QgsPdalProvider *> QgsPdalProvider::sIndexingQueue;

QgsPdalProvider::QgsPdalProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags, bool generateCopc )
  : QgsPointCloudDataProvider( uri, options, flags )
  , mIndex( nullptr )
  , mGenerateCopc( generateCopc )
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  mIsValid = load( uri );
  loadIndex( );
}

QgsPdalProvider::~QgsPdalProvider() = default;

QgsCoordinateReferenceSystem QgsPdalProvider::crs() const
{
  return mCrs;
}

QgsRectangle QgsPdalProvider::extent() const
{
  return mExtent;
}

QgsPointCloudAttributeCollection QgsPdalProvider::attributes() const
{
  return mIndex ? mIndex->attributes() : QgsPointCloudAttributeCollection();
}

static QString _outEptDir( const QString &filename )
{
  const QFileInfo fi( filename );
  const QDir directory = fi.absoluteDir();
  const QString outputDir = QStringLiteral( "%1/ept_%2" ).arg( directory.absolutePath() ).arg( fi.baseName() );
  return outputDir;
}

static QString _outCopcFile( const QString &filename )
{
  const QFileInfo fi( filename );
  const QDir directory = fi.absoluteDir();
  const QString outputFile = QStringLiteral( "%1/%2.copc.laz" ).arg( directory.absolutePath() ).arg( fi.baseName() );
  return outputFile;
}

void QgsPdalProvider::generateIndex()
{
  if ( mRunningIndexingTask || ( mIndex && mIndex->isValid() ) )
    return;

  if ( anyIndexingTaskExists() )
  {
    sIndexingQueue.push_back( this );
    return;
  }

  QString outputPath;

  if ( mGenerateCopc )
    outputPath = _outCopcFile( dataSourceUri() );
  else
    outputPath = _outEptDir( dataSourceUri() );

  QgsPdalIndexingTask *generationTask = new QgsPdalIndexingTask( dataSourceUri(), outputPath, mGenerateCopc ? QgsPdalIndexingTask::OutputFormat::Copc : QgsPdalIndexingTask::OutputFormat::Ept, QFileInfo( dataSourceUri() ).fileName() );

  connect( generationTask, &QgsPdalIndexingTask::taskTerminated, this, &QgsPdalProvider::onGenerateIndexFailed );
  connect( generationTask, &QgsPdalIndexingTask::taskCompleted, this, &QgsPdalProvider::onGenerateIndexFinished );

  mRunningIndexingTask = generationTask;
  QgsDebugMsgLevel( "Ept Generation Task Created", 2 );
  emit indexGenerationStateChanged( PointCloudIndexGenerationState::Indexing );
  QgsApplication::taskManager()->addTask( generationTask );
}

QgsPointCloudDataProvider::PointCloudIndexGenerationState QgsPdalProvider::indexingState()
{
  if ( mIndex && mIndex->isValid() )
    return PointCloudIndexGenerationState::Indexed;
  else if ( mRunningIndexingTask )
    return PointCloudIndexGenerationState::Indexing;
  else
    return PointCloudIndexGenerationState::NotIndexed;
}

void QgsPdalProvider::loadIndex( )
{
  if ( mIndex && mIndex->isValid() )
    return;
  // Try to load copc index
  if ( !mIndex || !mIndex->isValid() )
  {
    const QString outputFile = _outCopcFile( dataSourceUri() );
    const QFileInfo fi( outputFile );
    if ( fi.isFile() )
    {
      mIndex.reset( new QgsCopcPointCloudIndex );
      mIndex->load( outputFile );
    }
  }
  // Try to load ept index
  if ( !mIndex || !mIndex->isValid() )
  {
    const QString outputDir = _outEptDir( dataSourceUri() );
    const QString outEptJson = QStringLiteral( "%1/ept.json" ).arg( outputDir );
    const QFileInfo fi( outEptJson );
    if ( fi.isFile() )
    {
      mIndex.reset( new QgsEptPointCloudIndex );
      mIndex->load( outEptJson );
    }
  }
  if ( !mIndex || !mIndex->isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "pdalprovider: neither copc or ept index for dataset %1 is not correctly loaded" ).arg( dataSourceUri() ), 2 );
  }
}

void QgsPdalProvider::onGenerateIndexFinished()
{
  QgsPdalIndexingTask *task = qobject_cast<QgsPdalIndexingTask *>( QObject::sender() );
  // this may be already canceled task that we don't care anymore...
  if ( task == mRunningIndexingTask )
  {
    mRunningIndexingTask = nullptr;
    emit indexGenerationStateChanged( PointCloudIndexGenerationState::Indexed );
  }
  if ( !sIndexingQueue.empty() )
    sIndexingQueue.takeFirst()->generateIndex();
}

void QgsPdalProvider::onGenerateIndexFailed()
{
  QgsPdalIndexingTask *task = qobject_cast<QgsPdalIndexingTask *>( QObject::sender() );
  // this may be already canceled task that we don't care anymore...
  if ( task == mRunningIndexingTask )
  {
    QString error = task->errorMessage();
    if ( !error.isEmpty() )
    {
      appendError( error );
    }
    mRunningIndexingTask = nullptr;
    emit indexGenerationStateChanged( PointCloudIndexGenerationState::NotIndexed );
  }
  if ( !sIndexingQueue.empty() )
    sIndexingQueue.takeFirst()->generateIndex();
}

bool QgsPdalProvider::anyIndexingTaskExists()
{
  const QList< QgsTask * > tasks = QgsApplication::taskManager()->activeTasks();
  for ( const QgsTask *task : tasks )
  {
    const QgsPdalIndexingTask *eptTask = qobject_cast<const QgsPdalIndexingTask *>( task );
    if ( eptTask )
    {
      return true;
    }
  }
  return false;
}

qint64 QgsPdalProvider::pointCount() const
{
  return mPointCount;
}

QVariantMap QgsPdalProvider::originalMetadata() const
{
  return mOriginalMetadata;
}

bool QgsPdalProvider::isValid() const
{
  return mIsValid;
}

QString QgsPdalProvider::name() const
{
  return QStringLiteral( "pdal" );
}

QString QgsPdalProvider::description() const
{
  return QStringLiteral( "Point Clouds PDAL" );
}

QgsPointCloudIndex *QgsPdalProvider::index() const
{
  return mIndex.get();
}

bool QgsPdalProvider::load( const QString &uri )
{
  try
  {
    pdal::StageFactory stageFactory;
    const std::string driver( stageFactory.inferReaderDriver( uri.toStdString() ) );
    if ( driver.empty() )
      throw pdal::pdal_error( "No driver for " + uri.toStdString() );

    if ( pdal::Reader *reader = dynamic_cast<pdal::Reader *>( stageFactory.createStage( driver ) ) )
    {
      pdal::Options options;
      options.add( pdal::Option( "filename", uri.toStdString() ) );
      reader->setOptions( options );
      pdal::PointTable table;
      reader->prepare( table );

      const std::string tableMetadata = pdal::Utils::toJSON( table.metadata() );
      const QVariantMap readerMetadata = QgsJsonUtils::parseJson( tableMetadata ).toMap().value( QStringLiteral( "root" ) ).toMap();
      // source metadata is only value present here!
      if ( !readerMetadata.empty() )
        mOriginalMetadata = readerMetadata.constBegin().value().toMap();

      const pdal::QuickInfo quickInfo( reader->preview() );
      const double xmin = quickInfo.m_bounds.minx;
      const double xmax = quickInfo.m_bounds.maxx;
      const double ymin = quickInfo.m_bounds.miny;
      const double ymax = quickInfo.m_bounds.maxy;
      mExtent = QgsRectangle( xmin, ymin, xmax, ymax );

      mPointCount = quickInfo.m_pointCount;

      // projection
      const QString wkt = QString::fromStdString( quickInfo.m_srs.getWKT() );
      mCrs = QgsCoordinateReferenceSystem::fromWkt( wkt );
      return quickInfo.valid();
    }
    else
    {
      throw pdal::pdal_error( "No reader for " + driver );
    }
  }
  catch ( pdal::pdal_error &error )
  {
    QgsDebugMsg( QStringLiteral( "Error loading PDAL data source %1" ).arg( error.what() ) );
    QgsMessageLog::logMessage( tr( "Data source is invalid (%1)" ).arg( error.what() ), QStringLiteral( "PDAL" ) );
    return false;
  }
}

QString QgsPdalProviderMetadata::sFilterString;
QStringList QgsPdalProviderMetadata::sExtensions;

QgsPdalProviderMetadata::QgsPdalProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsPdalProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconPointCloudLayer.svg" ) );
}

QgsPdalProvider *QgsPdalProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsPdalProvider( uri, options, flags );
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsPdalProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}

QVariantMap QgsPdalProviderMetadata::decodeUri( const QString &uri ) const
{
  const QString path = uri;
  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "path" ), path );
  return uriComponents;
}

int QgsPdalProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );

  if ( filePath.endsWith( QStringLiteral( ".copc.laz" ), Qt::CaseSensitivity::CaseInsensitive ) )
    return 0;

  if ( sExtensions.contains( fi.suffix(), Qt::CaseInsensitive ) )
    return 100;

  return 0;
}

QList<QgsMapLayerType> QgsPdalProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );
  if ( sExtensions.contains( fi.suffix(), Qt::CaseInsensitive ) )
    return QList<QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;

  return QList<QgsMapLayerType>();
}

QList<QgsProviderSublayerDetails> QgsPdalProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );

  if ( sExtensions.contains( fi.suffix(), Qt::CaseInsensitive ) )
  {
    QgsProviderSublayerDetails details;
    details.setUri( uri );
    details.setProviderKey( QStringLiteral( "pdal" ) );
    details.setType( QgsMapLayerType::PointCloudLayer );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( uri ) );
    return {details};
  }
  else
  {
    return {};
  }
}

QString QgsPdalProviderMetadata::filters( QgsProviderMetadata::FilterType type )
{
  switch ( type )
  {
    case QgsProviderMetadata::FilterType::FilterVector:
    case QgsProviderMetadata::FilterType::FilterRaster:
    case QgsProviderMetadata::FilterType::FilterMesh:
    case QgsProviderMetadata::FilterType::FilterMeshDataset:
      return QString();

    case QgsProviderMetadata::FilterType::FilterPointCloud:
      buildSupportedPointCloudFileFilterAndExtensions();

      return sFilterString;
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsPdalProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<QgsMapLayerType> QgsPdalProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::PointCloudLayer };
}

QString QgsPdalProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

void QgsPdalProviderMetadata::buildSupportedPointCloudFileFilterAndExtensions()
{
  // get supported extensions
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {
    const pdal::StageFactory f;
    pdal::PluginManager<pdal::Stage>::loadAll();
    const pdal::StringList stages = pdal::PluginManager<pdal::Stage>::names();
    pdal::StageExtensions &extensions = pdal::PluginManager<pdal::Stage>::extensions();

    // Let's call the defaultReader() method just so we trigger the private method extensions.load()
    extensions.defaultReader( "laz" );

    const QStringList allowedReaders {
      QStringLiteral( "readers.las" ),
      QStringLiteral( "readers.e57" ),
      QStringLiteral( "readers.bpf" ) };
    for ( const auto &stage : stages )
    {
      if ( ! allowedReaders.contains( QString::fromStdString( stage ) ) )
        continue;

      const pdal::StringList readerExtensions = extensions.extensions( stage );
      for ( const auto &extension : readerExtensions )
      {
        sExtensions.append( QString::fromStdString( extension ) );
      }
    }
    sExtensions.sort();
    const QString extensionsString = QStringLiteral( "*." ).append( sExtensions.join( QStringLiteral( " *." ) ) );
    sFilterString = tr( "PDAL Point Clouds" ) + QString( " (%1 %2)" ).arg( extensionsString, extensionsString.toUpper() );
  } );
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsPdalProviderMetadata();
}
