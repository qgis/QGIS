/***************************************************************************
  qgspreviewquad.h
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPREVIEWQUAD_H
#define QGSPREVIEWQUAD_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Material component for QgsPreviewQuad object
 *
 * \note Not available in Python bindings
 * \since QGIS 3.16
 */
class QgsPreviewQuadMaterial : public Qt3DRender::QMaterial
{
  public:
    //! Constructor
    QgsPreviewQuadMaterial( Qt3DRender::QAbstractTexture *texture, QVector<Qt3DRender::QParameter *> additionalShaderParameters = QVector<Qt3DRender::QParameter *>(), QNode *parent = nullptr );

    //! Sets the view port of the quad
    void setViewPort( QVector2D centerTexCoords, QVector2D sizeTexCoords );
  private:
    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QParameter *mTextureParameter = nullptr;
    Qt3DRender::QParameter *mCenterTextureCoords = nullptr;
    Qt3DRender::QParameter *mSizeTextureCoords = nullptr;

};

/**
 * \ingroup 3d
 * \brief Rectangular quad entity used for debugging depth maps
 *
 * \note Not available in Python bindings
 * \since QGIS 3.16
 */
class QgsPreviewQuad : public Qt3DCore::QEntity
{
  public:

    /**
     * \brief Construct an object that displays a texture for debugging purposes (example: depth buffer)
     * \param texture The texture to be rendered
     * \param centerNDC The center of the texture in opnegl normalized device coordinates
     *                  ie. Bottom left of the viewport is (-1, -1), the top right of the viewport is (1, 1)
     * \param size The size of the displayed rectangle
     * \param additionalShaderParameters More parameters to pass to the shader
     * \param parent The parent of the quad
     */
    QgsPreviewQuad( Qt3DRender::QAbstractTexture *texture, const QPointF &centerNDC, const QSizeF &size, QVector<Qt3DRender::QParameter *> additionalShaderParameters = QVector<Qt3DRender::QParameter *>(), Qt3DCore::QEntity *parent = nullptr );

    //! Sets where the quad will be located on the scene
    void setViewPort( const QPointF &centerNDC, const QSizeF &size );
  private:
    QgsPreviewQuadMaterial *mMaterial = nullptr;
};

#endif // QGSPREVIEWQUAD_H
