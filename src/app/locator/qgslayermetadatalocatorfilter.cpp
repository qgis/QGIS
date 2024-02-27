/***************************************************************************
  qgslayermetadatalocatorfilter.cpp - QgsLayerMetadataLocatorFilter

 ---------------------
 begin                : 5.9.2022
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
#include "qgslayermetadatalocatorfilter.h"
#include "qgslayermetadataproviderregistry.h"
#include "qgsfeedback.h"
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgsiconutils.h"
#include <QIcon>

QgsLayerMetadataLocatorFilter::QgsLayerMetadataLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{
  setUseWithoutPrefix( false );
}

QgsLocatorFilter *QgsLayerMetadataLocatorFilter::clone() const
{
  return new QgsLayerMetadataLocatorFilter();
}

void QgsLayerMetadataLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback )
{

  QgsMetadataSearchContext ctx;
  ctx.transformContext = context.transformContext;
  const QList<QgsAbstractLayerMetadataProvider *> providers { QgsApplication::instance()->layerMetadataProviderRegistry()->layerMetadataProviders() };
  for ( QgsAbstractLayerMetadataProvider *mdProvider : std::as_const( providers ) )
  {
    const QList<QgsLayerMetadataProviderResult> mdRecords { mdProvider->search( ctx, string, QgsRectangle(), feedback ).metadata()  };
    for ( const QgsLayerMetadataProviderResult &metadata : std::as_const( mdRecords ) )
    {
      QgsLocatorResult result;
      result.displayString = metadata.identifier();
      result.description = metadata.title();
      result.icon = QgsIconUtils::iconForGeometryType( metadata.geometryType() );
      result.setUserData( QVariant::fromValue( metadata ) );
      emit resultFetched( result );
    }
  }
}

void QgsLayerMetadataLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QgsLayerMetadataProviderResult metadataResult { result.userData().value<QgsLayerMetadataProviderResult>() };
  switch ( metadataResult.layerType() )
  {
    case Qgis::LayerType::Raster:
    {
      QgisApp::instance()->addRasterLayer( metadataResult.uri(), metadataResult.identifier(), metadataResult.dataProviderName() );
      break;
    }
    case Qgis::LayerType::Vector:
    {
      QgisApp::instance()->addVectorLayer( metadataResult.uri(), metadataResult.identifier(), metadataResult.dataProviderName() );
      break;
    }
    case Qgis::LayerType::Mesh:
    {
      QgisApp::instance()->addMeshLayer( metadataResult.uri(), metadataResult.identifier(), metadataResult.dataProviderName() );
      break;
    }
    default:  // unsupported
    {
      // Ignore
      break;
    }
  }

}
