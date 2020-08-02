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

class QgsSkyboxEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
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

class QgsHDRSkyboxEntity : public QgsSkyboxEntity
{
  public:
    QgsHDRSkyboxEntity( const QString &hdrTexturePath, Qt3DCore::QNode *parent = nullptr );

    QString getHDRTexturePath() const { return mHDRTexturePath; }
  private:
    void reloadTexture();
  private:
    QString mHDRTexturePath;
    Qt3DRender::QTextureLoader *mLoadedTexture;
    Qt3DRender::QShaderProgram *mGlShader;
};

class QgsCubeFacesSkyboxEntity : public QgsSkyboxEntity
{
  public:
    QgsCubeFacesSkyboxEntity( const QString &posX, const QString &posY, const QString &posZ, const QString &negX, const QString &negY, const QString &negZ, Qt3DCore::QNode *parent = nullptr );
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
