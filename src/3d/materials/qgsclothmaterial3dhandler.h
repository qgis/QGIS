/***************************************************************************
  qgsclothmaterial3dhandler.h
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLOTHMATERIAL3DHANDLER_H
#define QGSCLOTHMATERIAL3DHANDLER_H

#include "qgis_3d.h"
#include "qgsmaterial3dhandler.h"

#define SIP_NO_FILE

class QgsClothMaterialSettings;
class QgsClothMaterial;


/**
 * \ingroup qgis_3d
 * \brief 3D handler for the PBR cloth material.
 *
 * \warning Not available in Python bindings
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsClothMaterial3DHandler : public QgsAbstractMaterial3DHandler
{
  public:
    QgsClothMaterial3DHandler() = default;

    QMap<QString, QString> toExportParameters( const QgsAbstractMaterialSettings *settings ) const override;
    QgsMaterial *toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    QgsMaterial *toInstancedMaterial( const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context, Qgis::InstancedMaterialFlags flags ) const override;
    bool updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const override;

    QByteArray dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const override;
    void applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const override;

  private:
    static void applySettingsToMaterial( const QgsClothMaterialSettings *clothSettings, QgsClothMaterial *material, const QgsMaterialContext &context );
};


#endif // QGSCLOTHMATERIAL3DHANDLER_H
