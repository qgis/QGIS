/***************************************************************************
                          qgsvectorlayerimport.cpp
                          vector layer importer
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

#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerimport.h"
#include "qgsproviderregistry.h"
#include "qgsdatasourceuri.h"

#include <QProgressDialog>

#define FEATURE_BUFFER_SIZE 200

typedef QgsVectorLayerImport::ImportError createEmptyLayer_t(
  const QString &uri,
  const QgsFields &fields,
  QGis::WkbType geometryType,
  const QgsCoordinateReferenceSystem *destCRS,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdx,
  QString *errorMessage,
  const QMap<QString, QVariant> *options
);


QgsVectorLayerImport::QgsVectorLayerImport( const QString &uri,
    const QString &providerKey,
    const QgsFields& fields,
    QGis::WkbType geometryType,
    const QgsCoordinateReferenceSystem* crs,
    bool overwrite,
    const QMap<QString, QVariant> *options,
    QProgressDialog *progress )
    : mErrorCount( 0 )
    , mAttributeCount( -1 )
    , mProgress( progress )

{
  mProvider = NULL;

  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();

  QLibrary *myLib = pReg->providerLibrary( providerKey );
  if ( !myLib )
  {
    mError = ErrInvalidProvider;
    mErrorMessage = QObject::tr( "Unable to load %1 provider" ).arg( providerKey );
    return;
  }

  createEmptyLayer_t * pCreateEmpty = ( createEmptyLayer_t * ) cast_to_fptr( myLib->resolve( "createEmptyLayer" ) );
  if ( !pCreateEmpty )
  {
    delete myLib;
    mError = ErrProviderUnsupportedFeature;
    mErrorMessage = QObject::tr( "Provider %1 has no %2 method" ).arg( providerKey ).arg( "createEmptyLayer" );
    return;
  }

  delete myLib;

  // create an empty layer
  QString errMsg;
  mError = pCreateEmpty( uri, fields, geometryType, crs, overwrite, &mOldToNewAttrIdx, &errMsg, options );
  if ( hasError() )
  {
    mErrorMessage = errMsg;
    return;
  }

  foreach ( int idx, mOldToNewAttrIdx.values() )
  {
    if ( idx > mAttributeCount )
      mAttributeCount = idx;
  }

  mAttributeCount++;

  QgsDebugMsg( "Created empty layer" );

  QgsVectorDataProvider *vectorProvider = ( QgsVectorDataProvider* ) pReg->provider( providerKey, uri );
  if ( !vectorProvider || !vectorProvider->isValid() || ( vectorProvider->capabilities() & QgsVectorDataProvider::AddFeatures ) == 0 )
  {
    mError = ErrInvalidLayer;
    mErrorMessage = QObject::tr( "Loading of layer failed" );

    if ( vectorProvider )
      delete vectorProvider;

    return;
  }

  mProvider = vectorProvider;
  mError = NoError;
}

QgsVectorLayerImport::~QgsVectorLayerImport()
{
  flushBuffer();

  if ( mProvider )
    delete mProvider;
}

QgsVectorLayerImport::ImportError QgsVectorLayerImport::hasError()
{
  return mError;
}

QString QgsVectorLayerImport::errorMessage()
{
  return mErrorMessage;
}

bool QgsVectorLayerImport::addFeature( QgsFeature& feat )
{
  QgsAttributes attrs = feat.attributes();

  QgsFeature newFeat;
  if ( feat.constGeometry() )
    newFeat.setGeometry( *feat.constGeometry() );

  newFeat.initAttributes( mAttributeCount );

  for ( int i = 0; i < attrs.count(); ++i )
  {
    // add only mapped attributes (un-mapped ones will not be present in the
    // destination layer)
    int dstIdx = mOldToNewAttrIdx.value( i, -1 );
    if ( dstIdx < 0 )
      continue;

    QgsDebugMsgLevel( QString( "moving field from pos %1 to %2" ).arg( i ).arg( dstIdx ), 3 );
    newFeat.setAttribute( dstIdx, attrs[i] );
  }

  mFeatureBuffer.append( newFeat );

  if ( mFeatureBuffer.count() >= FEATURE_BUFFER_SIZE )
  {
    return flushBuffer();
  }

  return true;
}

bool QgsVectorLayerImport::flushBuffer()
{
  if ( mFeatureBuffer.count() <= 0 )
    return true;

  if ( !mProvider->addFeatures( mFeatureBuffer ) )
  {
    QStringList errors = mProvider->errors();
    mProvider->clearErrors();

    mErrorMessage = QObject::tr( "Creation error for features from #%1 to #%2. Provider errors was: \n%3" )
                    .arg( mFeatureBuffer.first().id() )
                    .arg( mFeatureBuffer.last().id() )
                    .arg( errors.join( "\n" ) );

    mError = ErrFeatureWriteFailed;
    mErrorCount += mFeatureBuffer.count();

    mFeatureBuffer.clear();
    QgsDebugMsg( mErrorMessage );
    return false;
  }

  mFeatureBuffer.clear();
  return true;
}

bool QgsVectorLayerImport::createSpatialIndex()
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

QgsVectorLayerImport::ImportError
QgsVectorLayerImport::importLayer( QgsVectorLayer* layer,
                                   const QString& uri,
                                   const QString& providerKey,
                                   const QgsCoordinateReferenceSystem *destCRS,
                                   bool onlySelected,
                                   QString *errorMessage,
                                   bool skipAttributeCreation,
                                   QMap<QString, QVariant> *options,
                                   QProgressDialog *progress )
{
  const QgsCoordinateReferenceSystem* outputCRS;
  QgsCoordinateTransform* ct = 0;
  int shallTransform = false;

  if ( layer == NULL )
  {
    return ErrInvalidLayer;
  }

  if ( destCRS && destCRS->isValid() )
  {
    // This means we should transform
    outputCRS = destCRS;
    shallTransform = true;
  }
  else
  {
    // This means we shouldn't transform, use source CRS as output (if defined)
    outputCRS = &layer->crs();
  }


  bool overwrite = false;
  bool forceSinglePartGeom = false;
  if ( options )
  {
    overwrite = options->take( "overwrite" ).toBool();
    forceSinglePartGeom = options->take( "forceSinglePartGeometryType" ).toBool();
  }

  QgsFields fields = skipAttributeCreation ? QgsFields() : layer->pendingFields();
  QGis::WkbType wkbType = layer->wkbType();

  // Special handling for Shapefiles
  if ( layer->providerType() == "ogr" && layer->storageType() == "ESRI Shapefile" )
  {
    // convert field names to lowercase
    for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
    {
      fields[fldIdx].setName( fields[fldIdx].name().toLower() );
    }

    if ( !forceSinglePartGeom )
    {
      // convert wkbtype to multipart (see #5547)
      switch ( wkbType )
      {
        case QGis::WKBPoint:
          wkbType = QGis::WKBMultiPoint;
          break;
        case QGis::WKBLineString:
          wkbType = QGis::WKBMultiLineString;
          break;
        case QGis::WKBPolygon:
          wkbType = QGis::WKBMultiPolygon;
          break;
        case QGis::WKBPoint25D:
          wkbType = QGis::WKBMultiPoint25D;
          break;
        case QGis::WKBLineString25D:
          wkbType = QGis::WKBMultiLineString25D;
          break;
        case QGis::WKBPolygon25D:
          wkbType = QGis::WKBMultiPolygon25D;
          break;
        default:
          break;
      }
    }
  }

  QgsVectorLayerImport * writer =
    new QgsVectorLayerImport( uri, providerKey, fields, wkbType, outputCRS, overwrite, options, progress );

  // check whether file creation was successful
  ImportError err = writer->hasError();
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

  QgsAttributeList allAttr = skipAttributeCreation ? QgsAttributeList() : layer->pendingAllAttributesList();
  QgsFeature fet;

  QgsFeatureRequest req;
  if ( wkbType == QGis::WKBNoGeometry )
    req.setFlags( QgsFeatureRequest::NoGeometry );
  if ( skipAttributeCreation )
    req.setSubsetOfAttributes( QgsAttributeList() );

  QgsFeatureIterator fit = layer->getFeatures( req );

  const QgsFeatureIds& ids = layer->selectedFeaturesIds();

  // Create our transform
  if ( destCRS )
  {
    ct = new QgsCoordinateTransform( layer->crs(), *destCRS );
  }

  // Check for failure
  if ( ct == NULL )
  {
    shallTransform = false;
  }

  int n = 0;

  if ( errorMessage )
  {
    *errorMessage = QObject::tr( "Feature write errors:" );
  }

  if ( progress )
  {
    progress->setRange( 0, layer->featureCount() );
  }

  // write all features
  while ( fit.nextFeature( fet ) )
  {
    if ( progress && progress->wasCanceled() )
    {
      if ( errorMessage )
      {
        *errorMessage += "\n" + QObject::tr( "Import was canceled at %1 of %2" ).arg( progress->value() ).arg( progress->maximum() );
      }
      break;
    }

    if ( writer->errorCount() > 1000 )
    {
      if ( errorMessage )
      {
        *errorMessage += "\n" + QObject::tr( "Stopping after %1 errors" ).arg( writer->errorCount() );
      }
      break;
    }

    if ( onlySelected && !ids.contains( fet.id() ) )
      continue;

    if ( shallTransform )
    {
      try
      {
        if ( fet.geometry() )
        {
          fet.geometry()->transform( *ct );
        }
      }
      catch ( QgsCsException &e )
      {
        delete ct;
        delete writer;

        QString msg = QObject::tr( "Failed to transform a point while drawing a feature with ID '%1'. Writing stopped. (Exception: %2)" )
                      .arg( fet.id() ).arg( e.what() );
        QgsMessageLog::logMessage( msg, QObject::tr( "Vector import" ) );
        if ( errorMessage )
          *errorMessage += "\n" + msg;

        return ErrProjection;
      }
    }
    if ( skipAttributeCreation )
    {
      fet.initAttributes( 0 );
    }
    if ( !writer->addFeature( fet ) )
    {
      if ( writer->hasError() && errorMessage )
      {
        *errorMessage += "\n" + writer->errorMessage();
      }
    }
    n++;

    if ( progress )
    {
      progress->setValue( n );
    }
  }

  // flush the buffer to be sure that all features are written
  if ( !writer->flushBuffer() )
  {
    if ( writer->hasError() && errorMessage )
    {
      *errorMessage += "\n" + writer->errorMessage();
    }
  }
  int errors = writer->errorCount();

  if ( !writer->createSpatialIndex() )
  {
    if ( writer->hasError() && errorMessage )
    {
      *errorMessage += "\n" + writer->errorMessage();
    }
  }

  delete writer;

  if ( shallTransform )
  {
    delete ct;
  }

  if ( errorMessage )
  {
    if ( errors > 0 )
    {
      *errorMessage += "\n" + QObject::tr( "Only %1 of %2 features written." ).arg( n - errors ).arg( n );
    }
    else
    {
      errorMessage->clear();
    }
  }

  return errors == 0 ? NoError : ErrFeatureWriteFailed;
}
