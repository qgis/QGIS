/***************************************************************************
  qgsgoochmaterial.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGOOCHMATERIAL_H
#define QGSGOOCHMATERIAL_H

#include "qgis_3d.h"
#include "qgsmaterial.h"

#include <QColor>
#include <QObject>

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QParameter;
  class QShaderProgram;
} // namespace Qt3DRender

///@cond PRIVATE

/**
 * \ingroup qgis_3d
 * \brief A Gooch shading material for use in QGIS 3D views.
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsGoochMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsGoochMaterial, with the specified \a parent node.
     */
    explicit QgsGoochMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsGoochMaterial() override;

  public slots:
    //! Sets diffuse color component, must be a SRGB color
    void setDiffuse( const QColor &diffuse );
    //! Sets specular color component, must be a SRGB color
    void setSpecular( const QColor &specular );
    //! Sets warm color component, must be a SRGB color
    void setWarm( const QColor &warm );
    //! Sets cool color component, must be a SRGB color
    void setCool( const QColor &cool );
    void setShininess( float shininess );
    void setAlpha( float alpha );
    void setBeta( float beta );

    /**
     * Switches between data-defined (per-vertex attribute) and uniform color mode.
     * When \a enabled is TRUE, the goochDataDefined.vert shader is used and
     * the DATA_DEFINED define is injected into the fragment shader.
     */
    void setDataDefinedEnabled( bool enabled );

  private:
    void init();
    void updateShaders();

    Qt3DRender::QParameter *mDiffuseParameter = nullptr;
    Qt3DRender::QParameter *mSpecularParameter = nullptr;
    Qt3DRender::QParameter *mWarmParameter = nullptr;
    Qt3DRender::QParameter *mCoolParameter = nullptr;
    Qt3DRender::QParameter *mShininessParameter = nullptr;
    Qt3DRender::QParameter *mAlphaParameter = nullptr;
    Qt3DRender::QParameter *mBetaParameter = nullptr;
    Qt3DRender::QShaderProgram *mShaderProgram = nullptr;
    bool mDataDefinedEnabled = false;
};

///@endcond PRIVATE

#endif // QGSGOOCHMATERIAL_H
