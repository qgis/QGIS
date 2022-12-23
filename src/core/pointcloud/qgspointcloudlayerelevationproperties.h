/***************************************************************************
                         qgspointcloudlayerelevationproperties.h
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
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


#ifndef QGSPOINTCLOUDLAYERELEVATIONPROPERTIES_H
#define QGSPOINTCLOUDLAYERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsunittypes.h"
#include "qgsmaplayerelevationproperties.h"

/**
 * \class QgsPointCloudLayerElevationProperties
 * \ingroup core
 * \brief Point cloud layer specific subclass of QgsMapLayerElevationProperties.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudLayerElevationProperties : public QgsMapLayerElevationProperties
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPointCloudLayerElevationProperties, with the specified \a parent object.
     */
    QgsPointCloudLayerElevationProperties( QObject *parent SIP_TRANSFERTHIS );

    bool hasElevation() const override;
    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsPointCloudLayerElevationProperties *clone() const override SIP_FACTORY;
    QString htmlSummary() const override;
    bool isVisibleInZRange( const QgsDoubleRange &range ) const override;
    QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const override;
    bool showByDefaultInElevationProfilePlots() const override;

    /**
     * Returns the maximum screen error allowed when generating elevation profiles for the point cloud.
     *
     * Larger values result in a faster generation with less points included.
     *
     * Units are retrieved via maximumScreenErrorUnit().
     *
     * \see setMaximumScreenError()
     * \see maximumScreenErrorUnit()
     *
     * \since QGIS 3.26
     */
    double maximumScreenError() const { return mMaximumScreenError; }

    /**
     * Sets the maximum screen \a error allowed when generating elevation profiles for the point cloud.
     *
     * Larger values result in a faster generation with less points included.
     *
     * Units are set via setMaximumScreenErrorUnit().
     *
     * \see maximumScreenError()
     * \see setMaximumScreenErrorUnit()
     *
     * \since QGIS 3.26
     */
    void setMaximumScreenError( double error );

    /**
     * Returns the unit for the maximum screen error allowed when generating elevation profiles for the point cloud.
     *
     * \see maximumScreenError()
     * \see setMaximumScreenErrorUnit()
     *
     * \since QGIS 3.26
     */
    QgsUnitTypes::RenderUnit maximumScreenErrorUnit() const { return mMaximumScreenErrorUnit; }

    /**
     * Sets the \a unit for the maximum screen error allowed when generating elevation profiles for the point cloud.
     *
     * \see setMaximumScreenError()
     * \see maximumScreenErrorUnit()
     *
     * \since QGIS 3.26
     */
    void setMaximumScreenErrorUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns the symbol used drawing points in elevation profile charts.
     *
     * \see setPointSymbol()
     * \since QGIS 3.26
     */
    Qgis::PointCloudSymbol pointSymbol() const;

    /**
     * Sets the \a symbol used drawing points in elevation profile charts.
     *
     * \see pointSymbol()
     * \since QGIS 3.26
     */
    void setPointSymbol( Qgis::PointCloudSymbol symbol );

    /**
     * Returns the color used drawing points in elevation profile charts.
     *
     * \see setPointColor()
     * \since QGIS 3.26
     */
    QColor pointColor() const { return mPointColor; }

    /**
     * Sets the \a color used drawing points in elevation profile charts.
     *
     * \see pointColor()
     * \since QGIS 3.26
     */
    void setPointColor( const QColor &color );

    /**
     * Returns TRUE if a reduced opacity by distance from profile curve effect should
     * be applied when drawing points in elevation profile charts.
     *
     * \see setApplyOpacityByDistanceEffect()
     * \since QGIS 3.26
     */
    bool applyOpacityByDistanceEffect() const { return mApplyOpacityByDistanceEffect; }

    /**
     * Sets whether a reduced opacity by distance from profile curve effect should
     * be applied when drawing points in elevation profile charts.
     *
     * \see applyOpacityByDistanceEffect()
     * \since QGIS 3.26
     */
    void setApplyOpacityByDistanceEffect( bool apply );

    /**
     * Sets the point \a size used for drawing points in elevation profile charts.
     *
     * Point size units are specified via setPointSizeUnit().
     * \see pointSize()
     * \see setPointSizeUnit()
     *
     * \since QGIS 3.26
     */
    void setPointSize( double size );

    /**
     * Returns the point size used for drawing points in elevation profile charts.
     *
     * The point size units are retrieved by calling pointSizeUnit().
     *
     * \see setPointSize()
     * \see pointSizeUnit()
     *
     * \since QGIS 3.26
     */
    double pointSize() const { return mPointSize; }

    /**
     * Sets the \a units used for the point size used for drawing points in elevation profile charts.
     *
     * \see setPointSize()
     * \see pointSizeUnit()
     *
     * \since QGIS 3.26
     */
    void setPointSizeUnit( const QgsUnitTypes::RenderUnit units );

    /**
     * Returns the units used for the point size used for drawing points in elevation profile charts.
     * \see setPointSizeUnit()
     * \see pointSize()
     *
     * \since QGIS 3.26
     */
    QgsUnitTypes::RenderUnit pointSizeUnit() const { return mPointSizeUnit; }

    /**
     * Returns TRUE if layer coloring should be respected when rendering elevation profile plots.
     *
     * \see setRespectLayerColors()
     */
    bool respectLayerColors() const { return mRespectLayerColors; }

    /**
     * Sets whether layer coloring should be respected when rendering elevation profile plots.
     *
     * \see respectLayerColors()
     */
    void setRespectLayerColors( bool enabled );

  private:

    double mMaximumScreenError = 0.3;
    QgsUnitTypes::RenderUnit mMaximumScreenErrorUnit = QgsUnitTypes::RenderMillimeters;

    double mPointSize = 0.6;
    QgsUnitTypes::RenderUnit mPointSizeUnit = QgsUnitTypes::RenderMillimeters;
    Qgis::PointCloudSymbol mPointSymbol = Qgis::PointCloudSymbol::Square;
    QColor mPointColor;
    bool mRespectLayerColors = true;
    bool mApplyOpacityByDistanceEffect = false;
};

#endif // QGSPOINTCLOUDLAYERELEVATIONPROPERTIES_H
