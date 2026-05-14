/***************************************************************************
  qgssimplelinematerial3dhandler.h
  --------------------------------------
  Date                 : August 2020
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


#ifndef QGSSIMPLELINEMATERIAL3DHANDLER_H
#define QGSSIMPLELINEMATERIAL3DHANDLER_H

#include "qgis_3d.h"
#include "qgsmaterial3dhandler.h"

#define SIP_NO_FILE

class QgsMaterial;

/**
 * \ingroup qgis_3d
 * \brief 3D handler for the material used for rendering simple lines as solid line components.
 *
 * \warning Not available in Python bindings
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsSimpleLineMaterial3DHandler : public QgsAbstractMaterial3DHandler
{
  public:
    QgsSimpleLineMaterial3DHandler() = default;

    QMap<QString, QString> toExportParameters( const QgsAbstractMaterialSettings *settings ) const override;
    QgsMaterial *toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext ) const override;
    QByteArray dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const override;
    void applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const override;
    QList< PreviewMeshType > previewMeshTypes() const override;
    Qt3DCore::QEntity *createPreviewMesh( const QString &type, Qt3DCore::QEntity *parent ) const override;
    Qt3DCore::QEntity *createPreviewScene( const QgsAbstractMaterialSettings *settings, const QString &type, const QgsMaterialContext &context, QWindow *window, Qt3DCore::QEntity *parent ) const override;
    bool updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const override;
};


#endif // QGSSIMPLELINEMATERIAL3DHANDLER_H
