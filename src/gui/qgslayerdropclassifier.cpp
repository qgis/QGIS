/***************************************************************************
  qgslayerdropclassifier.cpp
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayerdropclassifier.h"

#include "qgscustomdrophandler.h"
#include "qgsmimedatautils.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"

#include <QFileInfo>
#include <QMimeData>
#include <QString>
#include <QUrl>

using namespace Qt::StringLiterals;

Qgis::LayerDropPayloadType QgsLayerDropClassifier::classify( const QMimeData *mimeData, const QVector<QPointer<QgsCustomDropHandler>> &customHandlers )
{
  bool hasLayer = false;
  bool hasCustomUri = false;

  if ( QgsMimeDataUtils::isUriList( mimeData ) )
  {
    const QgsMimeDataUtils::UriList uris = QgsMimeDataUtils::decodeUriList( mimeData );
    for ( const QgsMimeDataUtils::Uri &uri : uris )
    {
      if ( uri.layerType == "project"_L1 )
      {
        return Qgis::LayerDropPayloadType::Project;
      }
      if ( uri.layerType == "custom"_L1 )
      {
        // a custom uri (e.g. a Processing model dragged from the browser) is not inserted
        // as a layer: it is dispatched to a matching custom drop handler through
        // handleCustomUriDrop(), so it must not be treated like a loadable dataset
        for ( QgsCustomDropHandler *handler : customHandlers )
        {
          if ( handler && handler->customUriProviderKey() == uri.providerKey )
          {
            hasCustomUri = true;
            break;
          }
        }
        continue;
      }
      // any other uri comes from QGIS itself (e.g. the browser) and is a loadable dataset
      hasLayer = true;
    }
  }

  const QList<QUrl> urls = mimeData->urls();
  for ( const QUrl &url : urls )
  {
    const QString fileName = url.toLocalFile();
    if ( fileName.isEmpty() )
    {
      // remote url: assume it points to a loadable dataset
      hasLayer = true;
      continue;
    }
    if ( fileName.endsWith( ".qgs"_L1, Qt::CaseInsensitive ) || fileName.endsWith( ".qgz"_L1, Qt::CaseInsensitive ) )
      return Qgis::LayerDropPayloadType::Project;

    if ( fileName.endsWith( ".qlr"_L1, Qt::CaseInsensitive ) )
    {
      // QgsLayerDefinition files insert layers into the tree, exactly like datasets
      hasLayer = true;
      continue;
    }

    if ( !hasLayer )
    {
      const QFileInfo fileInfo( fileName );
      if ( fileInfo.suffix().isEmpty() || fileInfo.isDir() )
      {
        // the fast, extension based check cannot say anything meaningful about
        // extensionless files and directories, while the full dataset open attempted on
        // drop might succeed (e.g. raster file without extension, directory based formats).
        hasLayer = true;
      }
      else
      {
        hasLayer = !QgsProviderRegistry::instance()->querySublayers( fileName, Qgis::SublayerQueryFlag::FastScan ).isEmpty();
      }
    }
  }

  if ( hasLayer )
    return Qgis::LayerDropPayloadType::Layers;

  // a custom uri is consumed by a matching custom drop handler (loadable layers, if any,
  // took precedence above and keep the insertion indicator)
  if ( hasCustomUri )
    return Qgis::LayerDropPayloadType::CustomHandler;

  // no provider can load the payload, but a custom drop handler may still claim the
  // mime data (e.g. .qpt print templates, .py scripts, style .xml files)
  for ( QgsCustomDropHandler *handler : customHandlers )
  {
    if ( handler && handler->canHandleMimeData( mimeData ) )
      return Qgis::LayerDropPayloadType::CustomHandler;
  }

  return Qgis::LayerDropPayloadType::Invalid;
}

bool QgsLayerDropClassifier::isDatasetDrag( const QMimeData *mimeData )
{
  // internal layer tree reordering carries its own mime type and must not be treated as a
  // dataset drag; anything else with file urls or a QGIS uri list is a dataset/layer drag
  return !mimeData->hasFormat( u"application/qgis.layertreemodeldata"_s ) && ( mimeData->hasUrls() || mimeData->hasFormat( u"application/x-vnd.qgis.qgis.uri"_s ) );
}
