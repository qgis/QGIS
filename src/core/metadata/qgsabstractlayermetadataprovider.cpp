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

QString QgsLayerMetadataProviderResult::identifier() const
{
  return metadata.identifier();
}

QString QgsLayerMetadataProviderResult::title() const
{
  return metadata.title();
}

QString QgsLayerMetadataProviderResult::abstract() const
{
  return metadata.abstract();
}

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

