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

#define FEATURE_BUFFER_SIZE 200

typedef QgsVectorLayerImport::ImportError createEmptyLayer_t(
  const QString &uri,
  const QgsFieldMap &fields,
  QGis::WkbType geometryType,
  const QgsCoordinateReferenceSystem *destCRS,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdx,
  QString *errorMessage,
  const QMap<QString, QVariant> *options
);


QgsVectorLayerImport::QgsVectorLayerImport( const QString &uri,
    const QString &providerKey,
    const QgsFieldMap& fields,
    QGis::WkbType geometryType,
    const QgsCoordinateReferenceSystem* crs,
    bool overwrite,
    const QMap<QString, QVariant> *options )
    : mErrorCount( 0 )
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
    mErrorMessage = QObject::tr( "Provider %1 has no createEmptyLayer method" ).arg( providerKey );
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

  QgsDebugMsg( "Created empty layer" );

  QgsVectorDataProvider *vectorProvider = ( QgsVectorDataProvider* ) pReg->provider( providerKey, uri );
  if ( !vectorProvider || !vectorProvider->isValid() )
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
  const QgsAttributeMap &attrs = feat.attributeMap();

  QgsFeature newFeat;
  newFeat.setGeometry( *feat.geometry() );

  for ( QgsAttributeMap::const_iterator it = attrs.begin(); it != attrs.end(); it++ )
  {
    // add only mapped attributes (un-mapped ones are not present in the
    // destination layer)
    if ( mOldToNewAttrIdx.contains( it.key() ) )
    {
      QgsDebugMsgLevel( QString( "moving field from pos %1 to %2" ).arg( it.key() ).arg( mOldToNewAttrIdx.value( it.key() ) ), 3 );
      newFeat.addAttribute( mOldToNewAttrIdx.value( it.key() ), *it );
    }
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


QgsVectorLayerImport::ImportError
QgsVectorLayerImport::importLayer( QgsVectorLayer* layer,
                                   const QString& uri,
                                   const QString& providerKey,
                                   const QgsCoordinateReferenceSystem *destCRS,
                                   bool onlySelected,
                                   QString *errorMessage,
                                   bool skipAttributeCreation,
                                   QMap<QString, QVariant> *options )
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

  QgsFieldMap fields = skipAttributeCreation ? QgsFieldMap() : layer->pendingFields();
  if ( layer->providerType() == "ogr" && layer->storageType() == "ESRI Shapefile" )
  {
    // convert field names to lowercase
    for ( QgsFieldMap::iterator fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
    {
      fldIt.value().setName( fldIt.value().name().toLower() );
    }
  }

  bool overwrite = false;
  if ( options )
  {
    overwrite = options->take( "overwrite" ).toBool();
  }

  QgsVectorLayerImport * writer =
    new QgsVectorLayerImport( uri, providerKey, fields, layer->wkbType(), outputCRS, overwrite, options );

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

  layer->select( allAttr, QgsRectangle(), layer->wkbType() != QGis::WKBNoGeometry );

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

  // write all features
  while ( layer->nextFeature( fet ) )
  {
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

        QString msg = QObject::tr( "Failed to transform a point while drawing a feature of type '%1'. Writing stopped. (Exception: %2)" )
                      .arg( fet.typeName() ).arg( e.what() );
        QgsMessageLog::logMessage( msg, QObject::tr( "Vector import" ) );
        if ( errorMessage )
          *errorMessage += "\n" + msg;

        return ErrProjection;
      }
    }
    if ( skipAttributeCreation )
    {
      fet.clearAttributeMap();
    }
    if ( !writer->addFeature( fet ) )
    {
      if ( writer->hasError() && errorMessage )
      {
        *errorMessage += "\n" + writer->errorMessage();
      }
    }
    n++;
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
