/***************************************************************************
    qgsannotationlayer.h
    ----------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYER_H
#define QGSANNOTATIONLAYER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerrenderer.h"

class QgsAnnotationItem;


/**
 * \ingroup core
 *
 * \brief Represents a map layer containing a set of georeferenced annotations, e.g. markers, lines, polygons or
 * text items.
 *
 * Annotation layers store a set of QgsAnnotationItem items, which are rendered according to the item's
 * z-order.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationLayer : public QgsMapLayer
{
    Q_OBJECT

  public:

    /**
     * Setting options for loading annotation layers.
     * \since QGIS 3.16
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions.
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext )
        : transformContext( transformContext )
      {}

      /**
       * Coordinate transform context
       */
      QgsCoordinateTransformContext transformContext;

    };


    /**
     * Constructor for a new QgsAnnotationLayer with the specified layer \a name.
     *
     * The \a options argument specifies load-time layer options.
     */
    QgsAnnotationLayer( const QString &name, const QgsAnnotationLayer::LayerOptions &options );
    ~QgsAnnotationLayer() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsAnnotationLayer: '%1'>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Resets the annotation layer to a default state, and clears all items from it.
     */
    void reset();

    /**
     * Adds an \a item to the layer.
     *
     * Ownership of \a item is transferred to the layer.
     *
     * Returns the unique ID assigned to the item.
     */
    QString addItem( QgsAnnotationItem *item SIP_TRANSFER );

    /**
     * Removes (and deletes) the item with matching \a id.
     */
    bool removeItem( const QString &id );

    /**
     * Removes all items from the layer.
     */
    void clear();

    /**
     * Returns TRUE if the annotation layer is empty and contains no annotations.
     */
    bool isEmpty() const;

    /**
     * Returns a map of items contained in the layer, by unique item ID.
     *
     * This map contains references to items owned by the layer, and ownership of these remains
     * with the layer.
     */
    QMap<QString, QgsAnnotationItem *> items() const { return mItems; }

    /**
     * Sets the \a opacity for the annotation layer, where \a opacity is a value between 0 (totally transparent)
     * and 1.0 (fully opaque).
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the opacity for the annotation layer, where opacity is a value between 0 (totally transparent)
     * and 1.0 (fully opaque).
     * \see setOpacity()
     */
    double opacity() const { return mOpacity; }

    QgsAnnotationLayer *clone() const override SIP_FACTORY;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;
    QgsRectangle extent() const override;
    void setTransformContext( const QgsCoordinateTransformContext &context ) override;
    bool readXml( const QDomNode &layerNode, QgsReadWriteContext &context ) override;
    bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &, StyleCategories categories = AllStyleCategories ) const override;
    bool readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;

  private:
    QMap<QString, QgsAnnotationItem *> mItems;
    double mOpacity = 1;
    QgsCoordinateTransformContext mTransformContext;
};

#endif // QGSANNOTATIONLAYER_H
