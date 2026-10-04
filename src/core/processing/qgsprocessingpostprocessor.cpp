/***************************************************************************
                         qgsprocessingpostprocessor.cpp
                         ------------------------------
    begin                : October 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingpostprocessor.h"

#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingdefaultstyleregistry.h"
#include "qgsprocessingregistry.h"
#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsProcessingLayerPostProcessor::~QgsProcessingLayerPostProcessor() = default;

void QgsProcessingLayerPostProcessor::postProcessLayer( QgsMapLayer *layer, const QString &outputName, const QgsProcessingAlgorithm *algorithm )
{
  if ( !layer )
    return;

  QString styleFilePath;

  if ( !outputName.isEmpty() && algorithm )
  {
    styleFilePath = QgsApplication::processingRegistry()->defaultStyleRegistry()->defaultStyleForOutput( algorithm->id(), outputName );
  }

  if ( styleFilePath.isEmpty() )
  {
    QgsSettings settings;

    switch ( layer->type() )
    {
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vectorLayer = qobject_cast< QgsVectorLayer * >( layer );
        switch ( vectorLayer->geometryType() )
        {
          case Qgis::GeometryType::Point:
            styleFilePath = settings.value( u"Processing/Configuration/VECTOR_POINT_STYLE"_s, QString() ).toString();
            break;
          case Qgis::GeometryType::Line:
            styleFilePath = settings.value( u"Processing/Configuration/VECTOR_LINE_STYLE"_s, QString() ).toString();
            break;
          case Qgis::GeometryType::Polygon:
            styleFilePath = settings.value( u"Processing/Configuration/VECTOR_POLYGON_STYLE"_s, QString() ).toString();
            break;
          case Qgis::GeometryType::Unknown:
          case Qgis::GeometryType::Null:
            break;
        }
        break;
      }
      case Qgis::LayerType::Raster:
      {
        styleFilePath = settings.value( u"Processing/Configuration/RASTER_STYLE"_s, QString() ).toString();
        break;
      }
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::VectorTile:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
        break;
    }
  }

  if ( !styleFilePath.isEmpty() )
  {
    bool result = false;
    layer->loadNamedStyle( styleFilePath, result );
  }
}
