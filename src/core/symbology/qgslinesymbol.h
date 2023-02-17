/***************************************************************************
 qgslinesymbol.h
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

#ifndef QGSLINESYMBOL_H
#define QGSLINESYMBOL_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbol.h"

/**
 * \ingroup core
 * \class QgsLineSymbol
 *
 * \brief A line symbol type, for rendering LineString and MultiLineString geometries.
 */
class CORE_EXPORT QgsLineSymbol : public QgsSymbol
{
  public:

    /**
     * Create a line symbol with one symbol layer: SimpleLine with specified properties.
     * This is a convenience method for easier creation of line symbols.
     */
    static QgsLineSymbol *createSimple( const QVariantMap &properties ) SIP_FACTORY;

    /**
     * Constructor for QgsLineSymbol, with the specified list of initial symbol \a layers.
     *
     * Ownership of the \a layers are transferred to the symbol.
     */
    QgsLineSymbol( const QgsSymbolLayerList &layers SIP_TRANSFER = QgsSymbolLayerList() );

    /**
     * Sets the \a width for the whole line symbol. Individual symbol layer sizes
     * will be scaled to maintain their current relative size to the whole symbol size.
     *
     * \see width()
     */
    void setWidth( double width ) const;

    /**
     * Sets the width units for the whole symbol (including all symbol layers).
     * \param unit size units
     * \since QGIS 3.16
     */
    void setWidthUnit( QgsUnitTypes::RenderUnit unit ) const;


    /**
     * Returns the estimated width for the whole symbol, which is the maximum width of
     * all marker symbol layers in the symbol.
     *
     * \warning This returned value is inaccurate if the symbol consists of multiple
     * symbol layers with different width units. Use the overload accepting a QgsRenderContext
     * argument instead for accurate sizes in this case.
     *
     * \see setWidth()
     */
    double width() const;

    /**
     * Returns the symbol width, in painter units. This is the maximum width of
     * all marker symbol layers in the symbol.
     *
     * This method returns an accurate width by calculating the actual rendered
     * width of each symbol layer using the provided render \a context.
     *
     * \see setWidth()
     *
     * \since QGIS 3.4.5
     */
    double width( const QgsRenderContext &context ) const;

    /**
     * Set data defined width for whole symbol (including all symbol layers).
     * \see dataDefinedWidth()
     * \since QGIS 3.0
     */
    void setDataDefinedWidth( const QgsProperty &property ) const;

    /**
     * Returns data defined width for whole symbol (including all symbol layers).
     * \returns data defined width, or invalid property if size is not set
     * at the line level. Caller takes responsibility for deleting the returned object.
     * \see setDataDefinedWidth
     * \since QGIS 3.0
     */
    QgsProperty dataDefinedWidth() const;

    /**
     * Renders the symbol along the line joining \a points, using the given render \a context.
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
    void renderPolyline( const QPolygonF &points, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    QgsLineSymbol *clone() const override SIP_FACTORY;

  private:

    void renderPolylineUsingLayer( QgsLineSymbolLayer *layer, const QPolygonF &points, QgsSymbolRenderContext &context );

};


#endif // QGSLINESYMBOL_H

