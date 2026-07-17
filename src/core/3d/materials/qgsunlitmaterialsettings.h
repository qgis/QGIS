/***************************************************************************
  qgsunlitmaterialsettings.h
  --------------------------------------
  Date                 : June 2026
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

#ifndef QGSUNLITMATERIALSETTINGS_H
#define QGSUNLITMATERIALSETTINGS_H

#include "qgis_core.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QgsMaterial;
class QDomElement;

/**
 * \ingroup core
 * \brief Basic shading material used for rendering solid color, unlit objects.
 *
 * Unlit objects are flat shaded, with no interaction with lights in the scene.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsUnlitMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    QgsUnlitMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the material.
     */
    static bool supportsTechnique( Qgis::MaterialRenderingTechnique technique );

    /**
     * Returns a new instance of QgsUnlitMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsUnlitMaterialSettings *clone() const override SIP_FACTORY;
    bool equals( const QgsAbstractMaterialSettings *other ) const override;
    QSet< QgsAbstractMaterialSettings::Property > supportedProperties() const override;

    /**
     * Returns the material color.
     *
     * \see setColor()
     */
    QColor color() const { return mColor; }

    /**
     * Sets the material's \a color.
     *
     * \see color()
     */
    void setColor( const QColor &color ) { mColor = color; }

    QColor averageColor() const override;
    void setColorsFromBase( const QColor &baseColor ) override;

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

    // TODO c++20 - replace with = default
    bool operator==( const QgsUnlitMaterialSettings &other ) const { return mColor == other.mColor && dataDefinedProperties() == other.dataDefinedProperties(); }

  private:
    QColor mColor { QColor::fromRgbF( 0.8f, 0.4f, 0.4f, 1.0f ) };
};


#endif // QGSUNLITMATERIALSETTINGS_H
