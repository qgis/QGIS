/***************************************************************************
  qgsmetalroughtexturedmaterial3dhandler.h
  --------------------------------------
  Date                 : April 2026
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

#ifndef QGSMETALROUGHMATERIALTEXTURED3DHANDLER_H
#define QGSMETALROUGHMATERIALTEXTURED3DHANDLER_H

#include "qgis_3d.h"
#include "qgsmaterial3dhandler.h"

#define SIP_NO_FILE

class QgsMetalRoughTexturedMaterialSettings;
class QgsMetalRoughMaterial;

namespace Qt3DRender
{
  class QTexture2D;
}

/**
 * \ingroup qgis_3d
 * \brief 3D handler for the textured PBR metal rough material.
 *
 * \warning Not available in Python bindings
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsMetalRoughTexturedMaterial3DHandler : public QgsAbstractMaterial3DHandler
{
  public:
    QgsMetalRoughTexturedMaterial3DHandler() = default;

    QMap<QString, QString> toExportParameters( const QgsAbstractMaterialSettings *settings ) const override;
    QgsMaterial *toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext ) const override;
    bool updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const override;

  private:
    static Qt3DRender::QTexture2D *loadTexture( const QString &path, bool isSrgb, const QgsMaterialContext &context );

    static void applySettingsToMaterial( const QgsMetalRoughTexturedMaterialSettings *metalRoughTexturedSettings, QgsMetalRoughMaterial *material, const QgsMaterialContext &context );
};


#endif // QGSMETALROUGHMATERIALTEXTURED3DHANDLER_H
