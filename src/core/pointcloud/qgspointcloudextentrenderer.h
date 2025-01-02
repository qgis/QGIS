/***************************************************************************
                         qgspointcloudextentrenderer.h
                         --------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDEXTENTRENDERER_H
#define QGSPOINTCLOUDEXTENTRENDERER_H

#include "qgspointcloudrenderer.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsFillSymbol;

/**
 * \ingroup core
 * \brief A renderer for 2d visualisation of point clouds which shows the dataset's extents using a fill symbol.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudExtentRenderer : public QgsPointCloudRenderer
{
  public:

    /**
     * Constructor for QgsPointCloudExtentRenderer.
     *
     * Optionally the \a symbol to use for showing the extent can be specified. If specified, ownership is
     * transferred to the renderer. If no \a symbol is specified a default one will be created instead.
     */
    QgsPointCloudExtentRenderer( QgsFillSymbol *symbol SIP_TRANSFER = nullptr );
    ~QgsPointCloudExtentRenderer() override;

    QString type() const override;
    QgsPointCloudRenderer *clone() const override;
    void renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context ) override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;

    void startRender( QgsPointCloudRenderContext &context ) override;
    void stopRender( QgsPointCloudRenderContext &context ) override;
    QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) override SIP_FACTORY;

    /**
     * Creates an extent renderer from an XML \a element.
     */
    static QgsPointCloudRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Renders a polygon \a extent geometry to the specified render \a context.
     */
    void renderExtent( const QgsGeometry &extent, QgsPointCloudRenderContext &context );

    /**
     * Returns a new instance of the default fill symbol to use for showing point cloud extents.
     */
    static QgsFillSymbol *defaultFillSymbol() SIP_FACTORY;

    /**
     * Returns the symbol used to render the cloud's extent.
     *
     * \see setFillSymbol()
     */
    QgsFillSymbol *fillSymbol() const;

    /**
     * Sets the \a symbol used to render the cloud's extent.
     *
     * Ownership of \a symbol is transferred to the renderer.
     *
     * \see fillSymbol()
     */
    void setFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Renders a label inside the specified extent rectangle.
     *
     * The label will be rendered centered horizontally and vertically inside \a extent. If the label is too large to fit
     * inside this rectangle, it will not be rendered.
     *
     * \param extent rectangle (in painter coordinates) inside which the label will be rendered
     * \param text label text to render
     * \param context point cloud rendering context
     * \since QGIS 3.42
     */
    void renderLabel( const QRectF &extent, const QString &text, QgsPointCloudRenderContext &context ) const;

  private:

    std::unique_ptr< QgsFillSymbol > mFillSymbol;

};

#endif // QGSPOINTCLOUDEXTENTRENDERER_H
