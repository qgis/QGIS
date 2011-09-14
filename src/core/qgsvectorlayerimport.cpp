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
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerimport.h"
#include "qgsproviderregistry.h"

#include <QFile>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QTextCodec>
#include <QTextStream>
#include <QSet>
#include <QMetaType>

#include <cassert>
#include <cstdlib> // size_t
#include <limits> // std::numeric_limits


#define FEATURE_BUFFER_SIZE 200

typedef QgsVectorLayerImport::ImportError createEmptyLayer_t(
                        const QString &uri,
                        const QgsFieldMap &fields,
                        QGis::WkbType geometryType,
                        const QgsCoordinateReferenceSystem *destCRS,
                        bool overwrite,
                        QMap<int, int> *oldToNewAttrIdx,
                        QString *errorMessage = 0,
                        const QMap<QString, QVariant> *options = 0
                      );


QgsVectorLayerImport::QgsVectorLayerImport(
    const QString &uri,
    const QString &providerKey,
    const QgsFieldMap& fields,
    QGis::WkbType geometryType,
    const QgsCoordinateReferenceSystem* crs,
    bool overwrite,
    const QMap<QString, QVariant> *options )
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

  QgsAttributeMap newAttrs;
  for ( QgsAttributeMap::const_iterator it = attrs.begin(); it != attrs.end(); it++ )
  {
    if ( mOldToNewAttrIdx.contains( it.key() ) )
    {
      QgsDebugMsgLevel( QString( "moving field from pos %1 to %2" ).arg( it.key() ).arg( mOldToNewAttrIdx.value( it.key() ) ), 3 );
      newAttrs.insert( mOldToNewAttrIdx.value( it.key() ), *it );
    }
    else
    {
      QgsDebugMsgLevel( QString( "added attr pos %1" ).arg( it.key() ), 3 );
      newAttrs.insert( it.key(), *it );
    }
  }
  feat.setAttributeMap( newAttrs );

  mFeatureBuffer.append( feat );

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
    mErrorMessage = QObject::tr( "Creation error for features from #%1 to #%2" )
                    .arg( mFeatureBuffer.first().id() )
                    .arg( mFeatureBuffer.last().id() );
    mError = ErrFeatureWriteFailed;

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

  QgsVectorLayerImport * writer =
    new QgsVectorLayerImport( uri, providerKey, skipAttributeCreation ? QgsFieldMap() : layer->pendingFields(), layer->wkbType(), outputCRS, false, options );

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

  int n = 0, errors = 0;

  // write all features
  while ( layer->nextFeature( fet ) )
  {
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
        QgsLogger::warning( msg );
        if ( errorMessage )
          *errorMessage = msg;

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
        if ( errorMessage->isEmpty() )
        {
          *errorMessage = QObject::tr( "Feature write errors:" );
        }
        *errorMessage += "\n" + writer->errorMessage();
      }
      errors++;

      if ( errors > 1000 )
      {
        if ( errorMessage )
        {
          *errorMessage += QObject::tr( "Stopping after %1 errors" ).arg( errors );
        }

        n = -1;
        break;
      }
    }
    n++;
  }

  delete writer;

  if ( shallTransform )
  {
    delete ct;
  }

  if ( errors > 0 && errorMessage && n > 0 )
  {
    *errorMessage += QObject::tr( "\nOnly %1 of %2 features written." ).arg( n - errors ).arg( n );
  }

  return errors == 0 ? NoError : ErrFeatureWriteFailed;
}
