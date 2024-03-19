/***************************************************************************
  qgsdiffusespecularmaterial.h
  --------------------------------------
  Date                 : March 2024
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

#ifndef QGSDIFFUSESPECULARMATERIAL_H
#define QGSDIFFUSESPECULARMATERIAL_H

#include "qgis_3d.h"

#include <QColor>
#include <QObject>
#include <Qt3DRender/QMaterial>

#define SIP_NO_FILE

// adapted from Qt's qdiffusespecularmaterial.h
namespace Qt3DRender
{
  class QEffect;
  class QFilterKey;
  class QTechnique;
  class QParameter;
  class QShaderProgram;
  class QRenderPass;

} // namespace Qt3DRender

///@cond PRIVATE

/**
 * \ingroup 3d
 * \brief A diffuseSpecular material which discard transparent components.
 * \since QGIS 3.40
 */
class _3D_EXPORT QgsDiffuseSpecularMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT
    Q_PROPERTY( QColor ambient READ ambient WRITE setAmbient NOTIFY ambientChanged )
    Q_PROPERTY( QVariant diffuse READ diffuse WRITE setDiffuse NOTIFY diffuseChanged )
    Q_PROPERTY( QColor specular READ specular WRITE setSpecular NOTIFY specularChanged )
    Q_PROPERTY( float shininess READ shininess WRITE setShininess NOTIFY shininessChanged )

  public:

    /**
     * Constructor for QgsDiffuseSpecularMaterial, with the specified \a parent node.
     */
    explicit QgsDiffuseSpecularMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsDiffuseSpecularMaterial() override;

    QColor ambient() const;
    QVariant diffuse() const;
    QColor specular() const;
    float shininess() const;

  public Q_SLOTS:
    void setAmbient( const QColor &ambient );
    void setDiffuse( const QVariant &diffuse );
    void setSpecular( const QColor &specular );
    void setShininess( float shininess );

  Q_SIGNALS:
    void ambientChanged( const QColor &ambient );
    void diffuseChanged( const QVariant &diffuse );
    void specularChanged( const QColor &specular );
    void shininessChanged( float shininess );

  private:
    void init();

    void handleAmbientChanged( const QVariant &var );
    void handleDiffuseChanged( const QVariant &var );
    void handleSpecularChanged( const QVariant &var );
    void handleShininessChanged( const QVariant &var );

    Qt3DRender::QEffect *mEffect = nullptr;

    Qt3DRender::QParameter *mAmbientParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseTextureParameter = nullptr;
    Qt3DRender::QParameter *mSpecularParameter = nullptr;
    Qt3DRender::QParameter *mShininessParameter = nullptr;
    Qt3DRender::QTechnique *mGL3Technique = nullptr;
    Qt3DRender::QRenderPass *mGL3RenderPass = nullptr;
    Qt3DRender::QShaderProgram *mGL3Shader = nullptr;
    Qt3DRender::QFilterKey *mFilterKey = nullptr;
};

///@endcond PRIVATE

#endif // QGSDIFFUSESPECULARMATERIAL_H
