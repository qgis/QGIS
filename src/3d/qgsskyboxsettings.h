/***************************************************************************
  qgsskyboxsettings.h
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb uderscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSKYBOXSETTINGS_H
#define QGSSKYBOXSETTINGS_H

#include "qgis_3d.h"
#include "qgsskyboxentity.h"

#include <QMap>
#include <QString>

#define SIP_NO_FILE

class QgsReadWriteContext;
class QDomElement;

// this is broken for z-up coordinate system
#define ENABLE_PANORAMIC_SKYBOX 0

/**
 * \brief Contains the configuration of a skybox entity.
 *
 * \ingroup qgis_3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsSkyboxSettings
{
  public:
    QgsSkyboxSettings() = default;
    QgsSkyboxSettings( const QgsSkyboxSettings &other );
    QgsSkyboxSettings &operator=( QgsSkyboxSettings const &rhs );

    //! Reads settings from a DOM \a element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );
    //! Writes settings to a DOM \a element
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    //! Returns the type of the skybox
    Qgis::SkyboxType skyboxType() const { return mSkyboxType; }
    //! Sets the type of the skybox
    void setSkyboxType( Qgis::SkyboxType type ) { mSkyboxType = type; }

#if ENABLE_PANORAMIC_SKYBOX
    //! Returns the panoramic texture path of a skybox of type "Panormaic skybox"
    QString panoramicTexturePath() const { return mPanoramicTexturePath; }
    //! Sets the panoramic texture path of a skybox of type "Panoramic skybox"
    void setPanoramicTexturePath( const QString &texturePath ) { mPanoramicTexturePath = texturePath; }
#endif

    /**
     * Returns a map containing the path of each texture specified by the user.
     * The map will contain the following keys corresponding to each face "posX", "posY", "posZ", "negX", "negY", "negZ".
     */
    QMap<QString, QString> cubeMapFacesPaths() const { return mCubeMapFacesPaths; }

    /**
     * Sets a face of one of the skybox 6 textures
     * The face parameter needs to be one of the followings: "posX", "posY", "posZ", "negX", "negY", "negZ"
     */
    void setCubeMapFace( const QString &face, const QString &path ) { mCubeMapFacesPaths[face] = path; }

    /**
     * Returns the cube face mapping scheme.
     *
     * \see setCubeMapping()
     * \since QGIS 4.2
     */
    Qgis::SkyboxCubeMapping cubeMapping() const;

    /**
     * Sets the cube face \a mapping scheme.
     *
     * \see cubeMapping()
     * \since QGIS 4.2
     */
    void setCubeMapping( Qgis::SkyboxCubeMapping mapping );

  private:
    Qgis::SkyboxType mSkyboxType = Qgis::SkyboxType::DistinctTextures;

#if ENABLE_PANORAMIC_SKYBOX
    QString mPanoramicTexturePath;
#endif

    Qgis::SkyboxCubeMapping mCubeMapping = Qgis::SkyboxCubeMapping::NativeZUp;
    QMap<QString, QString> mCubeMapFacesPaths;
};

#endif // QGSSKYBOXSETTINGS_H
