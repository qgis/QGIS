/***************************************************************************
      qgsafsprovider.cpp
      ------------------
    begin                : May 27, 2015
    copyright            : (C) 2015 Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsafsprovider.h"
#include "qgsarcgisrestutils.h"
#include "qgsafsfeatureiterator.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "geometry/qgsgeometry.h"
#include "qgsnetworkaccessmanager.h"

#ifdef HAVE_GUI
#include "qgsafssourceselect.h"
#include "qgssourceselectprovider.h"
#endif

#include <QEventLoop>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>


QgsAfsProvider::QgsAfsProvider( const QString &uri )
  : QgsVectorDataProvider( uri )
  , mValid( false )
  , mObjectIdFieldIdx( -1 )
{
  mSharedData.reset( new QgsAfsSharedData() );
  mSharedData->mGeometryType = QgsWkbTypes::Unknown;
  mSharedData->mDataSource = QgsDataSourceUri( uri );

  // Set CRS
  mSharedData->mSourceCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mSharedData->mDataSource.param( QStringLiteral( "crs" ) ) );

  // Get layer info
  QString errorTitle, errorMessage;
  QVariantMap layerData = QgsArcGisRestUtils::getLayerInfo( mSharedData->mDataSource.param( QStringLiteral( "url" ) ), errorTitle, errorMessage );
  if ( layerData.isEmpty() )
  {
    pushError( errorTitle + ": " + errorMessage );
    appendError( QgsErrorMessage( tr( "getLayerInfo failed" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  mLayerName = layerData[QStringLiteral( "name" )].toString();
  mLayerDescription = layerData[QStringLiteral( "description" )].toString();

  // Set extent
  QStringList coords = mSharedData->mDataSource.param( QStringLiteral( "bbox" ) ).split( QStringLiteral( "," ) );
  if ( coords.size() == 4 )
  {
    bool xminOk = false, yminOk = false, xmaxOk = false, ymaxOk = false;
    mSharedData->mExtent.setXMinimum( coords[0].toDouble( &xminOk ) );
    mSharedData->mExtent.setYMinimum( coords[1].toDouble( &yminOk ) );
    mSharedData->mExtent.setXMaximum( coords[2].toDouble( &xmaxOk ) );
    mSharedData->mExtent.setYMaximum( coords[3].toDouble( &ymaxOk ) );
    if ( !xminOk || !yminOk || !xmaxOk || !ymaxOk )
      mSharedData->mExtent = QgsRectangle();
  }
  if ( mSharedData->mExtent.isEmpty() )
  {
    QVariantMap layerExtentMap = layerData[QStringLiteral( "extent" )].toMap();
    bool xminOk = false, yminOk = false, xmaxOk = false, ymaxOk = false;
    mSharedData->mExtent.setXMinimum( layerExtentMap[QStringLiteral( "xmin" )].toDouble( &xminOk ) );
    mSharedData->mExtent.setYMinimum( layerExtentMap[QStringLiteral( "ymin" )].toDouble( &yminOk ) );
    mSharedData->mExtent.setXMaximum( layerExtentMap[QStringLiteral( "xmax" )].toDouble( &xmaxOk ) );
    mSharedData->mExtent.setYMaximum( layerExtentMap[QStringLiteral( "ymax" )].toDouble( &ymaxOk ) );
    if ( !xminOk || !yminOk || !xmaxOk || !ymaxOk )
    {
      appendError( QgsErrorMessage( tr( "Could not retrieve layer extent" ), QStringLiteral( "AFSProvider" ) ) );
      return;
    }
    QgsCoordinateReferenceSystem extentCrs = QgsArcGisRestUtils::parseSpatialReference( layerExtentMap[QStringLiteral( "spatialReference" )].toMap() );
    if ( !extentCrs.isValid() )
    {
      appendError( QgsErrorMessage( tr( "Could not parse spatial reference" ), QStringLiteral( "AFSProvider" ) ) );
      return;
    }
    mSharedData->mExtent = QgsCoordinateTransform( extentCrs, mSharedData->mSourceCRS ).transformBoundingBox( mSharedData->mExtent );
  }

  // Read fields
  foreach ( const QVariant &fieldData, layerData["fields"].toList() )
  {
    QVariantMap fieldDataMap = fieldData.toMap();
    QString fieldName = fieldDataMap[QStringLiteral( "name" )].toString();
    QVariant::Type type = QgsArcGisRestUtils::mapEsriFieldType( fieldDataMap[QStringLiteral( "type" )].toString() );
    if ( fieldName == QLatin1String( "geometry" ) || type == QVariant::Invalid )
    {
      QgsDebugMsg( QString( "Skipping unsupported (or possibly geometry) field" ).arg( fieldName ) );
      continue;
    }
    QgsField field( fieldName, type, fieldDataMap[QStringLiteral( "type" )].toString(), fieldDataMap[QStringLiteral( "length" )].toInt() );
    mSharedData->mFields.append( field );
  }

  // Determine geometry type
  bool hasM = layerData[QStringLiteral( "hasM" )].toBool();
  bool hasZ = layerData[QStringLiteral( "hasZ" )].toBool();
  mSharedData->mGeometryType = QgsArcGisRestUtils::mapEsriGeometryType( layerData[QStringLiteral( "geometryType" )].toString() );
  if ( mSharedData->mGeometryType == QgsWkbTypes::Unknown )
  {
    appendError( QgsErrorMessage( tr( "Failed to determine geometry type" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  mSharedData->mGeometryType = QgsWkbTypes::zmType( mSharedData->mGeometryType, hasZ, hasM );

  // Read OBJECTIDs of all features: these may not be a continuous sequence,
  // and we need to store these to iterate through the features. This query
  // also returns the name of the ObjectID field.
  QVariantMap objectIdData = QgsArcGisRestUtils::getObjectIds( mSharedData->mDataSource.param( QStringLiteral( "url" ) ), errorTitle, errorMessage );
  if ( objectIdData.isEmpty() )
  {
    appendError( QgsErrorMessage( tr( "getObjectIds failed: %1 - %2" ).arg( errorTitle, errorMessage ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  if ( !objectIdData[QStringLiteral( "objectIdFieldName" )].isValid() || !objectIdData[QStringLiteral( "objectIds" )].isValid() )
  {
    appendError( QgsErrorMessage( tr( "Failed to determine objectIdFieldName and/or objectIds" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  QString objectIdFieldName = objectIdData[QStringLiteral( "objectIdFieldName" )].toString();
  for ( int idx = 0, nIdx = mSharedData->mFields.count(); idx < nIdx; ++idx )
  {
    if ( mSharedData->mFields.at( idx ).name() == objectIdFieldName )
    {
      mObjectIdFieldIdx = idx;

      // primary key is not null, unique
      QgsFieldConstraints constraints = mSharedData->mFields.at( idx ).constraints();
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
      mSharedData->mFields[ idx ].setConstraints( constraints );

      break;
    }
  }
  foreach ( const QVariant &objectId, objectIdData["objectIds"].toList() )
  {
    mSharedData->mObjectIds.append( objectId.toInt() );
  }

  mValid = true;
}

QgsAbstractFeatureSource *QgsAfsProvider::featureSource() const
{
  return new QgsAfsFeatureSource( mSharedData );
}

QgsFeatureIterator QgsAfsProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return new QgsAfsFeatureIterator( new QgsAfsFeatureSource( mSharedData ), true, request );
}

QgsWkbTypes::Type QgsAfsProvider::wkbType() const
{
  return mSharedData->mGeometryType;
}

long QgsAfsProvider::featureCount() const
{
  return mSharedData->mObjectIds.size();
}

QgsFields QgsAfsProvider::fields() const
{
  return mSharedData->mFields;
}

void QgsAfsProvider::setDataSourceUri( const QString &uri )
{
  mSharedData->mDataSource = QgsDataSourceUri( uri );
  QgsDataProvider::setDataSourceUri( uri );
}

QgsCoordinateReferenceSystem QgsAfsProvider::crs() const
{
  return mSharedData->crs();
}

QgsRectangle QgsAfsProvider::extent() const
{
  return mSharedData->extent();
}

void QgsAfsProvider::reloadData()
{
  mSharedData->mCache.clear();
}


#ifdef HAVE_GUI

//! Provider for AFS layers source select
class QgsAfsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    virtual QString providerKey() const override { return QStringLiteral( "arcgisfeatureserver" ); }
    virtual QString text() const override { return QObject::tr( "ArcGIS Feature Server" ); }
    virtual int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 150; }
    virtual QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddAfsLayer.svg" ) ); }
    virtual QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsAfsSourceSelect( parent, fl, widgetMode );
    }
};


QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers
      << new QgsAfsSourceSelectProvider;

  return providers;
}
#endif
