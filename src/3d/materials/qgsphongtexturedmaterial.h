/***************************************************************************
  qgsphongtexturedmaterial.h
  --------------------------------------
  Date                 : August 2024
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

#ifndef QGSPHONGTEXTUREDMATERIAL_H
#define QGSPHONGTEXTUREDMATERIAL_H

#include "qgis_3d.h"
#include "qgsmaterial.h"

#include <QColor>
#include <QObject>
#include <Qt3DRender/QTexture>

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QParameter;

} // namespace Qt3DRender

///@cond PRIVATE

/**
 * \ingroup 3d
 * \brief A diffuseSpecular material adapted from Qt's qdiffusespecularmaterial.h
 * \since QGIS 3.40
 */
class _3D_EXPORT QgsPhongTexturedMaterial : public QgsMaterial
{
    Q_OBJECT
    Q_PROPERTY( QColor ambient READ ambient WRITE setAmbient NOTIFY ambientChanged )
    Q_PROPERTY( Qt3DRender::QAbstractTexture *diffuseTexture READ diffuseTexture WRITE setDiffuseTexture NOTIFY diffuseTextureChanged )
    Q_PROPERTY( float diffuseTextureScale READ diffuseTextureScale WRITE setDiffuseTextureScale NOTIFY diffuseTextureScaleChanged )
    Q_PROPERTY( QColor specular READ specular WRITE setSpecular NOTIFY specularChanged )
    Q_PROPERTY( float shininess READ shininess WRITE setShininess NOTIFY shininessChanged )
    Q_PROPERTY( float opacity READ opacity WRITE setOpacity NOTIFY opacityChanged )

  public:
    /**
     * Constructor for QgsPhongTexturedMaterial, with the specified \a parent node.
     */
    explicit QgsPhongTexturedMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsPhongTexturedMaterial() override;

    QColor ambient() const;
    Qt3DRender::QAbstractTexture *diffuseTexture() const;
    float diffuseTextureScale() const;
    QColor specular() const;
    float shininess() const;
    float opacity() const;

  public Q_SLOTS:
    void setAmbient( const QColor &ambient );

    /**
     * Sets the diffuse component of the material.
     * Ownership is transferred to the material.
     */
    void setDiffuseTexture( Qt3DRender::QAbstractTexture *texture );

    void setDiffuseTextureScale( float textureScale );
    void setSpecular( const QColor &specular );
    void setShininess( float shininess );
    void setOpacity( float opacity );

  Q_SIGNALS:
    void ambientChanged( const QColor &ambient );
    void diffuseTextureChanged( Qt3DRender::QAbstractTexture *diffuseTexture );
    void diffuseTextureScaleChanged( float diffuseTextureScale );
    void specularChanged( const QColor &specular );
    void shininessChanged( float shininess );
    void opacityChanged( float opacity );

  private:
    void init();

    void handleAmbientChanged( const QVariant &var );
    void handleDiffuseTextureChanged( const QVariant &var );
    void handleDiffuseTextureScaleChanged( const QVariant &var );
    void handleSpecularChanged( const QVariant &var );
    void handleShininessChanged( const QVariant &var );
    void handleOpacityChanged( const QVariant &var );

    Qt3DRender::QParameter *mAmbientParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseTextureParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseTextureScaleParameter = nullptr;
    Qt3DRender::QParameter *mSpecularParameter = nullptr;
    Qt3DRender::QParameter *mShininessParameter = nullptr;
    Qt3DRender::QParameter *mOpacityParameter = nullptr;
};

///@endcond PRIVATE

#endif // QGSPHONGTEXTUREDMATERIAL_H
