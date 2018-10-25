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
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerexporter.h"
#include "qgsproviderregistry.h"
#include "qgsdatasourceuri.h"
#include "qgsexception.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QProgressDialog>

#define FEATURE_BUFFER_SIZE 200

typedef QgsVectorLayerExporter::ExportError createEmptyLayer_t(
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
    const QMap<QString, QVariant> &options )
  : mErrorCount( 0 )
  , mAttributeCount( -1 )

{
  mProvider = nullptr;

  QgsProviderRegistry *pReg = QgsProviderRegistry::instance();

  std::unique_ptr< QLibrary > myLib( pReg->createProviderLibrary( providerKey ) );
  if ( !myLib )
  {
    mError = ErrInvalidProvider;
    mErrorMessage = QObject::tr( "Unable to load %1 provider" ).arg( providerKey );
    return;
  }

  createEmptyLayer_t *pCreateEmpty = reinterpret_cast< createEmptyLayer_t * >( cast_to_fptr( myLib->resolve( "createEmptyLayer" ) ) );
  if ( !pCreateEmpty )
  {
    mError = ErrProviderUnsupportedFeature;
    mErrorMessage = QObject::tr( "Provider %1 has no %2 method" ).arg( providerKey, QStringLiteral( "createEmptyLayer" ) );
    return;
  }

  // create an empty layer
  QString errMsg;
  mError = pCreateEmpty( uri, fields, geometryType, crs, overwrite, &mOldToNewAttrIdx, &errMsg, !options.isEmpty() ? &options : nullptr );
  if ( errorCode() )
  {
    mErrorMessage = errMsg;
    return;
  }

  Q_FOREACH ( int idx, mOldToNewAttrIdx )
  {
    if ( idx > mAttributeCount )
      mAttributeCount = idx;
  }

  mAttributeCount++;

  QgsDebugMsg( QStringLiteral( "Created empty layer" ) );

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

  QgsDataProvider::ProviderOptions providerOptions;
  QgsVectorDataProvider *vectorProvider = dynamic_cast< QgsVectorDataProvider * >( pReg->createProvider( providerKey, uriUpdated, providerOptions ) );
  if ( !vectorProvider || !vectorProvider->isValid() || ( vectorProvider->capabilities() & QgsVectorDataProvider::AddFeatures ) == 0 )
  {
    mError = ErrInvalidLayer;
    mErrorMessage = QObject::tr( "Loading of layer failed" );

    delete vectorProvider;
    return;
  }

  mProvider = vectorProvider;
  mError = NoError;
}

QgsVectorLayerExporter::~QgsVectorLayerExporter()
{
  flushBuffer();
  delete mProvider;
}

QgsVectorLayerExporter::ExportError QgsVectorLayerExporter::errorCode() const
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
  QgsAttributes attrs = feat.attributes();

  QgsFeature newFeat;
  if ( feat.hasGeometry() )
    newFeat.setGeometry( feat.geometry() );

  newFeat.initAttributes( mAttributeCount );

  for ( int i = 0; i < attrs.count(); ++i )
  {
    // add only mapped attributes (un-mapped ones will not be present in the
    // destination layer)
    int dstIdx = mOldToNewAttrIdx.value( i, -1 );
    if ( dstIdx < 0 )
      continue;

    QgsDebugMsgLevel( QStringLiteral( "moving field from pos %1 to %2" ).arg( i ).arg( dstIdx ), 3 );
    newFeat.setAttribute( dstIdx, attrs.at( i ) );
  }

  mFeatureBuffer.append( newFeat );

  if ( mFeatureBuffer.count() >= FEATURE_BUFFER_SIZE )
  {
    return flushBuffer();
  }

  return true;
}

bool QgsVectorLayerExporter::flushBuffer()
{
  if ( mFeatureBuffer.count() <= 0 )
    return true;

  if ( !mProvider->addFeatures( mFeatureBuffer, QgsFeatureSink::FastInsert ) )
  {
    QStringList errors = mProvider->errors();
    mProvider->clearErrors();

    mErrorMessage = QObject::tr( "Creation error for features from #%1 to #%2. Provider errors was: \n%3" )
                    .arg( mFeatureBuffer.first().id() )
                    .arg( mFeatureBuffer.last().id() )
                    .arg( errors.join( QStringLiteral( "\n" ) ) );

    mError = ErrFeatureWriteFailed;
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
  if ( mProvider && ( mProvider->capabilities() & QgsVectorDataProvider::CreateSpatialIndex ) != 0 )
  {
    return mProvider->createSpatialIndex();
  }
  else
  {
    return true;
  }
}

QgsVectorLayerExporter::ExportError
QgsVectorLayerExporter::exportLayer( QgsVectorLayer *layer,
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
    return ErrInvalidLayer;

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
      fields[fldIdx].setName( fields.at( fldIdx ).name().toLower() );
    }

    if ( !forceSinglePartGeom )
    {
      // convert wkbtype to multipart (see #5547)
      switch ( wkbType )
      {
        case QgsWkbTypes::Point:
          wkbType = QgsWkbTypes::MultiPoint;
          break;
        case QgsWkbTypes::LineString:
          wkbType = QgsWkbTypes::MultiLineString;
          break;
        case QgsWkbTypes::Polygon:
          wkbType = QgsWkbTypes::MultiPolygon;
          break;
        case QgsWkbTypes::Point25D:
          wkbType = QgsWkbTypes::MultiPoint25D;
          break;
        case QgsWkbTypes::LineString25D:
          wkbType = QgsWkbTypes::MultiLineString25D;
          break;
        case QgsWkbTypes::Polygon25D:
          wkbType = QgsWkbTypes::MultiPolygon25D;
          break;
        default:
          break;
      }
    }
  }

  QgsVectorLayerExporter *writer =
    new QgsVectorLayerExporter( uri, providerKey, fields, wkbType, outputCRS, overwrite, providerOptions );

  // check whether file creation was successful
  ExportError err = writer->errorCode();
  if ( err != NoError )
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
    Q_NOWARN_DEPRECATED_PUSH
    ct = QgsCoordinateTransform( layer->crs(), destCRS );
    Q_NOWARN_DEPRECATED_POP
  }

  // Check for failure
  if ( !ct.isValid() )
    shallTransform = false;

  long n = 0;
  long approxTotal = onlySelected ? layer->selectedFeatureCount() : layer->featureCount();

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
        *errorMessage += '\n' + QObject::tr( "Stopping after %1 errors" ).arg( writer->errorCount() );
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

        QString msg = QObject::tr( "Failed to transform a point while drawing a feature with ID '%1'. Writing stopped. (Exception: %2)" )
                      .arg( fet.id() ).arg( e.what() );
        QgsMessageLog::logMessage( msg, QObject::tr( "Vector import" ) );
        if ( errorMessage )
          *errorMessage += '\n' + msg;

        return ErrProjection;
      }
    }
    if ( !writer->addFeature( fet ) )
    {
      if ( writer->errorCode() && errorMessage )
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
    if ( writer->errorCode() && errorMessage )
    {
      *errorMessage += '\n' + writer->errorMessage();
    }
  }
  int errors = writer->errorCount();

  if ( !writer->createSpatialIndex() )
  {
    if ( writer->errorCode() && errorMessage )
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
    return ErrUserCanceled;
  else if ( errors > 0 )
    return ErrFeatureWriteFailed;

  return NoError;
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

  return mError == QgsVectorLayerExporter::NoError;
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
