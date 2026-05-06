/***************************************************************************
  qgsmetalroughtexturedmaterialsettings.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMETALROUGHTEXTUREDMATERIALSETTINGS_H
#define QGSMETALROUGHTEXTUREDMATERIALSETTINGS_H

#include "qgis_core.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup core
 * \brief A PBR metal rough shading material used for rendering with support for image texture maps.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsMetalRoughTexturedMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    QgsMetalRoughTexturedMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the metal rough material.
     */
    static bool supportsTechnique( Qgis::MaterialRenderingTechnique technique );

    /**
     * Returns a new instance of QgsMetalRoughTexturedMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsMetalRoughTexturedMaterialSettings *clone() const override SIP_FACTORY;
    bool equals( const QgsAbstractMaterialSettings *other ) const override;

    /**
     * Returns the path to the base color texture map.
     *
     * \see setBaseColorTexturePath()
     */
    QString baseColorTexturePath() const { return mBaseColorTexturePath; }

    /**
     * Returns the path to the metalness texture map.
     *
     * \see setMetalnessTexturePath()
     */
    QString metalnessTexturePath() const { return mMetalnessTexturePath; }

    /**
     * Returns the path to the roughness texture map.
     *
     * \see setRoughnessTexturePath()
     */
    QString roughnessTexturePath() const { return mRoughnessTexturePath; }

    /**
     * Returns the path to the normal texture map.
     *
     * \see setNormalTexturePath()
     */
    QString normalTexturePath() const { return mNormalTexturePath; }

    /**
     * Returns the path to the height texture map.
     *
     * \see setHeightTexturePath()
     */
    QString heightTexturePath() const { return mHeightTexturePath; }

    /**
     * Returns the parallax scale, which dictates the strength of the height displacement effect.
     *
     * \see setParallaxScale()
     * \see heightTexturePath()
     */
    double parallaxScale() const { return mParallaxScale; }

    /**
     * Returns the path to the emission/luminosity texture map.
     *
     * \see emissionFactor()
     * \see setEmissionTexturePath()
     */
    QString emissionTexturePath() const { return mEmissionTexturePath; }

    /**
     * Returns the emission factor, which dictates the strength of the emission effect.
     *
     * A value of 1.0 indicates that the emission texture values should be used directly.
     * Larger values result in more light emission.
     *
     * \see setEmissionFactor()
     * \see emissionTexturePath()
     */
    double emissionFactor() const { return mEmissionFactor; }

    /**
     * Returns the path to the ambient occlusion texture map.
     *
     * \see setAmbientOcclusionTexturePath()
     */
    QString ambientOcclusionTexturePath() const { return mAmbientOcclusionTexturePath; }

    /**
     * Returns the texture scale.
     *
     * The texture scale changes the size of the displayed texture in the 3D scene.
     * If the texture scale is less than 1, the texture will be stretched.
     *
     * \see setTextureScale()
     */
    double textureScale() const { return mTextureScale; }

    /**
     * Returns the texture rotation, in degrees.
     *
     * \see setTextureRotation()
     */
    double textureRotation() const { return mTextureRotation; }

    /**
     * Returns the opacity of the surface
     *
     * \see setOpacity()
     * \since QGIS 4.2
     */
    double opacity() const { return mOpacity; }

    /**
     * Sets the \a path to the base color texture map.
     *
     * \see baseColorTexturePath()
     */
    void setBaseColorTexturePath( const QString &path ) { mBaseColorTexturePath = path; }

    /**
     * Sets the \a path to the metalness texture map.
     *
     * \see metalnessTexturePath()
     */
    void setMetalnessTexturePath( const QString &path ) { mMetalnessTexturePath = path; }

    /**
     * Sets the \a path to the roughness texture map.
     *
     * \see roughnessTexturePath()
     */
    void setRoughnessTexturePath( const QString &path ) { mRoughnessTexturePath = path; }

    /**
     * Sets the \a path to the normal texture map.
     *
     * \see normalTexturePath()
     */
    void setNormalTexturePath( const QString &path ) { mNormalTexturePath = path; }

    /**
     * Sets the \a path to the height texture map.
     *
     * \see heightTexturePath()
     */
    void setHeightTexturePath( const QString &path ) { mHeightTexturePath = path; }

    /**
     * Sets the parallax \a scale, which dictates the strength of the height displacement effect.
     *
     * \see parallaxScale()
     * \see setHeightTexturePath()
     */
    void setParallaxScale( double scale ) { mParallaxScale = scale; }

    /**
     * Sets the \a path to the ambient occlusion texture map.
     *
     * \see ambientOcclusionTexturePath()
     */
    void setAmbientOcclusionTexturePath( const QString &path ) { mAmbientOcclusionTexturePath = path; }

    /**
     * Sets the \a path to the emission/luminosity texture map.
     *
     * \see setEmissionFactor()
     * \see emissionTexturePath()
     */
    void setEmissionTexturePath( const QString &path ) { mEmissionTexturePath = path; }

    /**
     * Sets the emission \a factor, which dictates the strength of the emission effect.
     *
     * A value of 1.0 indicates that the emission texture values should be used directly.
     * Larger values result in more light emission.
     *
     * \see emissionFactor()
     * \see setEmissionTexturePath()
     */
    void setEmissionFactor( double factor ) { mEmissionFactor = factor; }

    /**
     * Sets the texture \a scale.
     *
     * The texture scale changes the size of the displayed texture in the 3D scene.
     * If the texture scale is less than 1, the texture will be stretched.
     *
     * \see textureScale()
     */
    void setTextureScale( double scale ) { mTextureScale = scale; }

    /**
     * Sets the texture \a rotation, in degrees.
     *
     * \see textureRotation()
     */
    void setTextureRotation( double rotation ) { mTextureRotation = rotation; }

    /**
     * Sets the \a opacity of the surface.
     *
     * \see opacity()
     * \since QGIS 4.2
     */
    void setOpacity( double opacity ) { mOpacity = opacity; }

    bool requiresTextureCoordinates() const override;
    bool requiresTangents() const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

    bool operator==( const QgsMetalRoughTexturedMaterialSettings &other ) const
    {
      return mBaseColorTexturePath == other.mBaseColorTexturePath
             && mMetalnessTexturePath == other.mMetalnessTexturePath
             && mRoughnessTexturePath == other.mRoughnessTexturePath
             && mNormalTexturePath == other.mNormalTexturePath
             && mAmbientOcclusionTexturePath == other.mAmbientOcclusionTexturePath
             && mHeightTexturePath == other.mHeightTexturePath
             && mEmissionTexturePath == other.mEmissionTexturePath
             && qgsDoubleNear( mTextureScale, other.mTextureScale )
             && qgsDoubleNear( mTextureRotation, other.mTextureRotation )
             && qgsDoubleNear( mEmissionFactor, other.mEmissionFactor )
             && qgsDoubleNear( mParallaxScale, other.mParallaxScale )
             && qgsDoubleNear( mOpacity, other.mOpacity )
             && dataDefinedProperties() == other.dataDefinedProperties();
    }

  private:
    QString mBaseColorTexturePath;
    QString mMetalnessTexturePath;
    QString mRoughnessTexturePath;
    QString mNormalTexturePath;
    QString mHeightTexturePath;
    double mParallaxScale { 0.1 };

    QString mAmbientOcclusionTexturePath;

    QString mEmissionTexturePath;
    double mEmissionFactor { 1.0 };

    double mTextureScale { 1.0 };
    double mTextureRotation { 0.0 };
    double mOpacity { 1.0 };
};


#endif // QGSMETALROUGHTEXTUREDMATERIALSETTINGS_H
