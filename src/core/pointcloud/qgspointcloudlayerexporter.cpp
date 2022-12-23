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
#include <QApplication>
#include <QThread>

#include "qgspointcloudlayerexporter.h"
#include "qgsmemoryproviderutils.h"
#include "qgspointcloudrequest.h"
#include "qgsvectorfilewriter.h"
#include "qgsgeos.h"

#ifdef HAVE_PDAL_QGIS
#include <pdal/StageFactory.hpp>
#include <pdal/io/BufferReader.hpp>
#include <pdal/Dimension.hpp>
#endif

QString QgsPointCloudLayerExporter::getOgrDriverName( ExportFormat format )
{
  switch ( format )
  {
    case ExportFormat::Gpkg:
      return QStringLiteral( "GPKG" );
    case ExportFormat::Dxf:
      return QStringLiteral( "DXF" );
    case ExportFormat::Shp:
      return QStringLiteral( "ESRI Shapefile" );
    case ExportFormat::Csv:
      return QStringLiteral( "CSV" );
    case ExportFormat::Memory:
    case ExportFormat::Las:
      break;
  }
  return QString();
}

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

  setAllAttributes();
}

QgsPointCloudLayerExporter::~QgsPointCloudLayerExporter()
{
  delete mMemoryLayer;
  delete mVectorSink;
  delete mTransform;
}

bool QgsPointCloudLayerExporter::setFormat( const ExportFormat format )
{
  if ( supportedFormats().contains( format ) )
  {
    mFormat = format;
    return true;
  }
  return false;
}

void QgsPointCloudLayerExporter::setFilterGeometry( const QgsAbstractGeometry *geometry )
{
  mFilterGeometryEngine.reset( new QgsGeos( geometry ) );
  mFilterGeometryEngine->prepareGeometry();
}

void QgsPointCloudLayerExporter::setFilterGeometry( QgsMapLayer *layer, bool selectedFeaturesOnly )
{
  QgsVectorLayer *vlayer = dynamic_cast< QgsVectorLayer * >( layer );
  if ( !vlayer )
    return;

  QVector< QgsGeometry > allGeometries;
  QgsFeatureIterator fit;
  const QgsFeatureRequest request = QgsFeatureRequest( mExtent ).setNoAttributes();
  if ( selectedFeaturesOnly )
    fit = vlayer->getSelectedFeatures( request );
  else
    fit = vlayer->getFeatures( request );

  QgsCoordinateTransform transform( vlayer->crs(), mSourceCrs, mTransformContext );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    if ( f.hasGeometry() )
    {
      allGeometries.append( f.geometry() );
    }
  }
  QgsGeometry unaryUnion = QgsGeometry::unaryUnion( allGeometries );
  try
  {
    unaryUnion.transform( transform );
  }
  catch ( const QgsCsException &cse )
  {
    QgsDebugMsg( QStringLiteral( "Error transforming union of filter layer: %1" ).arg( cse.what() ) );
    QgsDebugMsg( QStringLiteral( "FilterGeometry will be ignored." ) );
    return;
  }
  setFilterGeometry( unaryUnion.constGet() );
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

void QgsPointCloudLayerExporter::setAllAttributes()
{
  QStringList allAttributeNames;
  const QVector<QgsPointCloudAttribute> allAttributes = mLayerAttributeCollection.attributes();
  for ( const QgsPointCloudAttribute &attribute : allAttributes )
  {
    allAttributeNames.append( attribute.name() );
  }
  setAttributes( allAttributeNames );
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

void QgsPointCloudLayerExporter::prepareExport()
{
  delete mMemoryLayer;
  mMemoryLayer = nullptr;

  if ( mFormat == ExportFormat::Memory )
  {
    if ( QApplication::instance()->thread() != QThread::currentThread() )
      QgsDebugMsgLevel( QStringLiteral( "prepareExport() should better be called from the main thread!" ), 2 );

    mMemoryLayer = QgsMemoryProviderUtils::createMemoryLayer( mName, outputFields(), QgsWkbTypes::PointZ, mTargetCrs );
  }
}

void QgsPointCloudLayerExporter::doExport()
{
  mTransform = new QgsCoordinateTransform( mSourceCrs, mTargetCrs, mTransformContext );
  if ( mExtent.isFinite() )
  {
    try
    {
      mExtent = mTransform->transformBoundingBox( mExtent, Qgis::TransformDirection::Reverse );
    }
    catch ( const QgsCsException &cse )
    {
      QgsDebugMsg( QStringLiteral( "Error transforming extent: %1" ).arg( cse.what() ) );
    }
  }

  QStringList layerCreationOptions;

  switch ( mFormat )
  {
    case ExportFormat::Memory:
    {
      if ( !mMemoryLayer )
        prepareExport();

      ExporterMemory exp( this );
      exp.run();
      break;
    }

    case ExportFormat::Las:
    {
#ifdef HAVE_PDAL_QGIS
      setAllAttributes();
      // PDAL may throw exceptions
      try
      {
        ExporterPdal exp( this );
        exp.run();
      }
      catch ( std::runtime_error &e )
      {
        setLastError( QString::fromLatin1( e.what() ) );
        QgsDebugMsg( QStringLiteral( "PDAL has thrown an exception: {}" ).arg( e.what() ) );
      }
#endif
      break;
    }

    case ExportFormat::Csv:
      layerCreationOptions << QStringLiteral( "GEOMETRY=AS_XYZ" )
                           << QStringLiteral( "SEPARATOR=COMMA" ); // just in case ogr changes the default lco
      FALLTHROUGH
    case ExportFormat::Gpkg:
    case ExportFormat::Dxf:
    case ExportFormat::Shp:
    {
      const QString ogrDriver = getOgrDriverName( mFormat );
      QgsVectorFileWriter::SaveVectorOptions saveOptions;
      saveOptions.layerName = mName;
      saveOptions.driverName = ogrDriver;
      saveOptions.datasourceOptions = QgsVectorFileWriter::defaultDatasetOptions( ogrDriver );
      saveOptions.layerOptions = QgsVectorFileWriter::defaultLayerOptions( ogrDriver );
      saveOptions.layerOptions << layerCreationOptions;
      saveOptions.symbologyExport = QgsVectorFileWriter::NoSymbology;
      saveOptions.actionOnExistingFile = mActionOnExistingFile;
      saveOptions.feedback = mFeedback;
      mVectorSink = QgsVectorFileWriter::create( mFilename, outputFields(), QgsWkbTypes::PointZ, mTargetCrs, QgsCoordinateTransformContext(), saveOptions );
      ExporterVector exp( this );
      exp.run();
      return;
    }
  }
}

QgsMapLayer *QgsPointCloudLayerExporter::takeExportedLayer()
{
  switch ( mFormat )
  {
    case ExportFormat::Memory:
    {
      QgsMapLayer *retVal = mMemoryLayer;
      mMemoryLayer = nullptr;
      return retVal;
    }

    case ExportFormat::Las:
    {
      const QFileInfo fileInfo( mFilename );
      return new QgsPointCloudLayer( mFilename, fileInfo.completeBaseName(), QStringLiteral( "pdal" ) );
    }

    case ExportFormat::Gpkg:
    {
      QString uri( mFilename );
      uri += "|layername=" + mName;
      return new QgsVectorLayer( uri, mName, QStringLiteral( "ogr" ) );
    }

    case ExportFormat::Dxf:
    case ExportFormat::Shp:
    case ExportFormat::Csv:
    {
      const QFileInfo fileInfo( mFilename );
      return new QgsVectorLayer( mFilename, fileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
    }
  }
  BUILTIN_UNREACHABLE
}

//
// ExporterBase
//

void QgsPointCloudLayerExporter::ExporterBase::run()
{
  QgsRectangle geometryFilterRectangle( -std::numeric_limits<double>::infinity(),
                                        -std::numeric_limits<double>::infinity(),
                                        std::numeric_limits<double>::infinity(),
                                        std::numeric_limits<double>::infinity(),
                                        false );
  if ( mParent->mFilterGeometryEngine )
  {
    const QgsAbstractGeometry *envelope = mParent->mFilterGeometryEngine->envelope();
    if ( envelope )
      geometryFilterRectangle = envelope->boundingBox();
  }

  QVector<IndexedPointCloudNode> nodes;
  qint64 pointCount = 0;
  QQueue<IndexedPointCloudNode> queue;
  queue.push_back( mParent->mIndex->root() );
  while ( !queue.empty() )
  {
    IndexedPointCloudNode node = queue.front();
    queue.pop_front();
    const QgsRectangle nodeExtent = mParent->mIndex->nodeMapExtent( node );
    if ( mParent->mExtent.intersects( nodeExtent ) &&
         mParent->mZRange.overlaps( mParent->mIndex->nodeZRange( node ) ) &&
         geometryFilterRectangle.intersects( nodeExtent ) )
    {
      pointCount += mParent->mIndex->nodePointCount( node );
      nodes.push_back( node );
    }
    for ( const IndexedPointCloudNode &child : mParent->mIndex->nodeChildren( node ) )
    {
      queue.push_back( child );
    }
  }



  int pointsSkipped = 0;
  const qint64 pointsToExport = mParent->mPointsLimit > 0 ? std::min( mParent->mPointsLimit, pointCount ) : pointCount;
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
    int xOffset = 0, yOffset = 0, zOffset = 0;
    const QgsPointCloudAttribute::DataType xType = attributesCollection.find( QStringLiteral( "X" ), xOffset )->type();
    const QgsPointCloudAttribute::DataType yType = attributesCollection.find( QStringLiteral( "Y" ), yOffset )->type();
    const QgsPointCloudAttribute::DataType zType = attributesCollection.find( QStringLiteral( "Z" ), zOffset )->type();
    for ( int i = 0; i < count; ++i )
    {

      if ( mParent->mFeedback &&
           i % 1000 == 0 )
      {
        mParent->mFeedback->setProgress( 100 * static_cast< float >( pointsExported ) / pointsToExport );
        if ( mParent->mFeedback->isCanceled() )
        {
          mParent->setLastError( QObject::tr( "Canceled by user" ) );
          return;
        }
      }

      if ( pointsExported >= pointsToExport )
        break;

      double x, y, z;
      QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize,
                                           xOffset, xType,
                                           yOffset, yType,
                                           zOffset, zType,
                                           scale, offset,
                                           x, y, z );
      if ( ! mParent->mZRange.contains( z ) ||
           ! mParent->mExtent.contains( x, y ) ||
           ( mParent->mFilterGeometryEngine && ! mParent->mFilterGeometryEngine->contains( x, y ) ) )
      {
        ++pointsSkipped;
        continue;
      }

      try
      {
        mParent->mTransform->transformInPlace( x, y, z );
        const QVariantMap attributeMap = QgsPointCloudAttribute::getAttributeMap( ptr, i * recordSize, attributesCollection );
        handlePoint( x, y, z, attributeMap, pointsExported );
        ++pointsExported;
      }
      catch ( const QgsCsException &cse )
      {
        QgsDebugMsg( QStringLiteral( "Error transforming point: %1" ).arg( cse.what() ) );
        ++pointsSkipped;
      }
    }
    handleNode();
  }
  handleAll();
}

//
// ExporterMemory
//

QgsPointCloudLayerExporter::ExporterMemory::ExporterMemory( QgsPointCloudLayerExporter *exp )
{
  mParent = exp;
}

QgsPointCloudLayerExporter::ExporterMemory::~ExporterMemory()
{
  mParent->mMemoryLayer->moveToThread( QApplication::instance()->thread() );
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

QgsPointCloudLayerExporter::ExporterVector::~ExporterVector()
{
  delete mParent->mVectorSink;
  mParent->mVectorSink = nullptr;
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

}

//
// ExporterPdal
//

#ifdef HAVE_PDAL_QGIS

QgsPointCloudLayerExporter::ExporterPdal::ExporterPdal( QgsPointCloudLayerExporter *exp )
  : mPointFormat( exp->mPointRecordFormat )
{
  mParent = exp;

  mOptions.add( "filename", mParent->mFilename.toStdString() );
  mOptions.add( "a_srs", mParent->mTargetCrs.toWkt().toStdString() );
  mOptions.add( "minor_version", QStringLiteral( "4" ).toStdString() ); // delault to LAZ 1.4 to properly handle pdrf >= 6
  mOptions.add( "format", QString::number( mPointFormat ).toStdString() );
  if ( mParent->mTransform->isShortCircuited() )
  {
    mOptions.add( "offset_x", QString::number( mParent->mIndex->offset().x() ).toStdString() );
    mOptions.add( "offset_y", QString::number( mParent->mIndex->offset().y() ).toStdString() );
    mOptions.add( "offset_z", QString::number( mParent->mIndex->offset().z() ).toStdString() );
    mOptions.add( "scale_x", QString::number( mParent->mIndex->scale().x() ).toStdString() );
    mOptions.add( "scale_y", QString::number( mParent->mIndex->scale().y() ).toStdString() );
    mOptions.add( "scale_z", QString::number( mParent->mIndex->scale().z() ).toStdString() );
  }

  mTable.layout()->registerDim( pdal::Dimension::Id::X );
  mTable.layout()->registerDim( pdal::Dimension::Id::Y );
  mTable.layout()->registerDim( pdal::Dimension::Id::Z );

  mTable.layout()->registerDim( pdal::Dimension::Id::Classification );
  mTable.layout()->registerDim( pdal::Dimension::Id::Intensity );
  mTable.layout()->registerDim( pdal::Dimension::Id::ReturnNumber );
  mTable.layout()->registerDim( pdal::Dimension::Id::NumberOfReturns );
  mTable.layout()->registerDim( pdal::Dimension::Id::ScanDirectionFlag );
  mTable.layout()->registerDim( pdal::Dimension::Id::EdgeOfFlightLine );
  mTable.layout()->registerDim( pdal::Dimension::Id::ScanAngleRank );
  mTable.layout()->registerDim( pdal::Dimension::Id::UserData );
  mTable.layout()->registerDim( pdal::Dimension::Id::PointSourceId );

  if ( mPointFormat == 6 || mPointFormat == 7 || mPointFormat == 8 || mPointFormat == 9 || mPointFormat == 10 )
  {
    mTable.layout()->registerDim( pdal::Dimension::Id::ScanChannel );
    mTable.layout()->registerDim( pdal::Dimension::Id::ClassFlags );
  }

  if ( mPointFormat != 0 && mPointFormat != 2 )
  {
    mTable.layout()->registerDim( pdal::Dimension::Id::GpsTime );
  }

  if ( mPointFormat == 2 || mPointFormat == 3 || mPointFormat == 5 || mPointFormat == 7 || mPointFormat == 8 || mPointFormat == 10 )
  {
    mTable.layout()->registerDim( pdal::Dimension::Id::Red );
    mTable.layout()->registerDim( pdal::Dimension::Id::Green );
    mTable.layout()->registerDim( pdal::Dimension::Id::Blue );
  }

  if ( mPointFormat == 8 || mPointFormat == 10 )
  {
    mTable.layout()->registerDim( pdal::Dimension::Id::Infrared );
  }

  mView.reset( new pdal::PointView( mTable ) );
}

void QgsPointCloudLayerExporter::ExporterPdal::handlePoint( double x, double y, double z, const QVariantMap &map, const qint64 pointNumber )
{
  mView->setField( pdal::Dimension::Id::X, pointNumber, x );
  mView->setField( pdal::Dimension::Id::Y, pointNumber, y );
  mView->setField( pdal::Dimension::Id::Z, pointNumber, z );


  mView->setField( pdal::Dimension::Id::Classification, pointNumber, map[ QStringLiteral( "Classification" ) ].toInt() );
  mView->setField( pdal::Dimension::Id::Intensity, pointNumber, map[ QStringLiteral( "Intensity" ) ].toInt() );
  mView->setField( pdal::Dimension::Id::ReturnNumber, pointNumber, map[ QStringLiteral( "ReturnNumber" ) ].toInt() );
  mView->setField( pdal::Dimension::Id::NumberOfReturns, pointNumber, map[ QStringLiteral( "NumberOfReturns" ) ].toInt() );
  mView->setField( pdal::Dimension::Id::ScanDirectionFlag, pointNumber, map[ QStringLiteral( "ScanDirectionFlag" ) ].toInt() );
  mView->setField( pdal::Dimension::Id::EdgeOfFlightLine, pointNumber, map[ QStringLiteral( "EdgeOfFlightLine" ) ].toInt() );
  mView->setField( pdal::Dimension::Id::ScanAngleRank, pointNumber, map[ QStringLiteral( "ScanAngleRank" ) ].toInt() );
  mView->setField( pdal::Dimension::Id::UserData, pointNumber, map[ QStringLiteral( "UserData" ) ].toInt() );
  mView->setField( pdal::Dimension::Id::PointSourceId, pointNumber, map[ QStringLiteral( "PointSourceId" ) ].toInt() );

  if ( mPointFormat == 6 || mPointFormat == 7 || mPointFormat == 8 || mPointFormat == 9 || mPointFormat == 10 )
  {
    mView->setField( pdal::Dimension::Id::ScanChannel, pointNumber, map[ QStringLiteral( "ScannerChannel" ) ].toInt() );
    mView->setField( pdal::Dimension::Id::ClassFlags, pointNumber, map[ QStringLiteral( "ClassificationFlags" ) ].toInt() );
  }

  if ( mPointFormat != 0 && mPointFormat != 2 )
  {
    mView->setField( pdal::Dimension::Id::GpsTime, pointNumber, map[ QStringLiteral( "GpsTime" ) ].toDouble() );
  }

  if ( mPointFormat == 2 || mPointFormat == 3 || mPointFormat == 5 || mPointFormat == 7 || mPointFormat == 8 || mPointFormat == 10 )
  {
    mView->setField( pdal::Dimension::Id::Red, pointNumber, map[ QStringLiteral( "Red" ) ].toInt() );
    mView->setField( pdal::Dimension::Id::Green, pointNumber, map[ QStringLiteral( "Green" ) ].toInt() );
    mView->setField( pdal::Dimension::Id::Blue, pointNumber, map[ QStringLiteral( "Blue" ) ].toInt() );
  }

  if ( mPointFormat == 8 || mPointFormat == 10 )
  {
    mView->setField( pdal::Dimension::Id::Infrared, pointNumber, map[ QStringLiteral( "Infrared" ) ].toInt() );
  }
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
  : QgsTask( tr( "Exporting point cloud" ), QgsTask::CanCancel )
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

  return true;
}

void QgsPointCloudLayerExporterTask::finished( bool result )
{
  Q_UNUSED( result )

  emit exportComplete();
  delete mExp;
}
