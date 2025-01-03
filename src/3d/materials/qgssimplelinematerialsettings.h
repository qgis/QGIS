/***************************************************************************
  qgssimplelinematerialsettings.h
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSIMPLELINEMATERIALSETTINGS_H
#define QGSSIMPLELINEMATERIALSETTINGS_H

#include "qgis_3d.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsmaterial.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup 3d
 * \brief Basic shading material used for rendering simple lines as solid line components.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsSimpleLineMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    QgsSimpleLineMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the material.
     */
    static bool supportsTechnique( QgsMaterialSettingsRenderingTechnique technique );

    /**
     * Returns a new instance of QgsSimpleLineMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsSimpleLineMaterialSettings *clone() const override SIP_FACTORY;
    bool equals( const QgsAbstractMaterialSettings *other ) const override;

    /**
     * Returns the ambient color component.
     *
     * \see setAmbient()
     */
    QColor ambient() const { return mAmbient; }

    /**
     * Sets the \a ambient color component.
     *
     * \see ambient()
     */
    void setAmbient( const QColor &ambient ) { mAmbient = ambient; }

    QMap<QString, QString> toExportParameters() const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
#ifndef SIP_RUN
    QgsMaterial *toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect, const QgsMaterialContext &materialContext ) const override;
    QByteArray dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const override;
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    void applyDataDefinedToGeometry( Qt3DRender::QGeometry *geometry, int vertexCount, const QByteArray &data ) const override;
#else
    void applyDataDefinedToGeometry( Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const override;
#endif
#endif

    // TODO c++20 - replace with = default
    bool operator==( const QgsSimpleLineMaterialSettings &other ) const
    {
      return mAmbient == other.mAmbient && dataDefinedProperties() == other.dataDefinedProperties();
    }

  private:
    QColor mAmbient { QColor::fromRgbF( 0.1f, 0.1f, 0.1f, 1.0f ) };
};


#endif // QGSSIMPLELINEMATERIALSETTINGS_H
