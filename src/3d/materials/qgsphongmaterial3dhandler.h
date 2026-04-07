/***************************************************************************
  qgsphongmaterial3dhandler.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPHONGMATERIAL3DHANDLER_H
#define QGSPHONGMATERIAL3DHANDLER_H

#include "qgis_3d.h"
#include "qgsmaterial3dhandler.h"

#define SIP_NO_FILE

class QgsMaterial;

/**
 * \ingroup qgis_3d
 * \brief 3D handler for the Phong shading material.
 *
 * \warning Not available in Python bindings
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsPhongMaterial3DHandler : public QgsAbstractMaterial3DHandler
{
  public:
    QgsPhongMaterial3DHandler() = default;

    QMap<QString, QString> toExportParameters( const QgsAbstractMaterialSettings *settings ) const override;
    QgsMaterial *toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext ) const override;
    QByteArray dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const override;
    int dataDefinedByteStride( const QgsAbstractMaterialSettings *settings ) const override;
    void applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const override;
    bool updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const override;

  private:
    //! Constructs a material from shader files
    QgsMaterial *buildMaterial( const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const;
};


#endif // QGSPHONGMATERIAL3DHANDLER_H
