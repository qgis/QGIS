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
#include "qgspdaleptgenerationtask.h"
#include "qgseptpointcloudindex.h"
#include "qgstaskmanager.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"

#include <pdal/io/LasReader.hpp>
#include <pdal/io/LasHeader.hpp>
#include <pdal/Options.hpp>

#include <QQueue>
#include <QFileInfo>
#include <QDir>

#define PROVIDER_KEY QStringLiteral( "pdal" )
#define PROVIDER_DESCRIPTION QStringLiteral( "PDAL point cloud data provider" )

QQueue<QgsPdalProvider *> QgsPdalProvider::sIndexingQueue;

QgsPdalProvider::QgsPdalProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags )
  , mIndex( new QgsEptPointCloudIndex )
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
  return mIndex->attributes();
}

QVariantList QgsPdalProvider::metadataClasses( const QString &attribute ) const
{
  return mIndex->metadataClasses( attribute );
}

QVariant QgsPdalProvider::metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const
{
  return mIndex->metadataClassStatistic( attribute, value, statistic );
}

static QString _outdir( const QString &filename )
{
  const QFileInfo fi( filename );
  const QDir directory = fi.absoluteDir();
  const QString outputDir = QStringLiteral( "%1/ept_%2" ).arg( directory.absolutePath() ).arg( fi.baseName() );
  return outputDir;
}

void QgsPdalProvider::generateIndex()
{
  if ( mRunningIndexingTask || mIndex->isValid() )
    return;

  if ( anyIndexingTaskExists() )
  {
    sIndexingQueue.push_back( this );
    return;
  }

  const QString outputDir = _outdir( dataSourceUri() );

  QgsPdalEptGenerationTask *generationTask = new QgsPdalEptGenerationTask( dataSourceUri(), outputDir, QFileInfo( dataSourceUri() ).fileName() );

  connect( generationTask, &QgsPdalEptGenerationTask::taskTerminated, this, &QgsPdalProvider::onGenerateIndexFailed );
  connect( generationTask, &QgsPdalEptGenerationTask::taskCompleted, this, &QgsPdalProvider::onGenerateIndexFinished );

  mRunningIndexingTask = generationTask;
  QgsDebugMsgLevel( "Ept Generation Task Created", 2 );
  emit indexGenerationStateChanged( PointCloudIndexGenerationState::Indexing );
  QgsApplication::taskManager()->addTask( generationTask );
}

QgsPointCloudDataProvider::PointCloudIndexGenerationState QgsPdalProvider::indexingState()
{
  if ( mIndex->isValid() )
    return PointCloudIndexGenerationState::Indexed;
  else if ( mRunningIndexingTask )
    return PointCloudIndexGenerationState::Indexing;
  else
    return PointCloudIndexGenerationState::NotIndexed;
}

void QgsPdalProvider::loadIndex( )
{
  if ( mIndex->isValid() )
    return;

  const QString outputDir = _outdir( dataSourceUri() );
  const QString outEptJson = QStringLiteral( "%1/ept.json" ).arg( outputDir );
  const QFileInfo fi( outEptJson );
  if ( fi.isFile() )
  {
    mIndex->load( outEptJson );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "pdalprovider: ept index %1 is not correctly loaded" ).arg( outEptJson ), 2 );
  }
}

void QgsPdalProvider::onGenerateIndexFinished()
{
  QgsPdalEptGenerationTask *task = qobject_cast<QgsPdalEptGenerationTask *>( QObject::sender() );
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
  QgsPdalEptGenerationTask *task = qobject_cast<QgsPdalEptGenerationTask *>( QObject::sender() );
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
    const QgsPdalEptGenerationTask *eptTask = qobject_cast<const QgsPdalEptGenerationTask *>( task );
    if ( eptTask )
    {
      return true;
    }
  }
  return false;
}

QVariant QgsPdalProvider::metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const
{
  if ( mIndex )
    return mIndex->metadataStatistic( attribute, statistic );
  else
    return QVariant();
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
    const pdal::Option las_opt( "filename", uri.toStdString() );
    pdal::Options las_opts;
    las_opts.add( las_opt );
    pdal::LasReader las_reader;
    las_reader.setOptions( las_opts );
    pdal::PointTable table;
    las_reader.prepare( table );
    const pdal::LasHeader las_header = las_reader.header();

    const std::string tableMetadata = pdal::Utils::toJSON( table.metadata() );
    const QVariantMap readerMetadata = QgsJsonUtils::parseJson( tableMetadata ).toMap().value( QStringLiteral( "root" ) ).toMap();
    // source metadata is only value present here!
    if ( !readerMetadata.empty() )
      mOriginalMetadata = readerMetadata.constBegin().value().toMap();

    // extent
    /*
    double scale_x = las_header.scaleX();
    double scale_y = las_header.scaleY();
    double scale_z = las_header.scaleZ();

    double offset_x = las_header.offsetX();
    double offset_y = las_header.offsetY();
    double offset_z = las_header.offsetZ();
    */

    const double xmin = las_header.minX();
    const double xmax = las_header.maxX();
    const double ymin = las_header.minY();
    const double ymax = las_header.maxY();
    mExtent = QgsRectangle( xmin, ymin, xmax, ymax );

    mPointCount = las_header.pointCount();

    // projection
    const QString wkt = QString::fromStdString( las_reader.getSpatialReference().getWKT() );
    mCrs = QgsCoordinateReferenceSystem::fromWkt( wkt );
    return true;
  }
  catch ( pdal::pdal_error &error )
  {
    QgsDebugMsg( QStringLiteral( "Error loading PDAL data source %1" ).arg( error.what() ) );
    QgsMessageLog::logMessage( tr( "Data source is invalid (%1)" ).arg( error.what() ), QStringLiteral( "PDAL" ) );
    return false;
  }
}

QgsPdalProviderMetadata::QgsPdalProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
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

  if ( fi.suffix().compare( QLatin1String( "las" ), Qt::CaseInsensitive ) == 0 || fi.suffix().compare( QLatin1String( "laz" ), Qt::CaseInsensitive ) == 0 )
    return 100;

  return 0;
}

QList<QgsMapLayerType> QgsPdalProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );
  if ( fi.suffix().compare( QLatin1String( "las" ), Qt::CaseInsensitive ) == 0 || fi.suffix().compare( QLatin1String( "laz" ), Qt::CaseInsensitive ) == 0 || filePath.endsWith( QStringLiteral( ".copc.laz" ), Qt::CaseInsensitive ) )
    return QList<QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;

  return QList<QgsMapLayerType>();
}

QList<QgsProviderSublayerDetails> QgsPdalProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );

  if ( fi.suffix().compare( QLatin1String( "las" ), Qt::CaseInsensitive ) == 0 || fi.suffix().compare( QLatin1String( "laz" ), Qt::CaseInsensitive ) == 0 || filePath.endsWith( QStringLiteral( ".copc.laz" ), Qt::CaseInsensitive ) )
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
      // TODO get the available/supported filters from PDAL library
      return QObject::tr( "PDAL Point Clouds" ) + QStringLiteral( " (*.laz *.las *.LAZ *.LAS)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsPdalProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QString QgsPdalProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsPdalProviderMetadata();
}
