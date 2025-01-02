/***************************************************************************
  qgsmaterial.h
  --------------------------------------
  Date                 : July 2024
  Copyright            : (C) 2024 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMATERIAL_H
#define QGSMATERIAL_H

#define SIP_NO_FILE

#include "qgis_3d.h"

#include <Qt3DRender/QMaterial>
#include <QList>
#include <QVector4D>

namespace Qt3DRender
{
  class QEffect;
}

/**
 * \ingroup 3d
 * \brief Base class for all materials used within QGIS 3D views.
 * It provides common functionality (such as clipping) that all materials should support (where applicable).
 * \since QGIS 3.40
 */
class _3D_EXPORT QgsMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsMaterial, with the specified \a parent node.
     */
    explicit QgsMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsMaterial() override;

    /**
     * Adds two uniform parameters to define OpenGL clipping from \a clipPlanesEquations.
     * It also adds a define macro appropriate to vertex/geometry shaders.
     *
     * \since QGIS 3.40
    */
    void enableClipping( const QList<QVector4D> &clipPlanesEquations );

    /**
     * Removes the uniform parameters used to define OpenGL clipping.
     * It also removes the define macro used for clipping from vertex/geometry shaders.
     * If clipping was not enabled, nothing happens.
     *
     * \since QGIS 3.40
     */
    void disableClipping();

  private:
    //! The name of the QParameter which contains the plane equations
    static const QString CLIP_PLANE_ARRAY_PARAMETER_NAME;
    //! The name of the QParameter which contains the number of planes
    static const QString CLIP_PLANE_MAX_PLANE_PARAMETER_NAME;
    //! The name of the define macro which enables clipping
    static const QString CLIP_PLANE_DEFINE;

    bool mClippingEnabled = false;
};

#endif // QGSMATERIAL_H
