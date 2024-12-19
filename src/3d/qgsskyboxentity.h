/***************************************************************************
  qgsskyboxentity.h
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

#ifndef QGSSKYBOXENTITY_H
#define QGSSKYBOXENTITY_H

#include <Qt3DCore/QEntity>
#include <QVector3D>
#include <Qt3DRender/QTexture>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QRenderPass>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DRender/QParameter>

#include "qgis_3d.h"

class QgsImageTexture;

#define SIP_NO_FILE

/**
 * \brief Base class for all skybox types.
 *
 * It holds the common member data between different skybox entity types
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsSkyboxEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Skybox type enumeration
    enum SkyboxType
    {
      PanoramicSkybox,
      DistinctTexturesSkybox
    };
  public:
    //! Constructor
    QgsSkyboxEntity( QNode *parent = nullptr );

    //! Returns the type of the current skybox
    virtual SkyboxType type() const = 0;

  protected:
    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QMaterial *mMaterial = nullptr;
    Qt3DRender::QTechnique *mGl3Technique = nullptr;
    Qt3DRender::QFilterKey *mFilterKey = nullptr;
    Qt3DRender::QRenderPass *mGl3RenderPass = nullptr;
    Qt3DExtras::QCuboidMesh *mMesh = nullptr;
    Qt3DRender::QParameter *mGammaStrengthParameter = nullptr;
    Qt3DRender::QParameter *mTextureParameter = nullptr;
};

/**
 * \brief A skybox constructed from a panoramic image.
 *
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsPanoramicSkyboxEntity : public QgsSkyboxEntity
{
    Q_OBJECT

  public:
    //! Construct a skybox from a panoramic 360 image
    QgsPanoramicSkyboxEntity( const QString &texturePath, Qt3DCore::QNode *parent = nullptr );

    //! Returns the path of the current texture in use
    QString texturePath() const { return mTexturePath; }
    //! Returns the type of the current skybox
    SkyboxType type() const override { return SkyboxType::PanoramicSkybox; }

  private:
    void reloadTexture();
  private:
    QString mTexturePath;
    Qt3DRender::QTextureLoader *mLoadedTexture = nullptr;
    Qt3DRender::QShaderProgram *mGlShader = nullptr;
};

/**
 * \brief A skybox constructed from a 6 cube faces.
 *
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsCubeFacesSkyboxEntity : public QgsSkyboxEntity
{
    Q_OBJECT

  public:
    //! Constructs a skybox from 6 different images
    QgsCubeFacesSkyboxEntity( const QString &posX, const QString &posY, const QString &posZ, const QString &negX, const QString &negY, const QString &negZ, Qt3DCore::QNode *parent = nullptr );

    //! Returns the type of the current skybox
    SkyboxType type() const override { return SkyboxType::DistinctTexturesSkybox; }

  private:
    void init();
    void reloadTexture();
  private:
    QMap<Qt3DRender::QTextureCubeMap::CubeMapFace, QString> mCubeFacesPaths;
    Qt3DRender::QShaderProgram *mGlShader = nullptr;
    QVector<Qt3DRender::QTextureImage *> mFacesTextureImages;
    Qt3DRender::QTextureCubeMap *mCubeMap = nullptr;
};

#endif // QGSSKYBOXENTITY_H
