/***************************************************************************
  qgsnullmaterial3dhandler.cpp
  --------------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnullmaterial3dhandler.h"

#include "qgshighlightmaterial.h"
#include "qgsmaterial.h"

#include <QMap>
#include <QString>

using namespace Qt::StringLiterals;


QgsMaterial *QgsNullMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  if ( context.isHighlighted() )
  {
    switch ( technique )
    {
      case Qgis::MaterialRenderingTechnique::Triangles:
      case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
      case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
      case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
      {
        return new QgsHighlightMaterial();
      }
      case Qgis::MaterialRenderingTechnique::InstancedPoints:
      {
        QgsHighlightMaterial *mat = new QgsHighlightMaterial();
        mat->setInstancingEnabled( true, Qgis::InstancedMaterialFlags() );
        return mat;
      }
      case Qgis::MaterialRenderingTechnique::Lines:
      case Qgis::MaterialRenderingTechnique::Points:
      case Qgis::MaterialRenderingTechnique::Billboards:
      {
        // Lines are single color and do not need the highlight material
        // Billboards are not supported yet
        break;
      }
    }
  }

  return nullptr;
}

QMap<QString, QString> QgsNullMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings * ) const
{
  QMap<QString, QString> parameters;
  return parameters;
}

bool QgsNullMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *, const QgsAbstractMaterialSettings *, const QgsMaterialContext & ) const
{
  return true;
}
