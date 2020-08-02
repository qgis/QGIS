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

#include <QString>
#include <QMap>

#include "qgis_3d.h"

class QgsReadWriteContext;
class QDomElement;

/**
 * \brief class containing the configuration of a skybox entity
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsSkyboxSettings
{
  public:

    //! Reads settings from a DOM \a element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );

    //! Writes settings to a DOM \a element
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    //! Returns whether the skybox is enabled
    bool isSkyboxEnabled() const { return mIsSkyboxEnabled; }
    //! Sets whether the skybox is enabled
    void setIsSkyboxEnabled( bool enabled ) { mIsSkyboxEnabled = enabled; }

    /**
     * Returns the type of the skybox
     * This can be one of 3 types: "Textures collection", "HDR texture", "Distinct Faces"
     */
    QString skyboxType() const { return mSkyboxType; }

    /**
     * Sets the type of the skybox
     * the type can be one of 3 types: "Textures collection", "HDR texture", "Distinct Faces"
     */
    void setSkyboxType( const QString &type ) { mSkyboxType = type; }

    //! Returns the base name of a skybox of type "Textures collection"
    QString skyboxBaseName() const { return mSkyboxBaseName; }
    //! Sets the base name of a skybox of type  "Textures collection"
    void setSkyboxBaseName( const QString &baseName ) { mSkyboxBaseName = baseName; }

    //! Returns the extension of a skybox of type "Textures collection"
    QString skyboxExtension() const { return mSkyboxExt; }
    //! Sets the extension of a skybox of type "Textures collection"
    void setSkyboxExtension( const QString &extension ) { mSkyboxExt = extension; }

    //! Returns the HDR texture path of a skybox of type "HDR texture"
    QString hdrTexturePath() const { return mHDRTexturePath; }
    //! Sets the HDR texture path of a skybox of type "HDR texture"
    void setHdrTexturePath( const QString &texturePath ) { mHDRTexturePath = texturePath; }

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

  private:
    bool mIsSkyboxEnabled = false;
    QString mSkyboxType;
    //
    QString mSkyboxBaseName;
    QString mSkyboxExt;
    //
    QString mHDRTexturePath;
    //
    QMap<QString, QString> mCubeMapFacesPaths;
};

#endif // QGSSKYBOXSETTINGS_H
