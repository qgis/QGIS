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

#include "qgis_3d.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup 3d
 * \brief A PBR metal rough shading material used for rendering.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.36
 */
class _3D_EXPORT QgsMetalRoughMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    QgsMetalRoughMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the metal rough material.
     */
    static bool supportsTechnique( QgsMaterialSettingsRenderingTechnique technique );

    /**
     * Returns a new instance of QgsMetalRoughMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsMetalRoughMaterialSettings *clone() const override SIP_FACTORY;
    bool equals( const QgsAbstractMaterialSettings *other ) const override;

    /**
     * Returns the base material color.
     *
     * \see setBaseColor()
     */
    QColor baseColor() const { return mBaseColor; }

    /**
     * Returns the material's metalness.
     *
     * \see setMetalness()
     */
    double metalness() const { return mMetalness; }

    /**
     * Returns the material's roughness.
     *
     * \see setRoughness()
     */
    double roughness() const { return mRoughness; }

    QMap<QString, QString> toExportParameters() const override;

    /**
     * Sets the base material \a color.
     *
     * \see baseColor()
     */
    void setBaseColor( const QColor &color ) { mBaseColor = color; }

    /**
     * Returns the material's \a metalness.
     *
     * \see metalness()
     */
    void setMetalness( double metalness ) { mMetalness = metalness; }

    /**
     * Returns the material's \a roughness.
     *
     * \see roughness()
     */
    void setRoughness( double roughness ) { mRoughness = roughness; }

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

#ifndef SIP_RUN
    QgsMaterial *toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect, const QgsMaterialContext &materialContext ) const override;
#endif

    // TODO c++20 - replace with = default
    bool operator==( const QgsMetalRoughMaterialSettings &other ) const
    {
      return mBaseColor == other.mBaseColor && qgsDoubleNear( mMetalness, other.mMetalness ) && qgsDoubleNear( mRoughness, other.mRoughness ) && dataDefinedProperties() == other.dataDefinedProperties();
    }

  private:
    QColor mBaseColor { QColor::fromRgbF( 0.5f, 0.5f, 0.5f, 1.0f ) };
    double mMetalness = 0.0;
    double mRoughness = 0.0;
};


#endif // QGSMETALROUGHMATERIALSETTINGS_H
