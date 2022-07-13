/***************************************************************************
                         qgspointcloudlayerexporter.cpp
                         ---------------------
    begin                : July 2022
    copyright            : (C) 2022 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QQueue>
#include <QFileInfo>

#include "qgspointcloudlayerexporter.h"
#include "qgsmemoryproviderutils.h"
#include "qgspointcloudrequest.h"
#include "qgsvectorfilewriter.h"
#include "qgsproject.h"


QgsPointCloudLayerExporter::QgsPointCloudLayerExporter( QgsPointCloudLayer *layer )
  : mLayerAttributeCollection( layer->attributes() )
  , mIndex( layer->dataProvider()->index()->clone().release() )
  , mName( QString( layer->name() ) )
  , mCrs( QgsCoordinateReferenceSystem( layer->crs() ) )
{
  QStringList allAttributeNames;
  const QVector<QgsPointCloudAttribute> allAttributes = mLayerAttributeCollection.attributes();
  for ( const QgsPointCloudAttribute &attribute : allAttributes )
  {
    allAttributeNames.append( attribute.name() );
  }
  setAttributes( allAttributeNames );
}

bool QgsPointCloudLayerExporter::setFormat( const QString &format )
{
  if ( format == QLatin1String( "memory" ) )
  {
    mFormat = format;
    return true;
  }

  const QList< QgsVectorFileWriter::DriverDetails > drivers = QgsVectorFileWriter::ogrDriverList();

  for ( const QgsVectorFileWriter::DriverDetails &driver : drivers )
  {
    if ( ! format.compare( driver.driverName, Qt::CaseInsensitive ) )
    {
      mFormat = driver.driverName;
      return true;
    }
  }
  return false;
}

void QgsPointCloudLayerExporter::setAttributes( const QStringList &attributeList )
{
  mRequestedAttributes.clear();

  const QVector<QgsPointCloudAttribute> allAttributes = mLayerAttributeCollection.attributes();
  for ( const QgsPointCloudAttribute &attribute : allAttributes )
  {
    // Don't add x, y, z or duplicate attributes
    if ( attribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) &&
         attribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) &&
         attribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) &&
         attributeList.contains( attribute.name() ) &&
         ! mRequestedAttributes.contains( attribute.name() ) )
    {
      mRequestedAttributes.append( attribute.name() );
    }
  }
}

const QgsPointCloudAttributeCollection QgsPointCloudLayerExporter::requestedAttributeCollection()
{
  const QVector<QgsPointCloudAttribute> allAttributes = mLayerAttributeCollection.attributes();
  QgsPointCloudAttributeCollection requestAttributes;
  for ( const QgsPointCloudAttribute &attribute : allAttributes )
  {
    // For this collection we also need x, y, z apart from the requested attributes
    if ( attribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) ||
         attribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) ||
         attribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) ||
         mRequestedAttributes.contains( attribute.name(), Qt::CaseInsensitive ) )
    {
      requestAttributes.push_back( attribute );
    }
  }
  return requestAttributes;
}

QgsFields QgsPointCloudLayerExporter::outputFields()
{
  const QVector<QgsPointCloudAttribute> attributes = mLayerAttributeCollection.attributes();

  QgsFields fields;
  for ( const QgsPointCloudAttribute &attribute : attributes )
  {
    if ( mRequestedAttributes.contains( attribute.name(), Qt::CaseInsensitive ) )
      fields.append( QgsField( attribute.name(), attribute.variantType(), attribute.displayType() ) );
  }
  return fields;
}


void QgsPointCloudLayerExporter::exportToSink( QgsFeatureSink *sink )
{
  QVector<IndexedPointCloudNode> nodes;
  qint64 pointCount = 0;
  QQueue<IndexedPointCloudNode> queue;
  queue.push_back( mIndex->root() );
  while ( !queue.empty() )
  {
    IndexedPointCloudNode node = queue.front();
    queue.pop_front();
    if ( !nodes.contains( node ) &&
         mExtent.intersects( mIndex->nodeMapExtent( node ) ) &&
         mZRange.overlaps( mIndex->nodeZRange( node ) ) )
    {
      pointCount += mIndex->nodePointCount( node );
      nodes.push_back( node );
    }
    if ( pointCount >= mPointsLimit )
      break;
    for ( const IndexedPointCloudNode &child : mIndex->nodeChildren( node ) )
    {
      queue.push_back( child );
    }
  }

  const QgsCoordinateTransform ct = QgsCoordinateTransform( mIndex->crs(), mCrs, QgsCoordinateTransformContext() );

  int pointsSkipped = 0;
  const qint64 pointsToExport = std::max< qint64 >( std::min( mPointsLimit, pointCount ), 1 );
  QgsPointCloudRequest request;
  request.setAttributes( requestedAttributeCollection() );
  std::unique_ptr<QgsPointCloudBlock> block = nullptr;
  QgsFeatureList fl;
  qint64 pointsExported = 0;
  for ( const IndexedPointCloudNode &node : nodes )
  {
    block.reset( mIndex->nodeData( node, request ) );
    const QgsPointCloudAttributeCollection attributesCollection = block->attributes();
    const char *ptr = block->data();
    int count = block->pointCount();
    int recordSize = attributesCollection.pointRecordSize();
    const QgsVector3D scale = block->scale();
    const QgsVector3D offset = block->offset();
    int xOffset;
    int yOffset;
    int zOffset;
    attributesCollection.find( QStringLiteral( "X" ), xOffset );
    attributesCollection.find( QStringLiteral( "Y" ), yOffset );
    attributesCollection.find( QStringLiteral( "Z" ), zOffset );
    for ( int i = 0; i < count; ++i )
    {

      if ( mFeedback )
      {
        mFeedback->setProgress( 100 * static_cast< float >( pointsExported + pointsSkipped ) / pointsToExport );
        if ( mFeedback->isCanceled() )
          return;
      }

      if ( pointsExported > mPointsLimit )
        break;

      double x, y, z;
      QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize,
                                           xOffset, QgsPointCloudAttribute::DataType::Int32,
                                           yOffset, QgsPointCloudAttribute::DataType::Int32,
                                           zOffset, QgsPointCloudAttribute::DataType::Int32,
                                           scale, offset,
                                           x, y, z );
      if ( ! mZRange.contains( z ) ||
           ! mExtent.contains( x, y ) )
      {
        ++pointsSkipped;
        continue;
      }

      QgsFeature feature;
      ct.transformInPlace( x, y, z );
      feature.setGeometry( QgsGeometry( new QgsPoint( x, y, z ) ) );
      const auto attributeMap = QgsPointCloudAttribute::getAttributeMap( ptr, i * recordSize, attributesCollection );
      QgsAttributes featureAttributes;
      for ( const QString &attribute : std::as_const( mRequestedAttributes ) )
      {
        const double val = attributeMap[ attribute ].toDouble();
        featureAttributes.append( val );
      }
      feature.setAttributes( featureAttributes );
      fl.append( feature );
      ++pointsExported;
    }
  }
  sink->addFeatures( fl );
}

QgsVectorLayer *QgsPointCloudLayerExporter::exportToMemoryLayer()
{
  if ( ! mIndex->isValid() )
    return nullptr;

  std::unique_ptr< QgsVectorLayer > layer( QgsMemoryProviderUtils::createMemoryLayer( mName, outputFields(), QgsWkbTypes::PointZ, mCrs ) );
  exportToSink( layer->dataProvider() );
  return layer.release();
}

QgsVectorLayer *QgsPointCloudLayerExporter::exportToVectorFile( const QString &filename )
{
  if ( ! mIndex->isValid() )
    return nullptr;

  if ( ! filename.isEmpty() )
    mFilename = filename;

  if ( mFilename.isEmpty() )
    return nullptr;

  const QString extension = QFileInfo( mFilename ).suffix();
  if ( mFormat.isEmpty() )
  {
    const QString driver = QgsVectorFileWriter::driverForExtension( extension );
    mFormat = driver.isEmpty() ? QStringLiteral( "GPKG" ) : driver;
  }

  std::unique_ptr< QgsVectorLayer > memoryLayer( exportToMemoryLayer() );

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.layerName = mName;
  saveOptions.driverName = mFormat;
  saveOptions.datasourceOptions = QgsVectorFileWriter::defaultDatasetOptions( mFormat );
  saveOptions.layerOptions = QgsVectorFileWriter::defaultLayerOptions( mFormat );
  saveOptions.symbologyExport = QgsVectorFileWriter::NoSymbology;
  saveOptions.actionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
  saveOptions.feedback = mFeedback;

  QgsVectorFileWriter::writeAsVectorFormatV3( memoryLayer.get(), mFilename, memoryLayer->transformContext(), saveOptions );

  QgsVectorLayer *layer = new QgsVectorLayer( mFilename, mName, QStringLiteral( "ogr" ) );
  return layer;
}

void QgsPointCloudLayerExporter::exportToPdalFile( const QString &filename )
{
  Q_UNUSED( filename )
}



//
// QgsPointCloudLayerExporterTask
//

QgsPointCloudLayerExporterTask::QgsPointCloudLayerExporterTask( QgsPointCloudLayerExporter *exporter )
  : QgsTask( tr( "Exporting Pointcloud" ), QgsTask::CanCancel )
  , mExp( exporter )
  , mOwnedFeedback( new QgsFeedback() )
{
}

void QgsPointCloudLayerExporterTask::cancel()
{
  mOwnedFeedback->cancel();
  QgsTask::cancel();
}

bool QgsPointCloudLayerExporterTask::run()
{
  if ( !mExp )
    return false;

  connect( mOwnedFeedback.get(), &QgsFeedback::progressChanged, this, &QgsPointCloudLayerExporterTask::setProgress );
  mExp->setFeedback( mOwnedFeedback.get() );

  if ( mExp->format() == QLatin1String( "memory" ) )
    mOutputLayer = mExp->exportToMemoryLayer();
  else
    mOutputLayer = mExp->exportToVectorFile();

  return mError == Qgis::VectorExportResult::Success;
}

void QgsPointCloudLayerExporterTask::finished( bool result )
{
  delete mExp;

  if ( result )
  {
    QgsProject::instance()->addMapLayer( mOutputLayer );
    emit exportComplete( mOutputLayer );
  }
  else
  {
    emit errorOccurred( mError, mErrorMessage );
    delete mOutputLayer;
  }
}
