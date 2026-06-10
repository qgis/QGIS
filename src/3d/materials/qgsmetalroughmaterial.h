/***************************************************************************
  qgsmetalroughmaterial.h
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMETALROUGHMATERIAL_H
#define QGSMETALROUGHMATERIAL_H

#include "qgis.h"
#include "qgis_3d.h"
#include "qgspbrmaterial.h"

#include <QObject>

#define SIP_NO_FILE

// adapted from Qt's qmetalroughmaterial.h
namespace Qt3DRender
{

  class QFilterKey;
  class QEffect;
  class QAbstractTexture;
  class QTechnique;
  class QParameter;
  class QShaderProgram;
  class QShaderProgramBuilder;
  class QRenderPass;

} // namespace Qt3DRender

///@cond PRIVATE

/**
 * \ingroup qgis_3d
 * \brief A PBR metal rough material.
 * \since QGIS 3.36
 */
class _3D_EXPORT QgsMetalRoughMaterial : public QgsPBRMaterial
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsMetalRoughMaterial, with the specified \a parent node.
     */
    explicit QgsMetalRoughMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsMetalRoughMaterial() override;

  public slots:

    //! Set constant metalness value (between 0 - 1.0)
    void setMetalness( float metalness );

    //! Sets the metalness texture. Takes ownership
    void setMetalnessTexture( Qt3DRender::QAbstractTexture *metalness );

    //! Set constant reflectance value (between 0 - 1.0)
    void setReflectance( float reflectance );

    //! Set constant anisotropy value (between 0 - 1.0)
    void setAnisotropy( float anisotropy );

    //! Set constant anisotropy rotation (in degrees)
    void setAnisotropyRotation( float rotation );

    /**
     * Sets the emission texture map. Takes ownership. Set to NULLPTR to remove.
     *
     * \warning Make sure the texture format is correctly set, eg by setting to SRGB format wherever appropriate.
     */
    void setEmissionTexture( Qt3DRender::QAbstractTexture *emission );

    /**
     * Sets the solid emissive \a color.
     */
    void setEmissionColor( const QColor &color );

    //! Sets the emission strength factor
    void setEmissionFactor( double factor );

    //! Sets the clear coat factor
    void setClearCoatFactor( float factor );

    //! Sets the clear coat roughness
    void setClearCoatRoughness( float roughness );

  protected:
    QStringList fragmentShaderDefines() const final;

  private:
    void init();

    Qt3DRender::QParameter *mMetalnessParameter = nullptr;
    Qt3DRender::QParameter *mReflectanceParameter = nullptr;
    Qt3DRender::QParameter *mAnisotropyParameter = nullptr;
    Qt3DRender::QParameter *mAnisotropyRotationParameter = nullptr;
    Qt3DRender::QParameter *mMetalnessMapParameter = nullptr;
    Qt3DRender::QParameter *mEmissionMapParameter = nullptr;
    Qt3DRender::QParameter *mEmissiveColorParameter = nullptr;
    Qt3DRender::QParameter *mEmissionFactorParameter = nullptr;
    Qt3DRender::QParameter *mClearCoatFactorParameter = nullptr;
    Qt3DRender::QParameter *mClearCoatRoughnessParameter = nullptr;

    bool mUsingMetalnessMap = false;
    bool mUsingEmissionMap = false;

    friend class TestQgsGltf3DUtils;
};

///@endcond PRIVATE

#endif // QGSMETALROUGHMATERIAL_H
