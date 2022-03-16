/***************************************************************************
                         qgsvectorlayerelevationproperties.h
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSVECTORLAYERELEVATIONPROPERTIES_H
#define QGSVECTORLAYERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsmaplayerelevationproperties.h"

/**
 * \class QgsVectorLayerElevationProperties
 * \ingroup core
 * \brief Vector layer specific subclass of QgsMapLayerElevationProperties.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsVectorLayerElevationProperties : public QgsMapLayerElevationProperties
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorLayerElevationProperties, with the specified \a parent object.
     */
    QgsVectorLayerElevationProperties( QObject *parent SIP_TRANSFERTHIS );

    bool hasElevation() const override;
    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    bool isVisibleInZRange( const QgsDoubleRange &range ) const override;
    QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const override;

    /**
     * Returns the altitude clamping method, which dictates how feature heights are interpreted
     * with respect to terrain heights.
     *
     * \see setClamping()
     */
    Qgis::AltitudeClamping clamping() const { return mClamping; }

    /**
     * Sets the altitude \a clamping method, which dictates how feature heights are interpreted
     * with respect to terrain heights.
     *
     * \see clamping()
     */
    void setClamping( Qgis::AltitudeClamping clamping ) { mClamping = clamping; }

    /**
     * Returns the altitude binding method, which determines how altitude is bound to individual vertices in features.
     *
     * \note Binding only relevant for line or polygon feature types -- it is not applicable for point layers.
     *
     * \see setBinding()
     */
    Qgis::AltitudeBinding binding() const { return mBinding; }

    /**
     * Sets the altitude \a binding method, which determines how altitude is bound to individual vertices in features.
     *
     * \note Binding only relevant for line or polygon feature types -- it is not applicable for point layers.
     *
     * \see binding()
     */
    void setBinding( Qgis::AltitudeBinding binding ) { mBinding = binding; }

    /**
     * Returns TRUE if extrusion is enabled.
     *
     * \see setExtrusionEnabled()
     * \see extrusionHeight()
     */
    bool extrusionEnabled() const { return mEnableExtrusion; }

    /**
     * Sets whether extrusion is \a enabled.
     *
     * \see extrusionEnabled()
     * \see setExtrusionHeight()
     */
    void setExtrusionEnabled( bool enabled ) { mEnableExtrusion = enabled; }

    /**
     * Returns the feature extrusion height.
     *
     * \warning extrusion is only applied if extrusionEnabled() is TRUE.
     * \note the zScale() factor is NOT applied to extrusion heights.
     *
     * \see setExtrusionHeight()
     */
    double extrusionHeight() const { return mExtrusionHeight; }

    /**
     * Sets the feature extrusion height.
     *
     * \warning extrusion is only applied if extrusionEnabled() is TRUE.
     * \note the zScale() factor is NOT applied to extrusion heights.
     *
     * \see extrusionHeight()
     */
    void setExtrusionHeight( double height ) { mExtrusionHeight = height; }

  private:

    Qgis::AltitudeClamping mClamping = Qgis::AltitudeClamping::Terrain;
    Qgis::AltitudeBinding mBinding = Qgis::AltitudeBinding::Centroid;

    bool mEnableExtrusion = false;
    double mExtrusionHeight = 0;

};

#endif // QGSVECTORLAYERELEVATIONPROPERTIES_H
