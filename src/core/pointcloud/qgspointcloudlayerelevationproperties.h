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

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsmarkersymbol.h"
#include "qgsunittypes.h"

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
    bool isVisibleInZRange( const QgsDoubleRange &range, QgsMapLayer *layer = nullptr ) const override;
    QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const override;
    QList< double > significantZValues( QgsMapLayer *layer ) const override;
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
    Qgis::RenderUnit maximumScreenErrorUnit() const { return mMaximumScreenErrorUnit; }

    /**
     * Sets the \a unit for the maximum screen error allowed when generating elevation profiles for the point cloud.
     *
     * \see setMaximumScreenError()
     * \see maximumScreenErrorUnit()
     *
     * \since QGIS 3.26
     */
    void setMaximumScreenErrorUnit( Qgis::RenderUnit unit );

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
    void setPointSizeUnit( const Qgis::RenderUnit units );

    /**
     * Returns the units used for the point size used for drawing points in elevation profile charts.
     * \see setPointSizeUnit()
     * \see pointSize()
     *
     * \since QGIS 3.26
     */
    Qgis::RenderUnit pointSizeUnit() const { return mPointSizeUnit; }

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

    /**
     * Sets the profile \a type used when generating elevation profile plots.
     *
     * \see renderType()
     * \since QGIS 4.0
     */
    void setType( Qgis::PointCloudProfileType type ) { mType = type; }

    /**
     * Returns the profile type used when generating elevation profile plots.
     *
     * \see setRenderType()
     * \since QGIS 4.0
     */
    Qgis::PointCloudProfileType renderType() const { return mType; }

    /**
     * Returns the symbol used to render lines for the layer in elevation profile plots.
     *
     * \note This setting is only used when type() is Qgis::PointCloudProfileType::TriangulatedSurface.
     *
     * \see setProfileLineSymbol()
     * \since QGIS 4.0
     */
    QgsLineSymbol *profileLineSymbol() const;

    /**
     * Sets the line \a symbol used to render lines for the layer in elevation profile plots.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \note This setting is only used when type() is Qgis::PointCloudProfileType::TriangulatedSurface.
     *
     * \see profileLineSymbol()
     * \since QGIS 4.0
     */
    void setProfileLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the symbol used to render polygons for the layer in elevation profile plots.
     *
     * \note This setting is only used when type() is Qgis::PointCloudProfileType::TriangulatedSurface.
     *
     * \see setProfileFillSymbol()
     * \since QGIS 4.0
     */
    QgsFillSymbol *profileFillSymbol() const;

    /**
     * Sets the fill \a symbol used to render polygons for the layer in elevation profile plots.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \note This setting is only used when type() is Qgis::PointCloudProfileType::TriangulatedSurface.
     *
     * \see profileFillSymbol()
     * \since QGIS 4.0
     */
    void setProfileFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the symbology option used to render the point cloud profile in elevation profile plots.
     *
     * \note This setting is only used when type() is Qgis::PointCloudProfileType::TriangulatedSurface.
     *
     * \see setProfileSymbology()
     * \since QGIS 4.0
     */
    Qgis::ProfileSurfaceSymbology profileSymbology() const { return mSymbology; }

    /**
     * Sets the \a symbology option used to render the point cloud profile in elevation profile plots.
     *
     * \note This setting is only used when type() is Qgis::PointCloudProfileType::TriangulatedSurface.
     *
     * \see setProfileSymbology()
     * \since QGIS 4.0
     */
    void setProfileSymbology( Qgis::ProfileSurfaceSymbology symbology );

    /**
     * Returns the elevation limit, which is used when profileSymbology() is
     * Qgis::ProfileSurfaceSymbology::FillBelow or Qgis::ProfileSurfaceSymbology::FillAbove
     * to limit the fill to a specific elevation range.
     *
     * By default this is NaN, which indicates that there is no elevation limit.
     *
     * \note This setting is only used when type() is Qgis::PointCloudProfileType::TriangulatedSurface.
     *
     * \see setElevationLimit()
     * \since QGIS 4.0
     */
    double elevationLimit() const;

    /**
     * Sets the elevation \a limit, which is used when profileSymbology() is
     * Qgis::ProfileSurfaceSymbology::FillBelow or Qgis::ProfileSurfaceSymbology::FillAbove
     * to limit the fill to a specific elevation range.
     *
     * Set to NaN to indicate that there is no elevation limit.
     *
     * \note This setting is only used when type() is Qgis::PointCloudProfileType::TriangulatedSurface.
     *
     * \see elevationLimit()
     * \since QGIS 4.0
     */
    void setElevationLimit( double limit );

  private:
    void setDefaultProfileLineSymbol( const QColor &color );
    void setDefaultProfileMarkerSymbol( const QColor &color );
    void setDefaultProfileFillSymbol( const QColor &color );

    double mMaximumScreenError = 0.3;
    Qgis::RenderUnit mMaximumScreenErrorUnit = Qgis::RenderUnit::Millimeters;

    double mPointSize = 0.6;
    Qgis::RenderUnit mPointSizeUnit = Qgis::RenderUnit::Millimeters;
    Qgis::PointCloudSymbol mPointSymbol = Qgis::PointCloudSymbol::Square;
    std::unique_ptr< QgsLineSymbol > mProfileLineSymbol;
    std::unique_ptr< QgsFillSymbol > mProfileFillSymbol;
    Qgis::ProfileSurfaceSymbology mSymbology = Qgis::ProfileSurfaceSymbology::FillBelow;
    double mElevationLimit = std::numeric_limits<double>::quiet_NaN();
    QColor mPointColor;
    bool mRespectLayerColors = true;
    bool mApplyOpacityByDistanceEffect = false;
    Qgis::PointCloudProfileType mType = Qgis::PointCloudProfileType::IndividualPoints;
};

#endif // QGSPOINTCLOUDLAYERELEVATIONPROPERTIES_H
