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

class QgsLineSymbol;
class QgsFillSymbol;
class QgsMarkerSymbol;

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
    ~QgsVectorLayerElevationProperties() override;

    bool hasElevation() const override;
    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void setDefaultsFromLayer( QgsMapLayer *layer ) override;
    QgsVectorLayerElevationProperties *clone() const override SIP_FACTORY;
    QString htmlSummary() const override;
    bool isVisibleInZRange( const QgsDoubleRange &range ) const override;
    QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const override;
    bool showByDefaultInElevationProfilePlots() const override;

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
    void setClamping( Qgis::AltitudeClamping clamping );

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
    void setBinding( Qgis::AltitudeBinding binding );

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
    void setExtrusionEnabled( bool enabled );

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
    void setExtrusionHeight( double height );

    /**
     * Returns TRUE if layer symbology should be respected when rendering elevation profile plots.
     *
     * Specifically, this will result in the layer's symbols (or symbol colors) being used to draw features in the
     * profile plots.
     *
     * \see setRespectLayerSymbology()
     */
    bool respectLayerSymbology() const { return mRespectLayerSymbology; }

    /**
     * Sets whether layer symbology should be respected when rendering elevation profile plots.
     *
     * Specifically, this will result in the layer's symbols (or symbol colors) being used to draw features in the
     * profile plots.
     *
     * \see respectLayerSymbology()
     */
    void setRespectLayerSymbology( bool enabled );

    /**
     * Returns the symbol used to render lines for the layer in elevation profile plots.
     *
     * The symbol will be used in the following circumstances:
     *
     * - A point feature is shown on the profile chart when extrusionEnabled() is TRUE
     * - A line feature is intersected by a profile line and extrusionEnabled() is TRUE
     * - A polygon feature is intersected by a profile line and extrusionEnabled() is FALSE
     *
     * \see setProfileLineSymbol()
     */
    QgsLineSymbol *profileLineSymbol() const;

    /**
     * Sets the line \a symbol used to render lines for the layer in elevation profile plots.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * The symbol will be used in the following circumstances:
     *
     * - A point feature is shown on the profile chart when extrusionEnabled() is TRUE
     * - A line feature is intersected by a profile line and extrusionEnabled() is TRUE
     * - A polygon feature is intersected by a profile line and extrusionEnabled() is FALSE
     *
     * \see profileLineSymbol()
     */
    void setProfileLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the symbol used to render polygons for the layer in elevation profile plots.
     *
     * The symbol will be used in the following circumstances:
     *
     * - A polygon feature is intersected by a profile line and extrusionEnabled() is TRUE
     *
     * \see setProfileFillSymbol()
     */
    QgsFillSymbol *profileFillSymbol() const;

    /**
     * Sets the fill \a symbol used to render polygons for the layer in elevation profile plots.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * The symbol will be used in the following circumstances:
     *
     * - A polygon feature is intersected by a profile line and extrusionEnabled() is TRUE
     *
     * \see profileFillSymbol()
     */
    void setProfileFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the symbol used to render points for the layer in elevation profile plots.
     *
     * The symbol will be used in the following circumstances:
     *
     * - A point feature is shown on the profile chart when extrusionEnabled() is FALSE
     * - A line feature is intersected by a profile line and extrusionEnabled() is FALSE
     *
     * \see setProfileMarkerSymbol()
     */
    QgsMarkerSymbol *profileMarkerSymbol() const;

    /**
     * Sets the marker \a symbol used to render points for the layer in elevation profile plots.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * The symbol will be used in the following circumstances:
     *
     * - A point feature is shown on the profile chart when extrusionEnabled() is FALSE
     * - A line feature is intersected by a profile line and extrusionEnabled() is FALSE
     *
     * \see profileMarkerSymbol()
     */
    void setProfileMarkerSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );

  private:

    void setDefaultProfileLineSymbol( const QColor &color );
    void setDefaultProfileMarkerSymbol( const QColor &color );
    void setDefaultProfileFillSymbol( const QColor &color );

    Qgis::AltitudeClamping mClamping = Qgis::AltitudeClamping::Terrain;
    Qgis::AltitudeBinding mBinding = Qgis::AltitudeBinding::Centroid;

    bool mEnableExtrusion = false;
    double mExtrusionHeight = 0;

    std::unique_ptr< QgsLineSymbol > mProfileLineSymbol;
    std::unique_ptr< QgsFillSymbol > mProfileFillSymbol;
    std::unique_ptr< QgsMarkerSymbol > mProfileMarkerSymbol;
    bool mRespectLayerSymbology = true;

};

#endif // QGSVECTORLAYERELEVATIONPROPERTIES_H
