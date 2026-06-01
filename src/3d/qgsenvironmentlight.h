/***************************************************************************
  qgsenvironmentlight.h
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

#ifndef QGSENVIRONMENTLIGHT_H
#define QGSENVIRONMENTLIGHT_H

#include "qgis.h"
#include "qgis_3d.h"

#include <QMap>
#include <QString>
#include <QVector>
#include <Qt3DCore/QEntity>

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QParameter;
  class QTextureCubeMap;
  class QTexture2D;
} //namespace Qt3DRender

class QgsImageTexture;
class QgsFrameGraph;

/**
 * \brief An environment light entity.
 *
 * \ingroup qgis_3d
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsEnvironmentLight : public Qt3DCore::QEntity
{
    Q_OBJECT

  public:
    //! Environmental lighting modes
    enum class Mode
    {
      Disabled,                          //!< No environment lighting
      SpecularMapWithSphericalHarmonics, //!< Specular map, using spherical harmonics for irradiance
    };

    //! Constructor
    QgsEnvironmentLight( QgsFrameGraph *frameGraph, QNode *parent = nullptr );

    /**
     * Sets the environment light \a mode.
     */
    void setMode( Mode mode );

    /**
     * Sets the \a strength of the environmental light, as a factor between 0 and 1.
     */
    void setStrength( float strength );

    /**
     * Sets the spherical \a harmonics for irradiant light.
     */
    void setSphericalHarmonics( const QVector<QVector3D> &harmonics );

    /**
     * Sets the specular map texture and available mip levels.
     */
    void setSpecularMap( Qt3DRender::QTextureCubeMap *specularTexture, int mipLevels );

  private:
    Qt3DRender::QParameter *mEnvironmentLightModeParam = nullptr;     // whether environmental lighting is enabled
    Qt3DRender::QParameter *mEnvironmentLightStrengthParam = nullptr; // strength of environmental lighting effect
    Qt3DRender::QParameter *mShParam = nullptr;
    Qt3DRender::QParameter *mSpecularMapParam = nullptr;
    Qt3DRender::QParameter *mMipLevelsParam = nullptr;
    Qt3DRender::QTextureCubeMap *mDummyCubeMap = nullptr;
};


#endif // QGSENVIRONMENTLIGHT_H
