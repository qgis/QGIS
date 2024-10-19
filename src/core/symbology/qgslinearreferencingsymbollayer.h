/***************************************************************************
    qgslinearreferencingsymbollayer.h
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLINEARREFERENCINGSYMBOLLAYER_H
#define QGSLINEARREFERENCINGSYMBOLLAYER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbollayer.h"
#include "qgstextformat.h"

class QgsNumericFormat;

/**
 * \ingroup core
 * \brief Line symbol layer used for decorating accordingly to linear referencing.
 *
 * This symbol layer type allows placing text labels at regular intervals along
 * a line (or at positions corresponding to existing vertices). Positions
 * can be calculated using Cartesian distances, or interpolated from z or m values.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsLinearReferencingSymbolLayer : public QgsLineSymbolLayer
{
  public:
    QgsLinearReferencingSymbolLayer();
    ~QgsLinearReferencingSymbolLayer() override;

    /**
     * Creates a new QgsLinearReferencingSymbolLayer, using the specified \a properties.
     *
     * The caller takes ownership of the returned object.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    QgsLinearReferencingSymbolLayer *clone() const override SIP_FACTORY;
    QVariantMap properties() const override;
    QString layerType() const override;
    Qgis::SymbolLayerFlags flags() const override;
    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;

    /**
     * Returns the text format used to render the layer.
     *
     * \see setTextFormat()
     */
    QgsTextFormat textFormat() const;

    /**
     * Sets the text \a format used to render the layer.
     *
     * \see textFormat()
     */
    void setTextFormat( const QgsTextFormat &format );

    /**
     * Returns the numeric format used to format the labels for the layer.
     *
     * \see setNumericFormat()
     */
    QgsNumericFormat *numericFormat() const;

    /**
     * Sets the numeric \a format used to format the labels for the layer.
     *
     * Ownership of \a format is transferred to the layer.
     *
     * \see numericFormat()
     */
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Returns the interval between labels.
     *
     * Units are always in the original layer CRS units.
     *
     * \see setInterval()
     */
    double interval() const;

    /**
     * Sets the \a interval between labels.
     *
     * Units are always in the original layer CRS units.
     *
     * \see setInterval()
     */
    void setInterval( double interval );

    /**
     * Returns the multiple distance to skip labels for.
     *
     * If this value is non-zero, then any labels which are integer multiples of the returned
     * value will be skipped. This allows creation of advanced referencing styles where a single
     * QgsSymbol has multiple QgsLinearReferencingSymbolLayer symbol layers, eg allowing
     * labeling every 100 in a normal font and every 1000 in a bold, larger font.
     *
     * \see setSkipMultiplesOf()
     */
    double skipMultiplesOf() const;

    /**
     * Sets the \a multiple distance to skip labels for.
     *
     * If this value is non-zero, then any labels which are integer multiples of the returned
     * value will be skipped. This allows creation of advanced referencing styles where a single
     * QgsSymbol has multiple QgsLinearReferencingSymbolLayer symbol layers, eg allowing
     * labeling every 100 in a normal font and every 1000 in a bold, larger font.
     *
     * \see skipMultiplesOf()
     */
    void setSkipMultiplesOf( double multiple );

    /**
     * Returns TRUE if the labels and symbols are to be rotated to match their line segment orientation.
     *
     * \see setRotateLabels()
     */
    bool rotateLabels() const { return mRotateLabels; }

    /**
     * Sets whether the labels and symbols should be rotated to match their line segment orientation.
     *
     * \see rotateLabels()
     */
    void setRotateLabels( bool rotate ) { mRotateLabels = rotate; }

    /**
     * Returns the offset between the line and linear referencing labels.
     *
     * The unit for the offset is retrievable via labelOffsetUnit().
     *
     * \see setLabelOffset()
     * \see labelOffsetUnit()
     */
    QPointF labelOffset() const { return mLabelOffset; }

    /**
     * Sets the \a offset between the line and linear referencing labels.
     *
     * The unit for the offset is set via setLabelOffsetUnit().
     *
     * \see labelOffset()
     * \see setLabelOffsetUnit()
     */
    void setLabelOffset( const QPointF &offset ) { mLabelOffset = offset; }

    /**
     * Returns the unit used for the offset between the line and linear referencing labels.
     *
     * \see setLabelOffsetUnit()
     * \see labelOffset()
     */
    Qgis::RenderUnit labelOffsetUnit() const { return mLabelOffsetUnit; }

    /**
     * Sets the \a unit used for the offset between the line and linear referencing labels.
     *
     * \see labelOffsetUnit()
     * \see setLabelOffset()
     */
    void setLabelOffsetUnit( Qgis::RenderUnit unit ) { mLabelOffsetUnit = unit; }

    /**
     * Returns the map unit scale used for calculating the offset between the line and linear referencing labels.
     *
     * \see setLabelOffsetMapUnitScale()
     */
    const QgsMapUnitScale &labelOffsetMapUnitScale() const { return mLabelOffsetMapUnitScale; }

    /**
     * Sets the map unit \a scale used for calculating the offset between the line and linear referencing labels.
     *
     * \see labelOffsetMapUnitScale()
     */
    void setLabelOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mLabelOffsetMapUnitScale = scale; }

    /**
     * Returns TRUE if a marker symbol should be shown corresponding to the labeled point on line.
     *
     * The marker symbol is set using setSubSymbol()
     *
     * \see setShowMarker()
     */
    bool showMarker() const;

    /**
     * Sets whether a marker symbol should be shown corresponding to the labeled point on line.
     *
     * The marker symbol is set using setSubSymbol()
     *
     * \see showMarker()
     */
    void setShowMarker( bool show );

    /**
     * Returns the placement mode for the labels.
     *
     * \see setPlacement()
     */
    Qgis::LinearReferencingPlacement placement() const;

    /**
     * Sets the \a placement mode for the labels.
     *
     * \see placement()
     */
    void setPlacement( Qgis::LinearReferencingPlacement placement );

    /**
     * Returns the label source, which dictates what quantity to use for the labels shown.
     *
     * \see setLabelSource()
     */
    Qgis::LinearReferencingLabelSource labelSource() const;

    /**
     * Sets the label \a source, which dictates what quantity to use for the labels shown.
     *
     * \see labelSource()
     */
    void setLabelSource( Qgis::LinearReferencingLabelSource source );

    /**
     * Returns the length of line over which the line's direction is averaged when
     * calculating individual label angles. Longer lengths smooth out angles from jagged lines to a greater extent.
     *
     * Units are retrieved through averageAngleUnit()
     *
     * \see setAverageAngleLength()
     * \see averageAngleUnit()
     * \see averageAngleMapUnitScale()
     */
    double averageAngleLength() const { return mAverageAngleLength; }

    /**
     * Sets the \a length of line over which the line's direction is averaged when
     * calculating individual label angles. Longer lengths smooth out angles from jagged lines to a greater extent.
     *
     * Units are set through setAverageAngleUnit()
     *
     * \see averageAngleLength()
     * \see setAverageAngleUnit()
     * \see setAverageAngleMapUnitScale()
     */
    void setAverageAngleLength( double length ) { mAverageAngleLength = length; }

    /**
     * Sets the \a unit for the length over which the line's direction is averaged when
     * calculating individual label angles.
     *
     * \see averageAngleUnit()
     * \see setAverageAngleLength()
     * \see setAverageAngleMapUnitScale()
    */
    void setAverageAngleUnit( Qgis::RenderUnit unit ) { mAverageAngleLengthUnit = unit; }

    /**
     * Returns the unit for the length over which the line's direction is averaged when
     * calculating individual label angles.
     *
     * \see setAverageAngleUnit()
     * \see averageAngleLength()
     * \see averageAngleMapUnitScale()
    */
    Qgis::RenderUnit averageAngleUnit() const { return mAverageAngleLengthUnit; }

    /**
     * Sets the map unit \a scale for the length over which the line's direction is averaged when
     * calculating individual label angles.
     *
     * \see averageAngleMapUnitScale()
     * \see setAverageAngleLength()
     * \see setAverageAngleUnit()
    */
    void setAverageAngleMapUnitScale( const QgsMapUnitScale &scale ) { mAverageAngleLengthMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the length over which the line's direction is averaged when
     * calculating individual label angles.
     *
     * \see setAverageAngleMapUnitScale()
     * \see averageAngleLength()
     * \see averageAngleUnit()
    */
    const QgsMapUnitScale &averageAngleMapUnitScale() const { return mAverageAngleLengthMapUnitScale; }

  private:
    void renderPolylineInterval( const QgsLineString *line, QgsSymbolRenderContext &context, double skipMultiples, const QPointF &labelOffsetPainterUnits, double averageAngleLengthPainterUnits, bool showMarker );
    void renderPolylineVertex( const QgsLineString *line, QgsSymbolRenderContext &context, double skipMultiples, const QPointF &labelOffsetPainterUnits, double averageAngleLengthPainterUnits, bool showMarker );
    void renderGeometryPart( QgsSymbolRenderContext &context, const QgsAbstractGeometry *geometry, double labelOffsetPainterUnitsX, double labelOffsetPainterUnitsY, double skipMultiples, double averageAngleDistancePainterUnits, bool showMarker );
    void renderLineString( QgsSymbolRenderContext &context, const QgsLineString *line, double labelOffsetPainterUnitsX, double labelOffsetPainterUnitsY, double skipMultiples, double averageAngleDistancePainterUnits, bool showMarker );

    static QPointF pointToPainter( QgsSymbolRenderContext &context, double x, double y, double z );

    Qgis::LinearReferencingPlacement mPlacement = Qgis::LinearReferencingPlacement::IntervalCartesian2D;
    Qgis::LinearReferencingLabelSource mLabelSource = Qgis::LinearReferencingLabelSource::CartesianDistance2D;

    double mInterval = 1000;
    double mSkipMultiplesOf = 0;
    bool mRotateLabels = true;

    QPointF mLabelOffset{ 1, 0 };
    Qgis::RenderUnit mLabelOffsetUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mLabelOffsetMapUnitScale;

    QgsTextFormat mTextFormat;
    std::unique_ptr<QgsNumericFormat> mNumericFormat;

    bool mShowMarker = false;
    std::unique_ptr<QgsMarkerSymbol> mMarkerSymbol;

    double mAverageAngleLength = 4;
    Qgis::RenderUnit mAverageAngleLengthUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mAverageAngleLengthMapUnitScale;

    QString mLabelProviderId;

};

#endif // QGSLINEARREFERENCINGSYMBOLLAYER_H
