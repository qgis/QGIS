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

/**
 * \brief base class for all skybox types
 * It holds the commun member data between different skybox entity types
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsSkyboxEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructor
    QgsSkyboxEntity( QNode *parent = nullptr );

  protected:
    Qt3DRender::QEffect *mEffect;
    Qt3DRender::QMaterial *mMaterial;
    Qt3DRender::QTechnique *mGl3Technique;
    Qt3DRender::QFilterKey *mFilterKey;
    Qt3DRender::QRenderPass *mGl3RenderPass;
    Qt3DExtras::QCuboidMesh *mMesh;
    Qt3DRender::QParameter *mGammaStrengthParameter;
    Qt3DRender::QParameter *mTextureParameter;
};

/**
 * \brief a skybox constructed from a 360 HDR image
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsHDRSkyboxEntity : public QgsSkyboxEntity
{
  public:
    //! Construct a skybox from a high resolution 360 image
    QgsHDRSkyboxEntity( const QString &hdrTexturePath, Qt3DCore::QNode *parent = nullptr );

    //! Returns the path of the current texture in use
    QString getHDRTexturePath() const { return mHDRTexturePath; }
  private:
    void reloadTexture();
  private:
    QString mHDRTexturePath;
    Qt3DRender::QTextureLoader *mLoadedTexture;
    Qt3DRender::QShaderProgram *mGlShader;
};

/**
 * \brief a skybox constructed from a 6 cube faces
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsCubeFacesSkyboxEntity : public QgsSkyboxEntity
{
  public:
    //! Constructs a skybox from 6 different images
    QgsCubeFacesSkyboxEntity( const QString &posX, const QString &posY, const QString &posZ, const QString &negX, const QString &negY, const QString &negZ, Qt3DCore::QNode *parent = nullptr );

    /**
     * Constructs a skybox from a collection of images
     * The images in the source directory should match the pattern:
     * baseName + * "_posx|_posy|_posz|_negx|_negy|_negz" + extension
     */
    QgsCubeFacesSkyboxEntity( const QString &baseName, const QString &extension, Qt3DCore::QNode *parent = nullptr );

  private:
    void init();
    void reloadTexture();
  private:
    QMap<Qt3DRender::QTextureCubeMap::CubeMapFace, QString> mCubeFacesPaths;
    Qt3DRender::QShaderProgram *mGlShader;
    QVector<Qt3DRender::QTextureImage *> mFacesTextureImages;
    Qt3DRender::QTextureCubeMap *mCubeMap;
};

#endif // QGSSKYBOXENTITY_H
