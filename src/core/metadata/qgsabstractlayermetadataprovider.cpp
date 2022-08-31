/***************************************************************************
  qgsabstractlayermetadataprovider.cpp - QgsAbstractLayerMetadataProvider

 ---------------------
 begin                : 17.8.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsabstractlayermetadataprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsfeedback.h"


const QList<QgsLayerMetadataProviderResult> &QgsLayerMetadataSearchResult::metadata() const
{
  return mMetadata;
}

void QgsLayerMetadataSearchResult::addMetadata( const QgsLayerMetadataProviderResult &newMetadata )
{
  mMetadata.push_back( newMetadata );
}


const QStringList &QgsLayerMetadataSearchResult::errors() const
{
  return mErrors;
}

void QgsLayerMetadataSearchResult::addError( const QString &newError )
{
  mErrors.push_back( newError );
}


QgsLayerMetadataProviderResult::QgsLayerMetadataProviderResult( const QgsLayerMetadata &metadata )
  : QgsLayerMetadata( metadata )
{

}

const QgsPolygon &QgsLayerMetadataProviderResult::geographicExtent() const
{
  return mGeographicExtent;
}

void QgsLayerMetadataProviderResult::setGeographicExtent( const QgsPolygon &newGeographicExtent )
{
  mGeographicExtent = newGeographicExtent;
}

const QgsWkbTypes::GeometryType &QgsLayerMetadataProviderResult::geometryType() const
{
  return mGeometryType;
}

void QgsLayerMetadataProviderResult::setGeometryType( const QgsWkbTypes::GeometryType &newGeometryType )
{
  mGeometryType = newGeometryType;
}

const QString &QgsLayerMetadataProviderResult::authid() const
{
  return mAuthid;
}

void QgsLayerMetadataProviderResult::setAuthid( const QString &newAuthid )
{
  mAuthid = newAuthid;
}

const QString &QgsLayerMetadataProviderResult::uri() const
{
  return mUri;
}

void QgsLayerMetadataProviderResult::setUri( const QString &newUri )
{
  mUri = newUri;
}

const QString &QgsLayerMetadataProviderResult::dataProviderName() const
{
  return mDataProviderName;
}

void QgsLayerMetadataProviderResult::setDataProviderName( const QString &newDataProviderName )
{
  mDataProviderName = newDataProviderName;
}

QgsMapLayerType QgsLayerMetadataProviderResult::layerType() const
{
  return mLayerType;
}

void QgsLayerMetadataProviderResult::setLayerType( QgsMapLayerType newLayerType )
{
  mLayerType = newLayerType;
}

const QString &QgsLayerMetadataProviderResult::standardUri() const
{
  return mStandardUri;
}

void QgsLayerMetadataProviderResult::setStandardUri( const QString &newStandardUri )
{
  mStandardUri = newStandardUri;
}
