/***************************************************************************
  qgsphongmaterial.h
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

#ifndef QGSPHONGMATERIAL_H
#define QGSPHONGMATERIAL_H

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
 * \brief A Phong shading material for use in QGIS 3D views.
 * Supports both uniform colors and data-defined per-vertex colors.
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsPhongMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsPhongMaterial, with the specified \a parent node.
     */
    explicit QgsPhongMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsPhongMaterial() override;

  public slots:
    //! Sets ambient color component, must be a SRGB color
    void setAmbient( const QColor &ambient, float scaleFactor = 1 );
    //! Sets diffuse color component, must be a SRGB color
    void setDiffuse( const QColor &diffuse, float scaleFactor = 1 );
    //! Sets specular color component, must be a SRGB color
    void setSpecular( const QColor &specular, float scaleFactor = 1 );
    void setShininess( float shininess );
    void setOpacity( float opacity );

    /**
     * Switches between data-defined (per-vertex attribute) and uniform color mode.
     * When \a enabled is TRUE, the phongDataDefined.vert shader is used and
     * the DATA_DEFINED define is injected into the fragment shader.
     */
    void setDataDefinedEnabled( bool enabled );

  private:
    void init();
    void updateShaders();

    Qt3DRender::QParameter *mAmbientParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseParameter = nullptr;
    Qt3DRender::QParameter *mSpecularParameter = nullptr;
    Qt3DRender::QParameter *mShininessParameter = nullptr;
    Qt3DRender::QParameter *mOpacityParameter = nullptr;
    Qt3DRender::QShaderProgram *mShaderProgram = nullptr;
    bool mDataDefinedEnabled = false;
};

///@endcond PRIVATE

#endif // QGSPHONGMATERIAL_H
