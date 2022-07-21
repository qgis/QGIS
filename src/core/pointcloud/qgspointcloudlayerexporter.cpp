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

#ifdef HAVE_PDAL_QGIS
#include <pdal/StageFactory.hpp>
#include <pdal/io/BufferReader.hpp>
#include <pdal/Dimension.hpp>
#endif

QgsPointCloudLayerExporter::QgsPointCloudLayerExporter( QgsPointCloudLayer *layer )
  : mLayerAttributeCollection( layer->attributes() )
  , mIndex( layer->dataProvider()->index()->clone().release() )
  , mSourceCrs( QgsCoordinateReferenceSystem( layer->crs() ) )
  , mTargetCrs( QgsCoordinateReferenceSystem( layer->crs() ) )
{
  bool ok;
  mPointRecordFormat = layer->dataProvider()->originalMetadata().value( QStringLiteral( "dataformat_id" ) ).toInt( &ok );
  if ( !ok )
    mPointRecordFormat = 3;

  mSupportedFormats << QStringLiteral( "memory" )
#ifdef HAVE_PDAL_QGIS
                    << QStringLiteral( "LAZ" )
#endif
                    << QStringLiteral( "GPKG" )
                    << QStringLiteral( "ESRI Shapefile" )
                    << QStringLiteral( "DXF" );

  QStringList allAttributeNames;
  const QVector<QgsPointCloudAttribute> allAttributes = mLayerAttributeCollection.attributes();
  for ( const QgsPointCloudAttribute &attribute : allAttributes )
  {
    allAttributeNames.append( attribute.name() );
  }
  setAttributes( allAttributeNames );
}

QgsPointCloudLayerExporter::~QgsPointCloudLayerExporter()
{
//  delete mOutputLayer;
  delete mVectorSink;
  delete mTransform;
}

bool QgsPointCloudLayerExporter::setFormat( const QString &format )
{
  if ( mSupportedFormats.contains( format, Qt::CaseInsensitive ) )
  {
    mFormat = format;
    return true;
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

void QgsPointCloudLayerExporter::doExport()
{
  mTransform = new QgsCoordinateTransform( mSourceCrs, mTargetCrs, mTransformContext );
  if ( mExtent.isFinite() )
  {
    mExtent = mTransform->transformBoundingBox( mExtent, Qgis::TransformDirection::Reverse );
  }

  if ( mFormat == QLatin1String( "memory" ) )
  {
    mMemoryLayer = QgsMemoryProviderUtils::createMemoryLayer( mName, outputFields(), QgsWkbTypes::PointZ, mTargetCrs );
    ExporterMemory exp = ExporterMemory( this );
    exp.run();
  }
  else if ( mFormat == QLatin1String( "LAZ" ) )
  {
    // PDAL may throw exceptions
    try
    {
      ExporterPdal exp = ExporterPdal( this );
      exp.run();
    }
    catch ( std::runtime_error &e )
    {
      setLastError( QString::fromLatin1( e.what() ) );
      QgsDebugMsg( QStringLiteral( "PDAL has thrown an exception: {}" ).arg( e.what() ) );
    }
  }
  else
  {
    QgsVectorFileWriter::SaveVectorOptions saveOptions;
    saveOptions.layerName = mName;
    saveOptions.driverName = mFormat;
    saveOptions.datasourceOptions = QgsVectorFileWriter::defaultDatasetOptions( mFormat );
    saveOptions.layerOptions = QgsVectorFileWriter::defaultLayerOptions( mFormat );
    saveOptions.symbologyExport = QgsVectorFileWriter::NoSymbology;
    saveOptions.actionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
    saveOptions.feedback = mFeedback;
    mVectorSink = QgsVectorFileWriter::create( mFilename, outputFields(), QgsWkbTypes::PointZ, mTargetCrs, QgsCoordinateTransformContext(), saveOptions );
    ExporterVector exp = ExporterVector( this );
    exp.run();
  }
}

QgsMapLayer *QgsPointCloudLayerExporter::getExportedLayer()
{
  if ( mFormat == QLatin1String( "memory" ) && mMemoryLayer )
    return mMemoryLayer;

  if ( mFormat == QLatin1String( "LAZ" ) )
    return new QgsPointCloudLayer( mFilename, mName, QStringLiteral( "pdal" ) );

  QString uri( mFilename );
  if ( ! mName.isEmpty() )
    uri += "|layername=" + mName;
  return new QgsVectorLayer( uri, mName, QStringLiteral( "ogr" ) );
}

//
// ExporterBase
//

void QgsPointCloudLayerExporter::ExporterBase::run()
{
  QVector<IndexedPointCloudNode> nodes;
  qint64 pointCount = 0;
  QQueue<IndexedPointCloudNode> queue;
  queue.push_back( mParent->mIndex->root() );
  while ( !queue.empty() )
  {
    IndexedPointCloudNode node = queue.front();
    queue.pop_front();
    if ( !nodes.contains( node ) &&
         mParent->mExtent.intersects( mParent->mIndex->nodeMapExtent( node ) ) &&
         mParent->mZRange.overlaps( mParent->mIndex->nodeZRange( node ) ) )
    {
      pointCount += mParent->mIndex->nodePointCount( node );
      nodes.push_back( node );
    }
    if ( pointCount >= mParent->mPointsLimit )
      break;
    for ( const IndexedPointCloudNode &child : mParent->mIndex->nodeChildren( node ) )
    {
      queue.push_back( child );
    }
  }



  int pointsSkipped = 0;
  const qint64 pointsToExport = std::max< qint64 >( std::min( mParent->mPointsLimit, pointCount ), 1 );
  QgsPointCloudRequest request;
  request.setAttributes( mParent->requestedAttributeCollection() );
  std::unique_ptr<QgsPointCloudBlock> block = nullptr;
  qint64 pointsExported = 0;
  for ( const IndexedPointCloudNode &node : nodes )
  {
    block.reset( mParent->mIndex->nodeData( node, request ) );
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

      if ( mParent->mFeedback )
      {
        mParent->mFeedback->setProgress( 100 * static_cast< float >( pointsExported + pointsSkipped ) / pointsToExport );
        if ( mParent->mFeedback->isCanceled() )
        {
          mParent->setLastError( QObject::tr( "Canceled by user" ) );
          return;
        }
      }

      if ( pointsExported > mParent->mPointsLimit )
        break;

      double x, y, z;
      QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize,
                                           xOffset, QgsPointCloudAttribute::DataType::Int32,
                                           yOffset, QgsPointCloudAttribute::DataType::Int32,
                                           zOffset, QgsPointCloudAttribute::DataType::Int32,
                                           scale, offset,
                                           x, y, z );
      if ( ! mParent->mZRange.contains( z ) ||
           ! mParent->mExtent.contains( x, y ) )
      {
        ++pointsSkipped;
        continue;
      }
      const auto attributeMap = QgsPointCloudAttribute::getAttributeMap( ptr, i * recordSize, attributesCollection );
      mParent->mTransform->transformInPlace( x, y, z );
      handlePoint( x, y, z, attributeMap, pointsExported );
      ++pointsExported;
    }
    handleNode();
  }
  handleAll();
}

QgsPointCloudLayerExporter::ExporterBase::~ExporterBase()
{
//  delete mCt;
}

//
// ExporterMemory
//

QgsPointCloudLayerExporter::ExporterMemory::ExporterMemory( QgsPointCloudLayerExporter *exp )
{
  mParent = exp;
}

void QgsPointCloudLayerExporter::ExporterMemory::handlePoint( double x, double y, double z, const QVariantMap &map, const qint64 pointNumber )
{
  Q_UNUSED( pointNumber )

  QgsFeature feature;
  feature.setGeometry( QgsGeometry( new QgsPoint( x, y, z ) ) );
  QgsAttributes featureAttributes;
  for ( const QString &attribute : std::as_const( mParent->mRequestedAttributes ) )
  {
    const double val = map[ attribute ].toDouble();
    featureAttributes.append( val );
  }
  feature.setAttributes( featureAttributes );
  mFeatures.append( feature );
}

void QgsPointCloudLayerExporter::ExporterMemory::handleNode()
{
  QgsVectorLayer *vl = qgis::down_cast<QgsVectorLayer *>( mParent->mMemoryLayer );
  if ( vl )
  {
    if ( ! vl->dataProvider()->addFeatures( mFeatures ) )
    {
      mParent->setLastError( vl->dataProvider()->lastError() );
    }
  }
  mFeatures.clear();
}

void QgsPointCloudLayerExporter::ExporterMemory::handleAll()
{
}

//
// ExporterVector
//

QgsPointCloudLayerExporter::ExporterVector::ExporterVector( QgsPointCloudLayerExporter *exp )
{
  mParent = exp;
}

void QgsPointCloudLayerExporter::ExporterVector::handlePoint( double x, double y, double z, const QVariantMap &map, const qint64 pointNumber )
{
  Q_UNUSED( pointNumber )

  QgsFeature feature;
  feature.setGeometry( QgsGeometry( new QgsPoint( x, y, z ) ) );
  QgsAttributes featureAttributes;
  for ( const QString &attribute : std::as_const( mParent->mRequestedAttributes ) )
  {
    const double val = map[ attribute ].toDouble();
    featureAttributes.append( val );
  }
  feature.setAttributes( featureAttributes );
  mFeatures.append( feature );
}

void QgsPointCloudLayerExporter::ExporterVector::handleNode()
{
  if ( ! mParent->mVectorSink->addFeatures( mFeatures ) )
  {
    mParent->setLastError( mParent->mVectorSink->lastError() );
  }
  mFeatures.clear();
}

void QgsPointCloudLayerExporter::ExporterVector::handleAll()
{
  delete mParent->mVectorSink;
  mParent->mVectorSink = nullptr;
}

//
// ExporterPdal
//

#ifdef HAVE_PDAL_QGIS

QgsPointCloudLayerExporter::ExporterPdal::ExporterPdal( QgsPointCloudLayerExporter *exp )
{
  mParent = exp;


  mOptions.add( "filename", mParent->mFilename.toStdString() );
  mOptions.add( "a_srs", mParent->mTargetCrs.authid().toStdString() );
//  mOptions.add( "format", QString( mParent->mPointRecordFormat ).toStdString() );

  mTable.layout()->registerDim( pdal::Dimension::Id::X );
  mTable.layout()->registerDim( pdal::Dimension::Id::Y );
  mTable.layout()->registerDim( pdal::Dimension::Id::Z );

  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Intensity" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::Intensity );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "ReturnNumber" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::ReturnNumber );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "NumberOfReturns" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::NumberOfReturns );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "ScanDirectionFlag" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::ScanDirectionFlag );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "EdgeOfFlightLine" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::EdgeOfFlightLine );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Classification" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::Classification );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "ScanAngleRank" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::ScanAngleRank );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "UserData" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::UserData );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "PointSourceId" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::PointSourceId );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "ScannerChannel" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::ScanChannel );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Red" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::Red );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Green" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::Green );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Blue" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::Blue );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "GpsTime" ), Qt::CaseInsensitive ) )
    mTable.layout()->registerDim( pdal::Dimension::Id::GpsTime );

  mView.reset( new pdal::PointView( mTable ) );
}

void QgsPointCloudLayerExporter::ExporterPdal::handlePoint( double x, double y, double z, const QVariantMap &map, const qint64 pointNumber )
{
  mView->setField( pdal::Dimension::Id::X, pointNumber, x );
  mView->setField( pdal::Dimension::Id::Y, pointNumber, y );
  mView->setField( pdal::Dimension::Id::Z, pointNumber, z );


  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Intensity" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::Intensity, pointNumber, map[ QStringLiteral( "Intensity" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "ReturnNumber" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::ReturnNumber, pointNumber, map[ QStringLiteral( "ReturnNumber" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "NumberOfReturns" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::NumberOfReturns, pointNumber, map[ QStringLiteral( "NumberOfReturns" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "ScanDirectionFlag" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::ScanDirectionFlag, pointNumber, map[ QStringLiteral( "ScanDirectionFlag" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "EdgeOfFlightLine" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::EdgeOfFlightLine, pointNumber, map[ QStringLiteral( "EdgeOfFlightLine" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Classification" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::Classification, pointNumber, map[ QStringLiteral( "Classification" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "ScanAngleRank" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::ScanAngleRank, pointNumber, map[ QStringLiteral( "ScanAngleRank" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "UserData" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::UserData, pointNumber, map[ QStringLiteral( "UserData" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "PointSourceId" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::PointSourceId, pointNumber, map[ QStringLiteral( "PointSourceId" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "ScannerChannel" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::ScanChannel, pointNumber, map[ QStringLiteral( "ScannerChannel" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Red" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::Red, pointNumber, map[ QStringLiteral( "Red" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Green" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::Green, pointNumber, map[ QStringLiteral( "Green" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "Blue" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::Blue, pointNumber, map[ QStringLiteral( "Blue" ) ].toInt() );
  if ( mParent->mRequestedAttributes.contains( QLatin1String( "GpsTime" ), Qt::CaseInsensitive ) )
    mView->setField( pdal::Dimension::Id::GpsTime, pointNumber, map[ QStringLiteral( "GpsTime" ) ].toDouble() );

}

void QgsPointCloudLayerExporter::ExporterPdal::handleNode()
{

}

void QgsPointCloudLayerExporter::ExporterPdal::handleAll()
{
  pdal::BufferReader reader;
  reader.addView( mView );

  pdal::StageFactory factory;

  pdal::Stage *writer = factory.createStage( "writers.las" );

  writer->setInput( reader );
  writer->setOptions( mOptions );
  writer->prepare( mTable );
  writer->execute( mTable );
}
#endif

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

  mExp->doExport();

  return mError == Qgis::VectorExportResult::Success;
}

void QgsPointCloudLayerExporterTask::finished( bool result )
{
  Q_UNUSED( result )

  emit exportComplete();
  delete mExp;
}
