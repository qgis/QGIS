/***************************************************************************
 qgsmarkersymbol.h
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMARKERSYMBOL_H
#define QGSMARKERSYMBOL_H

#include "qgis_core.h"
#include "qgssymbol.h"

class QgsMarkerSymbolLayer;

/**
 * \ingroup core
 * \class QgsMarkerSymbol
 *
 * \brief A marker symbol type, for rendering Point and MultiPoint geometries.
 */
class CORE_EXPORT QgsMarkerSymbol : public QgsSymbol
{
  public:

    /**
     * Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
     * This is a convenience method for easier creation of marker symbols.
     */
    static QgsMarkerSymbol *createSimple( const QVariantMap &properties ) SIP_FACTORY;

    /**
     * Constructor for QgsMarkerSymbol, with the specified list of initial symbol \a layers.
     *
     * Ownership of the \a layers are transferred to the symbol.
     */
    QgsMarkerSymbol( const QgsSymbolLayerList &layers SIP_TRANSFER = QgsSymbolLayerList() );

    /**
     * Sets the angle for the whole symbol. Individual symbol layer sizes
     * will be rotated to maintain their current relative angle to the whole symbol angle.
     * \param symbolAngle new symbol angle
     * \see angle()
     */
    void setAngle( double symbolAngle ) const;

    /**
     * Returns the marker angle for the whole symbol. Note that for symbols with
     * multiple symbol layers, this will correspond just to the angle of
     * the first symbol layer.
     * \see setAngle()
     * \since QGIS 2.16
     */
    double angle() const;

    /**
     * Set data defined angle for whole symbol (including all symbol layers).
     * \see dataDefinedAngle()
     * \since QGIS 3.0
     */
    void setDataDefinedAngle( const QgsProperty &property );

    /**
     * Returns data defined angle for whole symbol (including all symbol layers).
     * \returns data defined angle, or invalid property if angle is not set
     * at the marker level.
     * \see setDataDefinedAngle()
     * \since QGIS 3.0
     */
    QgsProperty dataDefinedAngle() const;

    /**
     * Sets the line angle modification for the symbol's angle. This angle is added to
     * the marker's rotation and data defined rotation before rendering the symbol, and
     * is usually used for orienting symbols to match a line's angle.
     * \param lineAngle Angle in degrees, valid values are between 0 and 360
     * \since QGIS 2.9
     */
    void setLineAngle( double lineAngle ) const;

    /**
     * Sets the size for the whole symbol. Individual symbol layer sizes
     * will be scaled to maintain their current relative size to the whole symbol size.
     * \param size new symbol size
     * \see size()
     * \see setSizeUnit()
     * \see setSizeMapUnitScale()
     */
    void setSize( double size ) const;

    /**
     * Returns the estimated size for the whole symbol, which is the maximum size of
     * all marker symbol layers in the symbol.
     *
     * \warning This returned value is inaccurate if the symbol consists of multiple
     * symbol layers with different size units. Use the overload accepting a QgsRenderContext
     * argument instead for accurate sizes in this case.
     *
     * \see setSize()
     * \see sizeUnit()
     * \see sizeMapUnitScale()
     */
    double size() const;

    /**
     * Returns the symbol size, in painter units. This is the maximum size of
     * all marker symbol layers in the symbol.
     *
     * This method returns an accurate size by calculating the actual rendered
     * size of each symbol layer using the provided render \a context.
     *
     * \see setSize()
     * \see sizeUnit()
     * \see sizeMapUnitScale()
     *
     * \since QGIS 3.4.5
     */
    double size( const QgsRenderContext &context ) const;

    /**
     * Sets the size units for the whole symbol (including all symbol layers).
     * \param unit size units
     * \see sizeUnit()
     * \see setSizeMapUnitScale()
     * \see setSize()
     * \since QGIS 2.16
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit ) const;

    /**
     * Returns the size units for the whole symbol (including all symbol layers).
     * \returns size units, or mixed units if symbol layers have different units
     * \see setSizeUnit()
     * \see sizeMapUnitScale()
     * \see size()
     * \since QGIS 2.16
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /**
     * Sets the size map unit scale for the whole symbol (including all symbol layers).
     * \param scale map unit scale
     * \see sizeMapUnitScale()
     * \see setSizeUnit()
     * \see setSize()
     * \since QGIS 2.16
     */
    void setSizeMapUnitScale( const QgsMapUnitScale &scale ) const;

    /**
     * Returns the size map unit scale for the whole symbol. Note that for symbols with
     * multiple symbol layers, this will correspond just to the map unit scale
     * for the first symbol layer.
     * \see setSizeMapUnitScale()
     * \see sizeUnit()
     * \see size()
     * \since QGIS 2.16
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /**
     * Set data defined size for whole symbol (including all symbol layers).
     * \see dataDefinedSize()
     * \since QGIS 3.0
     */
    void setDataDefinedSize( const QgsProperty &property ) const;

    /**
     * Returns data defined size for whole symbol (including all symbol layers).
     * \returns data defined size, or invalid property if size is not set
     * at the marker level.
     * \see setDataDefinedSize
     * \since QGIS 3.0
     */
    QgsProperty dataDefinedSize() const;

    /**
     * Sets the method to use for scaling the marker's size.
     * \param scaleMethod scale method
     * \see scaleMethod()
     */
    void setScaleMethod( Qgis::ScaleMethod scaleMethod ) const;

    /**
     * Returns the method to use for scaling the marker's size.
     * \see setScaleMethod()
     */
    Qgis::ScaleMethod scaleMethod() const;

    /**
     * Renders the symbol at the specified \a point, using the given render \a context.
     *
     * The \a f argument is used to pass the feature currently being rendered (when available).
     *
     * If only a single symbol layer from the symbol should be rendered, it should be specified
     * in the \a layer argument. A \a layer of -1 indicates that all symbol layers should be
     * rendered.
     *
     * If \a selected is TRUE then the symbol will be drawn using the "selected feature"
     * style and colors instead of the symbol's normal style.
     */
    void renderPoint( QPointF point, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    /**
     * Returns the approximate bounding box of the marker symbol, which includes the bounding box
     * of all symbol layers for the symbol. It is recommended to use this method only between startRender()
     * and stopRender() calls, or data defined rotation and offset will not be correctly calculated.
     * \param point location of rendered point in painter units
     * \param context render context
     * \param feature feature being rendered at point (optional). If not specified, the bounds calculation will not
     * include data defined parameters such as offset and rotation
     * \returns approximate symbol bounds, in painter units
     * \since QGIS 2.14
    */
    QRectF bounds( QPointF point, QgsRenderContext &context, const QgsFeature &feature = QgsFeature() ) const;

    QgsMarkerSymbol *clone() const override SIP_FACTORY;

  private:

    void renderPointUsingLayer( QgsMarkerSymbolLayer *layer, QPointF point, QgsSymbolRenderContext &context );

};


#endif // QGSMARKERSYMBOL_H

