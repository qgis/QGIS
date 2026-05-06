/***************************************************************************
  qgsmetalroughmaterial3dhandler.h
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

#ifndef QGSMETALROUGHMATERIAL3DHANDLER_H
#define QGSMETALROUGHMATERIAL3DHANDLER_H

#include "qgis_3d.h"
#include "qgsmaterial3dhandler.h"

#define SIP_NO_FILE

class QgsMetalRoughMaterialSettings;
class QgsMetalRoughMaterial;


/**
 * \ingroup qgis_3d
 * \brief 3D handler for the PBR metal rough material.
 *
 * \warning Not available in Python bindings
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsMetalRoughMaterial3DHandler : public QgsAbstractMaterial3DHandler
{
  public:
    QgsMetalRoughMaterial3DHandler() = default;

    QMap<QString, QString> toExportParameters( const QgsAbstractMaterialSettings *settings ) const override;
    QgsMaterial *toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext ) const override;
    bool updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const override;

  private:
    static void applySettingsToMaterial( const QgsMetalRoughMaterialSettings *metalRoughSettings, QgsMetalRoughMaterial *material, const QgsMaterialContext &context );
};


#endif // QGSMETALROUGHMATERIAL3DHANDLER_H
