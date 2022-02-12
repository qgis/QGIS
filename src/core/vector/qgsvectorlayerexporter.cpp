/***************************************************************************
                          qgsvectorlayerexporter.cpp
                             -------------------
    begin                : Thu Aug 25 2011
    copyright            : (C) 2011 by Giuseppe Sucameli
    email                : brush.tyler at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsgeometrycollection.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerexporter.h"
#include "qgsproviderregistry.h"
#include "qgsdatasourceuri.h"
#include "qgsexception.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsabstractgeometry.h"
#include "qgscoordinatetransform.h"

#include <QProgressDialog>

typedef Qgis::VectorExportResult createEmptyLayer_t(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type geometryType,
  const QgsCoordinateReferenceSystem &destCRS,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdx,
  QString *errorMessage,
  const QMap<QString, QVariant> *options
);


QgsVectorLayerExporter::QgsVectorLayerExporter( const QString &uri,
    const QString &providerKey,
    const QgsFields &fields,
    QgsWkbTypes::Type geometryType,
    const QgsCoordinateReferenceSystem &crs,
    bool overwrite,
    const QMap<QString, QVariant> &options,
    QgsFeatureSink::SinkFlags sinkFlags )
  : mErrorCount( 0 )
  , mAttributeCount( -1 )

{
  mProvider = nullptr;

  QMap<QString, QVariant> modifiedOptions( options );

  if ( providerKey == QLatin1String( "ogr" ) &&
       options.contains( QStringLiteral( "driverName" ) ) &&
       ( options[ QStringLiteral( "driverName" ) ].toString().compare( QLatin1String( "GPKG" ), Qt::CaseInsensitive ) == 0 ||
         options[ QStringLiteral( "driverName" ) ].toString().compare( QLatin1String( "SQLite" ), Qt::CaseInsensitive ) == 0 ) )
  {
    if ( geometryType != QgsWkbTypes::NoGeometry )
    {
      // For GPKG/Spatialite, we explicitly ask not to create a spatial index at
      // layer creation since this would slow down inserts. Defer its creation
      // to end of exportLayer() or destruction of this object.
      QStringList modifiedLayerOptions;
      if ( options.contains( QStringLiteral( "layerOptions" ) ) )
      {
        const QStringList layerOptions = options.value( QStringLiteral( "layerOptions" ) ).toStringList();
        for ( const QString &layerOption : layerOptions )
        {
          if ( layerOption.compare( QLatin1String( "SPATIAL_INDEX=YES" ), Qt::CaseInsensitive ) == 0 ||
               layerOption.compare( QLatin1String( "SPATIAL_INDEX=ON" ), Qt::CaseInsensitive ) == 0 ||
               layerOption.compare( QLatin1String( "SPATIAL_INDEX=TRUE" ), Qt::CaseInsensitive ) == 0 ||
               layerOption.compare( QLatin1String( "SPATIAL_INDEX=1" ), Qt::CaseInsensitive ) == 0 )
          {
            // do nothing
          }
          else if ( layerOption.compare( QLatin1String( "SPATIAL_INDEX=NO" ), Qt::CaseInsensitive ) == 0 ||
                    layerOption.compare( QLatin1String( "SPATIAL_INDEX=OFF" ), Qt::CaseInsensitive ) == 0 ||
                    layerOption.compare( QLatin1String( "SPATIAL_INDEX=FALSE" ), Qt::CaseInsensitive ) == 0 ||
                    layerOption.compare( QLatin1String( "SPATIAL_INDEX=0" ), Qt::CaseInsensitive ) == 0 )
          {
            mCreateSpatialIndex = false;
          }
          else
          {
            modifiedLayerOptions << layerOption;
          }
        }
      }
      modifiedLayerOptions << QStringLiteral( "SPATIAL_INDEX=FALSE" );
      modifiedOptions[ QStringLiteral( "layerOptions" ) ] = modifiedLayerOptions;
    }
  }

  // create an empty layer
  QString errMsg;
  QgsProviderRegistry *pReg = QgsProviderRegistry::instance();
  mError = pReg->createEmptyLayer( providerKey, uri, fields, geometryType, crs, overwrite, mOldToNewAttrIdx,
                                   errMsg, !modifiedOptions.isEmpty() ? &modifiedOptions : nullptr );

  if ( errorCode() != Qgis::VectorExportResult::Success )
  {
    mErrorMessage = errMsg;
    return;
  }

  const auto constMOldToNewAttrIdx = mOldToNewAttrIdx;
  for ( const int idx : constMOldToNewAttrIdx )
  {
    if ( idx > mAttributeCount )
      mAttributeCount = idx;
  }

  mAttributeCount++;

  QgsDebugMsgLevel( QStringLiteral( "Created empty layer" ), 2 );

  QString uriUpdated( uri );
  // HACK sorry...
  if ( providerKey == QLatin1String( "ogr" ) )
  {
    QString layerName;
    if ( options.contains( QStringLiteral( "layerName" ) ) )
      layerName = options.value( QStringLiteral( "layerName" ) ).toString();
    if ( !layerName.isEmpty() )
    {
      uriUpdated += QLatin1String( "|layername=" );
      uriUpdated += layerName;
    }
  }

  // Oracle specific HACK: we cannot guess the geometry type when there is no rows, so we need
  // to force it in the uri
  if ( providerKey == QLatin1String( "oracle" ) )
  {
    uriUpdated += QStringLiteral( " type=%1" ).arg( QgsWkbTypes::displayString( geometryType ) );
  }

  const QgsDataProvider::ProviderOptions providerOptions;
  QgsVectorDataProvider *vectorProvider = qobject_cast< QgsVectorDataProvider * >( pReg->createProvider( providerKey, uriUpdated, providerOptions ) );
  if ( !vectorProvider || !vectorProvider->isValid() || ( vectorProvider->capabilities() & QgsVectorDataProvider::AddFeatures ) == 0 )
  {
    mError = Qgis::VectorExportResult::ErrorInvalidLayer;
    mErrorMessage = QObject::tr( "Loading of layer failed" );

    delete vectorProvider;
    return;
  }

  // If the result is a geopackage layer and there is already a field name FID requested which
  // might contain duplicates, make sure to generate a new field with a unique name instead
  // that will be filled by ogr with unique values.

  // HACK sorry
  const QString path = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), uri ).value( QStringLiteral( "path" ) ).toString();
  if ( sinkFlags.testFlag( QgsFeatureSink::SinkFlag::RegeneratePrimaryKey ) && path.endsWith( QLatin1String( ".gpkg" ), Qt::CaseInsensitive ) )
  {
    const QString fidName = options.value( QStringLiteral( "FID" ), QStringLiteral( "FID" ) ).toString();
    const int fidIdx = fields.lookupField( fidName );
    if ( fidIdx != -1 )
    {
      mOldToNewAttrIdx.remove( fidIdx );
    }
  }

  mProvider = vectorProvider;
  mError = Qgis::VectorExportResult::Success;
}

QgsVectorLayerExporter::~QgsVectorLayerExporter()
{
  flushBuffer();

  if ( mCreateSpatialIndex )
  {
    createSpatialIndex();
  }

  delete mProvider;
}

Qgis::VectorExportResult QgsVectorLayerExporter::errorCode() const
{
  return mError;
}

QString QgsVectorLayerExporter::errorMessage() const
{
  return mErrorMessage;
}

bool QgsVectorLayerExporter::addFeatures( QgsFeatureList &features, Flags flags )
{
  QgsFeatureList::iterator fIt = features.begin();
  bool result = true;
  for ( ; fIt != features.end(); ++fIt )
  {
    result = result && addFeature( *fIt, flags );
  }
  return result;
}

bool QgsVectorLayerExporter::addFeature( QgsFeature &feat, Flags )
{
  const QgsAttributes attrs = feat.attributes();

  QgsFeature newFeat;
  if ( feat.hasGeometry() )
    newFeat.setGeometry( feat.geometry() );

  newFeat.initAttributes( mAttributeCount );

  for ( int i = 0; i < attrs.count(); ++i )
  {
    // add only mapped attributes (un-mapped ones will not be present in the
    // destination layer)
    const int dstIdx = mOldToNewAttrIdx.value( i, -1 );
    if ( dstIdx < 0 )
      continue;

    QgsDebugMsgLevel( QStringLiteral( "moving field from pos %1 to %2" ).arg( i ).arg( dstIdx ), 3 );
    newFeat.setAttribute( dstIdx, attrs.at( i ) );
  }

  mFeatureBuffer.append( newFeat );
  mFeatureBufferMemoryUsage += newFeat.approximateMemoryUsage();

  if ( mFeatureBufferMemoryUsage >= 100 * 1000 * 1000 )
  {
    return flushBuffer();
  }

  return true;
}

QString QgsVectorLayerExporter::lastError() const
{
  return mErrorMessage;
}

bool QgsVectorLayerExporter::flushBuffer()
{
  mFeatureBufferMemoryUsage = 0;
  if ( mFeatureBuffer.count() <= 0 )
    return true;

  if ( !mProvider->addFeatures( mFeatureBuffer, QgsFeatureSink::FastInsert ) )
  {
    const QStringList errors = mProvider->errors();
    mProvider->clearErrors();

    mErrorMessage = QObject::tr( "Creation error for features from #%1 to #%2. Provider errors was: \n%3" )
                    .arg( mFeatureBuffer.first().id() )
                    .arg( mFeatureBuffer.last().id() )
                    .arg( errors.join( QLatin1Char( '\n' ) ) );

    mError = Qgis::VectorExportResult::ErrorFeatureWriteFailed;
    mErrorCount += mFeatureBuffer.count();

    mFeatureBuffer.clear();
    QgsDebugMsg( mErrorMessage );
    return false;
  }

  mFeatureBuffer.clear();
  return true;
}

bool QgsVectorLayerExporter::createSpatialIndex()
{
  mCreateSpatialIndex = false;
  if ( mProvider && ( mProvider->capabilities() & QgsVectorDataProvider::CreateSpatialIndex ) != 0 )
  {
    return mProvider->createSpatialIndex();
  }
  else
  {
    return true;
  }
}

Qgis::VectorExportResult QgsVectorLayerExporter::exportLayer( QgsVectorLayer *layer,
    const QString &uri,
    const QString &providerKey,
    const QgsCoordinateReferenceSystem &destCRS,
    bool onlySelected,
    QString *errorMessage,
    const QMap<QString, QVariant> &options,
    QgsFeedback *feedback )
{
  QgsCoordinateReferenceSystem outputCRS;
  QgsCoordinateTransform ct;
  bool shallTransform = false;

  if ( !layer )
    return Qgis::VectorExportResult::ErrorInvalidLayer;

  if ( destCRS.isValid() )
  {
    // This means we should transform
    outputCRS = destCRS;
    shallTransform = true;
  }
  else
  {
    // This means we shouldn't transform, use source CRS as output (if defined)
    outputCRS = layer->crs();
  }


  bool overwrite = false;
  bool forceSinglePartGeom = false;
  QMap<QString, QVariant> providerOptions = options;
  if ( !options.isEmpty() )
  {
    overwrite = providerOptions.take( QStringLiteral( "overwrite" ) ).toBool();
    forceSinglePartGeom = providerOptions.take( QStringLiteral( "forceSinglePartGeometryType" ) ).toBool();
  }

  QgsFields fields = layer->fields();

  QgsWkbTypes::Type wkbType = layer->wkbType();

  // Special handling for Shapefiles
  if ( layer->providerType() == QLatin1String( "ogr" ) && layer->storageType() == QLatin1String( "ESRI Shapefile" ) )
  {
    // convert field names to lowercase
    for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
    {
      fields.rename( fldIdx, fields.at( fldIdx ).name().toLower() );
    }
  }

  bool convertGeometryToSinglePart = false;
  if ( forceSinglePartGeom && QgsWkbTypes::isMultiType( wkbType ) )
  {
    wkbType = QgsWkbTypes::singleType( wkbType );
    convertGeometryToSinglePart = true;
  }

  QgsVectorLayerExporter *writer =
    new QgsVectorLayerExporter( uri, providerKey, fields, wkbType, outputCRS, overwrite, providerOptions );

  // check whether file creation was successful
  const Qgis::VectorExportResult err = writer->errorCode();
  if ( err != Qgis::VectorExportResult::Success )
  {
    if ( errorMessage )
      *errorMessage = writer->errorMessage();
    delete writer;
    return err;
  }

  if ( errorMessage )
  {
    errorMessage->clear();
  }

  QgsFeature fet;

  QgsFeatureRequest req;
  if ( wkbType == QgsWkbTypes::NoGeometry )
    req.setFlags( QgsFeatureRequest::NoGeometry );
  if ( onlySelected )
    req.setFilterFids( layer->selectedFeatureIds() );

  QgsFeatureIterator fit = layer->getFeatures( req );

  // Create our transform
  if ( destCRS.isValid() )
  {
    ct = QgsCoordinateTransform( layer->crs(), destCRS, layer->transformContext() );
  }

  // Check for failure
  if ( !ct.isValid() )
    shallTransform = false;

  long long n = 0;
  const long long approxTotal = onlySelected ? layer->selectedFeatureCount() : layer->featureCount();

  if ( errorMessage )
  {
    *errorMessage = QObject::tr( "Feature write errors:" );
  }

  bool canceled = false;

  // write all features
  while ( fit.nextFeature( fet ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      canceled = true;
      if ( errorMessage )
      {
        *errorMessage += '\n' + QObject::tr( "Import was canceled at %1 of %2" ).arg( n ).arg( approxTotal );
      }
      break;
    }

    if ( writer->errorCount() > 1000 )
    {
      if ( errorMessage )
      {
        *errorMessage += '\n' + QObject::tr( "Stopping after %n error(s)", nullptr, writer->errorCount() );
      }
      break;
    }

    if ( shallTransform )
    {
      try
      {
        if ( fet.hasGeometry() )
        {
          QgsGeometry g = fet.geometry();
          g.transform( ct );
          fet.setGeometry( g );
        }
      }
      catch ( QgsCsException &e )
      {
        delete writer;

        const QString msg = QObject::tr( "Failed to transform a point while drawing a feature with ID '%1'. Writing stopped. (Exception: %2)" )
                            .arg( fet.id() ).arg( e.what() );
        QgsMessageLog::logMessage( msg, QObject::tr( "Vector import" ) );
        if ( errorMessage )
          *errorMessage += '\n' + msg;

        return Qgis::VectorExportResult::ErrorProjectingFeatures;
      }
    }

    // Handles conversion to single-part
    if ( convertGeometryToSinglePart && fet.geometry().isMultipart() )
    {
      QgsGeometry singlePartGeometry { fet.geometry() };
      // We want a failure if the geometry cannot be converted to single-part without data loss!
      // check if there are more than one part
      const QgsGeometryCollection *c = qgsgeometry_cast<const QgsGeometryCollection *>( singlePartGeometry.constGet() );
      if ( ( c && c->partCount() > 1 ) || ! singlePartGeometry.convertToSingleType() )
      {
        delete writer;
        const QString msg = QObject::tr( "Failed to transform a feature with ID '%1' to single part. Writing stopped." )
                            .arg( fet.id() );
        QgsMessageLog::logMessage( msg, QObject::tr( "Vector import" ) );
        if ( errorMessage )
          *errorMessage += '\n' + msg;
        return Qgis::VectorExportResult::ErrorFeatureWriteFailed;
      }
      fet.setGeometry( singlePartGeometry );
    }

    if ( !writer->addFeature( fet ) )
    {
      if ( writer->errorCode() != Qgis::VectorExportResult::Success && errorMessage )
      {
        *errorMessage += '\n' + writer->errorMessage();
      }
    }
    n++;

    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( n ) / approxTotal );
    }

  }

  // flush the buffer to be sure that all features are written
  if ( !writer->flushBuffer() )
  {
    if ( writer->errorCode() != Qgis::VectorExportResult::Success && errorMessage )
    {
      *errorMessage += '\n' + writer->errorMessage();
    }
  }
  const int errors = writer->errorCount();

  if ( writer->mCreateSpatialIndex && !writer->createSpatialIndex() )
  {
    if ( writer->errorCode() != Qgis::VectorExportResult::Success && errorMessage )
    {
      *errorMessage += '\n' + writer->errorMessage();
    }
  }

  delete writer;

  if ( errorMessage )
  {
    if ( errors > 0 )
    {
      *errorMessage += '\n' + QObject::tr( "Only %1 of %2 features written." ).arg( n - errors ).arg( n );
    }
    else
    {
      errorMessage->clear();
    }
  }

  if ( canceled )
    return Qgis::VectorExportResult::UserCanceled;
  else if ( errors > 0 )
    return Qgis::VectorExportResult::ErrorFeatureWriteFailed;

  return Qgis::VectorExportResult::Success;
}


//
// QgsVectorLayerExporterTask
//

QgsVectorLayerExporterTask::QgsVectorLayerExporterTask( QgsVectorLayer *layer, const QString &uri, const QString &providerKey, const QgsCoordinateReferenceSystem &destinationCrs, const QMap<QString, QVariant> &options, bool ownsLayer )
  : QgsTask( tr( "Exporting %1" ).arg( layer->name() ), QgsTask::CanCancel )
  , mLayer( layer )
  , mOwnsLayer( ownsLayer )
  , mDestUri( uri )
  , mDestProviderKey( providerKey )
  , mDestCrs( destinationCrs )
  , mOptions( options )
  , mOwnedFeedback( new QgsFeedback() )
{
  if ( mLayer )
    setDependentLayers( QList< QgsMapLayer * >() << mLayer );
}

QgsVectorLayerExporterTask *QgsVectorLayerExporterTask::withLayerOwnership( QgsVectorLayer *layer, const QString &uri, const QString &providerKey, const QgsCoordinateReferenceSystem &destinationCrs, const QMap<QString, QVariant> &options )
{
  std::unique_ptr< QgsVectorLayerExporterTask > newTask( new QgsVectorLayerExporterTask( layer, uri, providerKey, destinationCrs, options ) );
  newTask->mOwnsLayer = true;
  return newTask.release();
}

void QgsVectorLayerExporterTask::cancel()
{
  mOwnedFeedback->cancel();
  QgsTask::cancel();
}

bool QgsVectorLayerExporterTask::run()
{
  if ( !mLayer )
    return false;

  connect( mOwnedFeedback.get(), &QgsFeedback::progressChanged, this, &QgsVectorLayerExporterTask::setProgress );


  mError = QgsVectorLayerExporter::exportLayer(
             mLayer.data(), mDestUri, mDestProviderKey, mDestCrs, false, &mErrorMessage,
             mOptions, mOwnedFeedback.get() );

  return mError == Qgis::VectorExportResult::Success;
}

void QgsVectorLayerExporterTask::finished( bool result )
{
  // QgsMapLayer has QTimer member, which must not be destroyed from another thread
  if ( mOwnsLayer )
    delete mLayer;

  if ( result )
    emit exportComplete();
  else
    emit errorOccurred( mError, mErrorMessage );
}
