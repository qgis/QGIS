/***************************************************************************
  qgsmetalroughmaterialsettings.h
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

#ifndef QGSMETALROUGHMATERIALSETTINGS_H
#define QGSMETALROUGHMATERIALSETTINGS_H

#include "qgis_core.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup core
 * \brief A PBR metal rough shading material used for rendering.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsMetalRoughMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    QgsMetalRoughMaterialSettings() = default;

    QString type() const override;
    QSet< QgsAbstractMaterialSettings::Property > supportedProperties() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the metal rough material.
     */
    static bool supportsTechnique( Qgis::MaterialRenderingTechnique technique );

    /**
     * Returns a new instance of QgsMetalRoughMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsMetalRoughMaterialSettings *clone() const override SIP_FACTORY;
    bool equals( const QgsAbstractMaterialSettings *other ) const override;
    bool requiresTangents() const override;

    /**
     * Returns the base material color.
     *
     * \see setBaseColor()
     */
    QColor baseColor() const { return mBaseColor; }

    /**
     * Returns the material's metalness, as a value between 0 and 1.
     *
     * \see setMetalness()
     */
    double metalness() const { return mMetalness; }

    /**
     * Returns the material's roughness, as a value between 0 and 1.
     *
     * \see setRoughness()
     */
    double roughness() const { return mRoughness; }

    /**
     * Returns the material's reflectance, as a value between 0 and 1.
     *
     * This controls the specular intensity for non-metals.
     *
     * \see setReflectance()
     * \since QGIS 4.2
     */
    double reflectance() const { return mReflectance; }

    /**
     * Returns the material's anisotropy, as a value between 0 and 1.
     *
     * Anisotropic materials are those with properties which vary in different directions.
     *
     * \see anisotropyRotation()
     * \see setAnisotropy()
     * \since QGIS 4.2
     */
    double anisotropy() const { return mAnisotropy; }

    /**
     * Returns the rotation of the material's anisotropy, as a angle in degrees.
     *
     * Anisotropic materials are those with properties which vary in different directions.
     *
     * \see setAnisotropyRotation()
     * \see anisotropy()
     * \since QGIS 4.2
     */
    double anisotropyRotation() const { return mAnisotropyRotation; }

    /**
     * Returns the material's emissive color.
     *
     * \see setEmissionColor()
     * \since QGIS 4.2
     */
    QColor emissionColor() const { return mEmissiveColor; }

    /**
     * Returns the emission factor, which dictates the strength of the emission effect.
     *
     * A value of 1.0 indicates that the emission color values should be used directly.
     * Larger values result in more light emission.
     *
     * \see setEmissionFactor()
     * \see emissionColor()
     *
     * \since QGIS 4.2
     */
    double emissionFactor() const { return mEmissionFactor; }

    /**
     * Returns the opacity of the surface
     *
     * \see setOpacity()
     * \since QGIS 4.2
     */
    double opacity() const { return mOpacity; }

    /**
     * Returns an approximate color representing the blended material color.
     *
     * Since this material contains only a single color, this function
     * simply returns baseColor().
     *
     * \see baseColor()
     *
     * \since QGIS 4.2
     */
    QColor averageColor() const override;

    /**
     * Sets the base material \a color.
     *
     * \see baseColor()
     */
    void setBaseColor( const QColor &color ) { mBaseColor = color; }

    /**
     * Sets the material's \a metalness, as a value between 0 and 1.
     *
     * \see metalness()
     */
    void setMetalness( double metalness ) { mMetalness = metalness; }

    /**
     * Sets the material's \a roughness, as a value between 0 and 1.
     *
     * \see roughness()
     */
    void setRoughness( double roughness ) { mRoughness = roughness; }

    /**
     * Sets the material's \a reflectance, as a value between 0 and 1.
     *
     * This controls the specular intensity for non-metals.
     *
     * \see reflectance()
     * \since QGIS 4.2
     */
    void setReflectance( double reflectance ) { mReflectance = reflectance; }

    /**
     * Sets the material's \a anisotropy, as a value between 0 and 1.
     *
     * Anisotropic materials are those with properties which vary in different directions.
     *
     * \see setAnisotropyRotation()
     * \see anisotropy()
     * \since QGIS 4.2
     */
    void setAnisotropy( double anisotropy ) { mAnisotropy = anisotropy; }

    /**
     * Sets the \a rotation of the material's anisotropy, as a angle in degrees.
     *
     * Anisotropic materials are those with properties which vary in different directions.
     *
     * \see anisotropyRotation()
     * \see setAnisotropy()
     * \since QGIS 4.2
     */
    void setAnisotropyRotation( double rotation ) { mAnisotropyRotation = rotation; }

    /**
     * Sets the \a opacity of the surface.
     *
     * \see opacity()
     * \since QGIS 4.2
     */
    void setOpacity( double opacity ) { mOpacity = opacity; }

    /**
     * Sets the material's emissive \a color.
     *
     * \see emissionColor()
     * \since QGIS 4.2
     */
    void setEmissionColor( const QColor &color ) { mEmissiveColor = color; }

    /**
     * Sets the emission \a factor, which dictates the strength of the emission effect.
     *
     * A value of 1.0 indicates that the emission color values should be used directly.
     * Larger values result in more light emission.
     *
     * \see emissionFactor()
     * \see setEmissionColor()
     *
     * \since QGIS 4.2
     */
    void setEmissionFactor( double factor ) { mEmissionFactor = factor; }

    /**
     * Returns the material's clear coat factor, as a value between 0 and 1.
     *
     * \see setClearCoatFactor()
     * \see clearCoatRoughness()
     * \since QGIS 4.2
     */
    double clearCoatFactor() const { return mClearCoatFactor; }

    /**
     * Sets the material's clear coat \a factor, as a value between 0 and 1.
     *
     * \see clearCoatFactor()
     * \see setClearCoatRoughness()
     * \since QGIS 4.2
     */
    void setClearCoatFactor( double factor ) { mClearCoatFactor = factor; }

    /**
     * Returns the material's clear coat roughness, as a value between 0 and 1.
     *
     * \see setClearCoatRoughness()
     * \see clearCoatFactor()
     * \since QGIS 4.2
     */
    double clearCoatRoughness() const { return mClearCoatRoughness; }

    /**
     * Sets the material's clear coat \a roughness, as a value between 0 and 1.
     *
     * \see clearCoatRoughness()
     * \see setClearCoatFactor()
     * \since QGIS 4.2
     */
    void setClearCoatRoughness( double roughness ) { mClearCoatRoughness = roughness; }

    /**
     * Decomposes a base color into the material's color components, and sets the material's color accordingly.
     *
     * Since this material contains only a single color, this function
     * is equivalent to calling setBaseColor(baseColor).
     *
     * \param baseColor The color to decompose
     *
     * \see setBaseColor()
     *
     * \since QGIS 4.2
     */
    void setColorsFromBase( const QColor &baseColor ) override;

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

    bool operator==( const QgsMetalRoughMaterialSettings &other ) const
    {
      return mBaseColor == other.mBaseColor
             && mEmissiveColor == other.mEmissiveColor
             && qgsDoubleNear( mMetalness, other.mMetalness )
             && qgsDoubleNear( mRoughness, other.mRoughness )
             && qgsDoubleNear( mReflectance, other.mReflectance )
             && qgsDoubleNear( mAnisotropy, other.mAnisotropy )
             && qgsDoubleNear( mAnisotropyRotation, other.mAnisotropyRotation )
             && qgsDoubleNear( mOpacity, other.mOpacity )
             && qgsDoubleNear( mEmissionFactor, other.mEmissionFactor )
             && qgsDoubleNear( mClearCoatFactor, other.mClearCoatFactor )
             && qgsDoubleNear( mClearCoatRoughness, other.mClearCoatRoughness )
             && dataDefinedProperties() == other.dataDefinedProperties();
    }

  private:
    QColor mBaseColor { QColor::fromRgbF( 0.5f, 0.5f, 0.5f, 1.0f ) };
    QColor mEmissiveColor;
    double mMetalness = 0.0;
    double mRoughness = 0.5;
    double mReflectance = 0.5;
    double mAnisotropy = 0.0;
    double mAnisotropyRotation = 0.0;
    double mEmissionFactor = 1.0;
    double mClearCoatFactor = 0.0;
    double mClearCoatRoughness = 0.0;
    double mOpacity = 1.0;
};


#endif // QGSMETALROUGHMATERIALSETTINGS_H
