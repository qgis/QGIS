/***************************************************************************
  qgsclothmaterialsettings.h
  --------------------------------------
  Date                 : May 2026
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

#ifndef QGSCLOTHMATERIALSETTINGS_H
#define QGSCLOTHMATERIALSETTINGS_H

#include "qgis_core.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup core
 * \brief A PBR cloth shading material used for rendering.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsClothMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    QgsClothMaterialSettings() = default;

    QString type() const override;
    QSet< QgsAbstractMaterialSettings::Property > supportedProperties() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the metal rough material.
     */
    static bool supportsTechnique( Qgis::MaterialRenderingTechnique technique );

    /**
     * Returns a new instance of QgsClothMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsClothMaterialSettings *clone() const override SIP_FACTORY;
    bool equals( const QgsAbstractMaterialSettings *other ) const override;

    /**
     * Returns the base material color.
     *
     * \see setBaseColor()
     */
    QColor baseColor() const { return mBaseColor; }

    /**
     * Returns the base sheen color.
     *
     * \see setSheenColor()
     */
    QColor sheenColor() const { return mSheenColor; }

    /**
     * Returns the material's roughness, as a value between 0 and 1.
     *
     * \see setRoughness()
     */
    double roughness() const { return mRoughness; }

    /**
     * Returns the opacity of the surface
     *
     * \see setOpacity()
     */
    double opacity() const { return mOpacity; }

    QColor averageColor() const override;

    /**
     * Sets the base material \a color.
     *
     * \see baseColor()
     */
    void setBaseColor( const QColor &color ) { mBaseColor = color; }

    /**
     * Sets the material sheen \a color.
     *
     * \see sheenColor()
     */
    void setSheenColor( const QColor &color ) { mSheenColor = color; }

    /**
     * Sets the material's \a roughness, as a value between 0 and 1.
     *
     * \see roughness()
     */
    void setRoughness( double roughness ) { mRoughness = roughness; }

    /**
     * Sets the \a opacity of the surface.
     *
     * \see opacity()
     */
    void setOpacity( double opacity ) { mOpacity = opacity; }

    void setColorsFromBase( const QColor &baseColor ) override;

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

    bool operator==( const QgsClothMaterialSettings &other ) const
    {
      return mBaseColor == other.mBaseColor
             && mSheenColor == other.mSheenColor
             && qgsDoubleNear( mRoughness, other.mRoughness )
             && qgsDoubleNear( mOpacity, other.mOpacity )
             && dataDefinedProperties() == other.dataDefinedProperties();
    }

  private:
    QColor mBaseColor { QColor::fromRgbF( 0.5f, 0.5f, 0.5f, 1.0f ) };
    QColor mSheenColor { QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) };
    double mRoughness = 0.5;
    double mOpacity = 1.0;
};


#endif // QGSCLOTHMATERIALSETTINGS_H
