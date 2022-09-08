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

QList<QgsLayerMetadataProviderResult> QgsLayerMetadataSearchResults::metadata() const
{
  return mMetadata;
}

void QgsLayerMetadataSearchResults::addMetadata( const QgsLayerMetadataProviderResult &metadata )
{
  mMetadata.push_back( metadata );
}


QStringList QgsLayerMetadataSearchResults::errors() const
{
  return mErrors;
}

void QgsLayerMetadataSearchResults::addError( const QString &error )
{
  mErrors.push_back( error );
}


QgsLayerMetadataProviderResult::QgsLayerMetadataProviderResult( const QgsLayerMetadata &metadata )
  : QgsLayerMetadata( metadata )
{

}

const QgsPolygon &QgsLayerMetadataProviderResult::geographicExtent() const
{
  return mGeographicExtent;
}

void QgsLayerMetadataProviderResult::setGeographicExtent( const QgsPolygon &geographicExtent )
{
  mGeographicExtent = geographicExtent;
}

const QgsWkbTypes::GeometryType &QgsLayerMetadataProviderResult::geometryType() const
{
  return mGeometryType;
}

void QgsLayerMetadataProviderResult::setGeometryType( const QgsWkbTypes::GeometryType &geometryType )
{
  mGeometryType = geometryType;
}

const QString &QgsLayerMetadataProviderResult::authid() const
{
  return mAuthid;
}

void QgsLayerMetadataProviderResult::setAuthid( const QString &authid )
{
  mAuthid = authid;
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

void QgsLayerMetadataProviderResult::setDataProviderName( const QString &dataProviderName )
{
  mDataProviderName = dataProviderName;
}

QgsMapLayerType QgsLayerMetadataProviderResult::layerType() const
{
  return mLayerType;
}

void QgsLayerMetadataProviderResult::setLayerType( QgsMapLayerType layerType )
{
  mLayerType = layerType;
}

const QString &QgsLayerMetadataProviderResult::standardUri() const
{
  return mStandardUri;
}

void QgsLayerMetadataProviderResult::setStandardUri( const QString &standardUri )
{
  mStandardUri = standardUri;
}


