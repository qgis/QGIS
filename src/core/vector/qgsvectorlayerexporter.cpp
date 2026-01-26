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


#include "qgsvectorlayerexporter.h"

#include "qgsabstractgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollection.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QProgressDialog>
#include <QThread>

#include "moc_qgsvectorlayerexporter.cpp"

typedef Qgis::VectorExportResult createEmptyLayer_t(
  const QString &uri,
  const QgsFields &fields,
  Qgis::WkbType geometryType,
  const QgsCoordinateReferenceSystem &destCRS,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdx,
  QString *errorMessage,
  const QMap<QString, QVariant> *options
);


//
// QgsVectorLayerExporter::ExportOptions
//

void QgsVectorLayerExporter::ExportOptions::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
}

QgsCoordinateTransformContext QgsVectorLayerExporter::ExportOptions::transformContext() const
{
  return mTransformContext;
}

void QgsVectorLayerExporter::ExportOptions::setDestinationCrs( const QgsCoordinateReferenceSystem &crs )
{
  mDestinationCrs = crs;
}

QgsCoordinateReferenceSystem QgsVectorLayerExporter::ExportOptions::destinationCrs() const
{
  return mDestinationCrs;
}

void QgsVectorLayerExporter::ExportOptions::setExtent( const QgsReferencedRectangle &extent )
{
  mExtent = extent;
}

QgsReferencedRectangle QgsVectorLayerExporter::ExportOptions::extent() const
{
  return mExtent;
}

void QgsVectorLayerExporter::ExportOptions::setFilterExpression( const QString &expression )
{
  mFilterExpression = expression;
}

QString QgsVectorLayerExporter::ExportOptions::filterExpression() const
{
  return mFilterExpression;
}

void QgsVectorLayerExporter::ExportOptions::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
}

const QgsExpressionContext &QgsVectorLayerExporter::ExportOptions::expressionContext() const
{
  return mExpressionContext;
}

QList<QgsVectorLayerExporter::OutputField> QgsVectorLayerExporter::ExportOptions::outputFields() const
{
  return mOutputFields;
}

void QgsVectorLayerExporter::ExportOptions::setOutputFields( const QList<QgsVectorLayerExporter::OutputField> &fields )
{
  mOutputFields = fields;
}


//
// QgsVectorLayerExporter
//

QgsVectorLayerExporter::QgsVectorLayerExporter( const QString &uri,
    const QString &providerKey,
    const QgsFields &fields,
    Qgis::WkbType geometryType,
    const QgsCoordinateReferenceSystem &crs,
    bool overwrite,
    const QMap<QString, QVariant> &options,
    QgsFeatureSink::SinkFlags sinkFlags )
{
  mProvider = nullptr;

  QMap<QString, QVariant> modifiedOptions( options );

  if ( providerKey == "ogr"_L1 &&
       options.contains( u"driverName"_s ) &&
       ( options[ u"driverName"_s ].toString().compare( "GPKG"_L1, Qt::CaseInsensitive ) == 0 ||
         options[ u"driverName"_s ].toString().compare( "SQLite"_L1, Qt::CaseInsensitive ) == 0 ) )
  {
    if ( geometryType != Qgis::WkbType::NoGeometry )
    {
      // For GPKG/Spatialite, we explicitly ask not to create a spatial index at
      // layer creation since this would slow down inserts. Defer its creation
      // to end of exportLayer() or destruction of this object.
      QStringList modifiedLayerOptions;
      if ( options.contains( u"layerOptions"_s ) )
      {
        const QStringList layerOptions = options.value( u"layerOptions"_s ).toStringList();
        for ( const QString &layerOption : layerOptions )
        {
          if ( layerOption.compare( "SPATIAL_INDEX=YES"_L1, Qt::CaseInsensitive ) == 0 ||
               layerOption.compare( "SPATIAL_INDEX=ON"_L1, Qt::CaseInsensitive ) == 0 ||
               layerOption.compare( "SPATIAL_INDEX=TRUE"_L1, Qt::CaseInsensitive ) == 0 ||
               layerOption.compare( "SPATIAL_INDEX=1"_L1, Qt::CaseInsensitive ) == 0 )
          {
            // do nothing
          }
          else if ( layerOption.compare( "SPATIAL_INDEX=NO"_L1, Qt::CaseInsensitive ) == 0 ||
                    layerOption.compare( "SPATIAL_INDEX=OFF"_L1, Qt::CaseInsensitive ) == 0 ||
                    layerOption.compare( "SPATIAL_INDEX=FALSE"_L1, Qt::CaseInsensitive ) == 0 ||
                    layerOption.compare( "SPATIAL_INDEX=0"_L1, Qt::CaseInsensitive ) == 0 )
          {
            mCreateSpatialIndex = false;
          }
          else
          {
            modifiedLayerOptions << layerOption;
          }
        }
      }
      modifiedLayerOptions << u"SPATIAL_INDEX=FALSE"_s;
      modifiedOptions[ u"layerOptions"_s ] = modifiedLayerOptions;
    }
  }

  // create an empty layer
  QString errMsg;
  QgsProviderRegistry *pReg = QgsProviderRegistry::instance();
  QString uriUpdated;
  mError = pReg->createEmptyLayer( providerKey, uri, fields, geometryType, crs, overwrite, mOldToNewAttrIdx,
                                   errMsg, !modifiedOptions.isEmpty() ? &modifiedOptions : nullptr, uriUpdated );

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

  QgsDebugMsgLevel( u"Created empty layer"_s, 2 );

  // Oracle specific HACK: we cannot guess the geometry type when there is no rows, so we need
  // to force it in the uri
  if ( providerKey == "oracle"_L1 )
  {
    uriUpdated += u" type=%1"_s.arg( QgsWkbTypes::displayString( geometryType ) );
  }

  const QgsDataProvider::ProviderOptions providerOptions;
  QgsVectorDataProvider *vectorProvider = qobject_cast< QgsVectorDataProvider * >( pReg->createProvider( providerKey, uriUpdated, providerOptions ) );
  if ( !vectorProvider || !vectorProvider->isValid() || ( vectorProvider->capabilities() & Qgis::VectorProviderCapability::AddFeatures ) == 0 )
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
  const QString path = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, uri ).value( u"path"_s ).toString();
  if ( sinkFlags.testFlag( QgsFeatureSink::SinkFlag::RegeneratePrimaryKey ) && path.endsWith( ".gpkg"_L1, Qt::CaseInsensitive ) )
  {
    const QString fidName = options.value( u"FID"_s, u"FID"_s ).toString();
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

Qgis::VectorDataProviderAttributeEditCapabilities QgsVectorLayerExporter::attributeEditCapabilities() const
{
  return mProvider ? mProvider->attributeEditCapabilities() : Qgis::VectorDataProviderAttributeEditCapabilities();
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

    QgsDebugMsgLevel( u"moving field from pos %1 to %2"_s.arg( i ).arg( dstIdx ), 3 );
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

    mErrorMessage = QObject::tr( "Creation error for features from #%1 to #%2. Provider errors were: \n%3" )
                    .arg( mFeatureBuffer.first().id() )
                    .arg( mFeatureBuffer.last().id() )
                    .arg( errors.join( QLatin1Char( '\n' ) ) );

    mError = Qgis::VectorExportResult::ErrorFeatureWriteFailed;
    mErrorCount += mFeatureBuffer.count();

    mFeatureBuffer.clear();
    QgsDebugError( mErrorMessage );
    return false;
  }

  mFeatureBuffer.clear();
  return true;
}

bool QgsVectorLayerExporter::createSpatialIndex()
{
  mCreateSpatialIndex = false;
  if ( mProvider && ( mProvider->capabilities() & Qgis::VectorProviderCapability::CreateSpatialIndex ) != 0 )
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
  ExportOptions exportOptions;
  exportOptions.setSelectedOnly( onlySelected );
  exportOptions.setDestinationCrs( destCRS );
  exportOptions.setTransformContext( layer->transformContext() );
  return exportLayer( layer, uri, providerKey, exportOptions, errorMessage, options, feedback );
}

Qgis::VectorExportResult QgsVectorLayerExporter::exportLayer( QgsVectorLayer *layer, const QString &uri, const QString &providerKey, const ExportOptions &exportOptions, QString *errorMessage, const QMap<QString, QVariant> &_providerOptions, QgsFeedback *feedback )
{
  QgsExpressionContext expressionContext = exportOptions.expressionContext();

  QgsCoordinateReferenceSystem outputCRS;
  QgsCoordinateTransform ct;
  bool shallTransform = false;

  if ( !layer )
    return Qgis::VectorExportResult::ErrorInvalidLayer;

  if ( exportOptions.destinationCrs().isValid() )
  {
    // This means we should transform
    outputCRS = exportOptions.destinationCrs();
    shallTransform = true;
  }
  else
  {
    // This means we shouldn't transform, use source CRS as output (if defined)
    outputCRS = layer->crs();
  }

  QMap<QString, QVariant> providerOptions = _providerOptions;
  const bool overwrite = providerOptions.take( u"overwrite"_s ).toBool();
  const bool forceSinglePartGeom = providerOptions.take( u"forceSinglePartGeometryType"_s ).toBool();

  QgsFields outputFields;
  bool useFieldMapping = false;
  QList<QgsExpression> expressions;
  if ( exportOptions.outputFields().isEmpty() )
  {
    outputFields = layer->fields();
  }
  else
  {
    useFieldMapping = true;
    const QList<QgsVectorLayerExporter::OutputField> exportFieldDefinitions = exportOptions.outputFields();
    for ( const QgsVectorLayerExporter::OutputField &field : exportFieldDefinitions )
    {
      outputFields.append( field.field );
      expressions.append( QgsExpression( field.expression ) );
      expressions.last().prepare( &expressionContext );
      if ( expressions.last().hasParserError() )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Parser error for field \"%1\" with expression \"%2\": %3" )
                          .arg(
                            field.field.name(),
                            field.expression,
                            expressions.last().parserErrorString()
                          );
        return Qgis::VectorExportResult::ErrorAttributeCreationFailed;
      }
    }
  }

  Qgis::WkbType wkbType = layer->wkbType();

  bool convertGeometryToSinglePart = false;
  if ( forceSinglePartGeom && QgsWkbTypes::isMultiType( wkbType ) )
  {
    wkbType = QgsWkbTypes::singleType( wkbType );
    convertGeometryToSinglePart = true;
  }

  auto writer = std::make_unique< QgsVectorLayerExporter >(
                  uri, providerKey, outputFields, wkbType, outputCRS, overwrite, providerOptions );

  // check whether file creation was successful
  const Qgis::VectorExportResult err = writer->errorCode();
  if ( err != Qgis::VectorExportResult::Success )
  {
    if ( errorMessage )
      *errorMessage = writer->errorMessage();
    return err;
  }

  if ( errorMessage )
  {
    errorMessage->clear();
  }

  // Create our transform
  if ( exportOptions.destinationCrs().isValid() )
  {
    ct = QgsCoordinateTransform( layer->crs(), exportOptions.destinationCrs(), exportOptions.transformContext() );
  }

  // Check for failure
  if ( !ct.isValid() )
    shallTransform = false;

  QgsFeatureRequest req;
  if ( wkbType == Qgis::WkbType::NoGeometry )
    req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );

  if ( !exportOptions.extent().isNull() )
  {
    QgsCoordinateTransform extentFilterTransform( exportOptions.extent().crs(), layer->crs(), exportOptions.transformContext() );
    extentFilterTransform.setBallparkTransformsAreAppropriate( true );

    try
    {
      const QgsRectangle layerExtent = extentFilterTransform.transformBoundingBox( exportOptions.extent() );
      req.setFilterRect( layerExtent );
    }
    catch ( QgsCsException &e )
    {
      QgsDebugError( u"Could not transform filter extent: %1"_s.arg( e.what() ) );
    }
  }

  if ( !exportOptions.filterExpression().isEmpty() )
  {
    req.setFilterExpression( exportOptions.filterExpression() );
    req.setExpressionContext( expressionContext );
  }
  else if ( exportOptions.selectedOnly() )
  {
    req.setFilterFids( layer->selectedFeatureIds() );
  }

  QgsFeatureIterator fit = layer->getFeatures( req );

  long long n = 0;
  const long long approxTotal = exportOptions.selectedOnly() ? layer->selectedFeatureCount() : layer->featureCount();

  if ( errorMessage )
  {
    *errorMessage = QObject::tr( "Feature write errors:" );
  }

  bool canceled = false;

  // write all features
  QgsFeature sourceFeature;
  while ( fit.nextFeature( sourceFeature ) )
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

    QgsFeature outputFeature( outputFields );
    outputFeature.setId( sourceFeature.id() );
    outputFeature.setGeometry( sourceFeature.geometry() );

    if ( shallTransform )
    {
      try
      {
        if ( outputFeature.hasGeometry() )
        {
          QgsGeometry g = outputFeature.geometry();
          g.transform( ct );
          outputFeature.setGeometry( g );
        }
      }
      catch ( QgsCsException &e )
      {
        const QString msg = QObject::tr( "Failed to transform feature with ID '%1'. Writing stopped. (Exception: %2)" )
                            .arg( sourceFeature.id() ).arg( e.what() );
        QgsMessageLog::logMessage( msg, QObject::tr( "Vector import" ) );
        if ( errorMessage )
          *errorMessage += '\n' + msg;

        return Qgis::VectorExportResult::ErrorProjectingFeatures;
      }
    }

    // Handles conversion to single-part
    if ( convertGeometryToSinglePart && outputFeature.geometry().isMultipart() )
    {
      QgsGeometry singlePartGeometry { outputFeature.geometry() };
      // We want a failure if the geometry cannot be converted to single-part without data loss!
      // check if there are more than one part
      const QgsGeometryCollection *c = qgsgeometry_cast<const QgsGeometryCollection *>( singlePartGeometry.constGet() );
      if ( ( c && c->partCount() > 1 ) || ! singlePartGeometry.convertToSingleType() )
      {
        const QString msg = QObject::tr( "Failed to transform a feature with ID '%1' to single part. Writing stopped." )
                            .arg( sourceFeature.id() );
        QgsMessageLog::logMessage( msg, QObject::tr( "Vector import" ) );
        if ( errorMessage )
          *errorMessage += '\n' + msg;
        return Qgis::VectorExportResult::ErrorFeatureWriteFailed;
      }
      outputFeature.setGeometry( singlePartGeometry );
    }

    // handle attribute mapping
    if ( useFieldMapping )
    {
      QgsAttributes attributes;
      attributes.reserve( expressions.size() );
      for ( auto it = expressions.begin(); it != expressions.end(); ++it )
      {
        if ( it->isValid() )
        {
          expressionContext.setFeature( sourceFeature );
          const QVariant value = it->evaluate( &expressionContext );
          if ( it->hasEvalError() )
          {
            const QString msg = QObject::tr( "Evaluation error in expression \"%1\": %2" ).arg( it->expression(), it->evalErrorString() );
            QgsMessageLog::logMessage( msg, QObject::tr( "Vector import" ) );
            if ( errorMessage )
              *errorMessage += '\n' + msg;
            return Qgis::VectorExportResult::ErrorFeatureWriteFailed;
          }
          attributes.append( value );
        }
        else
        {
          attributes.append( QVariant() );
        }
      }
      outputFeature.setAttributes( attributes );
    }
    else
    {
      outputFeature.setAttributes( sourceFeature.attributes() );
    }

    if ( !writer->addFeature( outputFeature ) )
    {
      if ( writer->errorCode() != Qgis::VectorExportResult::Success && errorMessage )
      {
        *errorMessage += '\n' + writer->errorMessage();
      }
    }
    n++;

    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( n ) / static_cast< double >( approxTotal ) );
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

  writer.reset();

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
  , mOptions( options )
  , mOwnedFeedback( new QgsFeedback() )
{
  mExportOptions.setDestinationCrs( destinationCrs );
  mExportOptions.setTransformContext( layer->transformContext() );

  if ( mLayer )
    setDependentLayers( QList< QgsMapLayer * >() << mLayer );
}

QgsVectorLayerExporterTask::QgsVectorLayerExporterTask( QgsVectorLayer *layer, const QString &uri, const QString &providerKey, const QgsVectorLayerExporter::ExportOptions &exportOptions, const QMap<QString, QVariant> &providerOptions, bool ownsLayer )
  : QgsTask( tr( "Exporting %1" ).arg( layer->name() ), QgsTask::CanCancel )
  , mLayer( layer )
  , mOwnsLayer( ownsLayer )
  , mDestUri( uri )
  , mDestProviderKey( providerKey )
  , mExportOptions( exportOptions )
  , mOptions( providerOptions )
  , mOwnedFeedback( new QgsFeedback() )
{
  if ( mLayer )
    setDependentLayers( QList< QgsMapLayer * >() << mLayer );
  if ( mLayer && mOwnsLayer )
    mLayer->moveToThread( nullptr );
}

QgsVectorLayerExporterTask *QgsVectorLayerExporterTask::withLayerOwnership( QgsVectorLayer *layer, const QString &uri, const QString &providerKey, const QgsCoordinateReferenceSystem &destinationCrs, const QMap<QString, QVariant> &options )
{
  auto newTask = std::make_unique<QgsVectorLayerExporterTask>( layer, uri, providerKey, destinationCrs, options, true );
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

  if ( mOwnsLayer )
    mLayer->moveToThread( QThread::currentThread() );

  connect( mOwnedFeedback.get(), &QgsFeedback::progressChanged, this, &QgsVectorLayerExporterTask::setProgress );


  mError = QgsVectorLayerExporter::exportLayer(
             mLayer.data(), mDestUri, mDestProviderKey, mExportOptions, &mErrorMessage,
             mOptions, mOwnedFeedback.get() );

  if ( mOwnsLayer )
    mLayer->moveToThread( nullptr );

  return mError == Qgis::VectorExportResult::Success;
}

void QgsVectorLayerExporterTask::finished( bool result )
{
  if ( mOwnsLayer && mLayer )
    mLayer->moveToThread( QThread::currentThread() );

  // QgsMapLayer has QTimer member, which must not be destroyed from another thread
  if ( mOwnsLayer )
    delete mLayer;

  if ( result )
    emit exportComplete();
  else
    emit errorOccurred( mError, mErrorMessage );
}
