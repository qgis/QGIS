/***************************************************************************
  qgsmetalroughmaterial3dhandler.cpp
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetalroughmaterial3dhandler.h"

#include "qgshighlightmaterial.h"
#include "qgsmetalroughmaterial.h"
#include "qgsmetalroughmaterialsettings.h"

#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QParameter>

using namespace Qt::StringLiterals;

QgsMaterial *QgsMetalRoughMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  const QgsMetalRoughMaterialSettings *metalRoughSettings = dynamic_cast< const QgsMetalRoughMaterialSettings * >( settings );
  Q_ASSERT( metalRoughSettings );

  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    {
      if ( context.isHighlighted() )
      {
        return new QgsHighlightMaterial( technique );
      }

      QgsMetalRoughMaterial *material = new QgsMetalRoughMaterial;
      material->setObjectName( u"metalRoughMaterial"_s );
      applySettingsToMaterial( metalRoughSettings, material, context );
      return material;
    }

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return nullptr;
  }
  return nullptr;
}

QMap<QString, QString> QgsMetalRoughMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings * ) const
{
  QMap<QString, QString> parameters;
  return parameters;
}

void QgsMetalRoughMaterial3DHandler::addParametersToEffect( Qt3DRender::QEffect *, const QgsAbstractMaterialSettings *, const QgsMaterialContext & ) const
{}

bool QgsMetalRoughMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const
{
  const QgsMetalRoughMaterialSettings *metalRoughSettings = qgis::down_cast< const QgsMetalRoughMaterialSettings * >( settings );

  QgsMetalRoughMaterial *material = sceneRoot->findChild<QgsMetalRoughMaterial *>();
  if ( material->objectName() != "metalRoughMaterial"_L1 )
    return false;

  applySettingsToMaterial( metalRoughSettings, material, context );
  return true;
}

void QgsMetalRoughMaterial3DHandler::applySettingsToMaterial( const QgsMetalRoughMaterialSettings *metalRoughSettings, QgsMetalRoughMaterial *material, const QgsMaterialContext &context )
{
  material->setBaseColor( context.isSelected() ? context.selectionColor() : metalRoughSettings->baseColor() );
  material->setMetalness( static_cast< float >( metalRoughSettings->metalness() ) );
  material->setRoughness( static_cast< float >( metalRoughSettings->roughness() ) );
  material->setOpacity( static_cast< float >( metalRoughSettings->opacity() ) );
}
