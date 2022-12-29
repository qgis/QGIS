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
class QgsAbstractAnnotationItemEditOperation;
class QgsPaintEffect;


///@cond PRIVATE
class QgsAnnotationLayerSpatialIndex;
///@endcond

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
     * Replaces the existing item with matching \a id with a new \a item.
     *
     * Ownership of \a item is transferred to the layer.
     *
     * \since QGIS 3.22
     */
    void replaceItem( const QString &id, QgsAnnotationItem *item SIP_TRANSFER );

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
     * Returns the item with the specified \a id, or NULLPTR if no matching item was found.
     *
     * \since QGIS 3.22
     */
    QgsAnnotationItem *item( const QString &id );

    /**
     * Returns a list of the IDs of all annotation items within the specified \a bounds (in layer CRS), when
     * rendered using the given render \a context.
     *
     * The optional \a feedback argument can be used to cancel the search early.
     *
     * \since QGIS 3.22
     */
    QStringList itemsInBounds( const QgsRectangle &bounds, QgsRenderContext &context, QgsFeedback *feedback = nullptr ) const;

    /**
     * Applies an edit \a operation to the layer.
     *
     * Returns TRUE if the operation was successfully applied.
     *
     * \since QGIS 3.22
     */
    Qgis::AnnotationItemEditOperationResult applyEdit( QgsAbstractAnnotationItemEditOperation *operation );

    Qgis::MapLayerProperties properties() const override;
    QgsAnnotationLayer *clone() const override SIP_FACTORY;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;
    QgsRectangle extent() const override;
    void setTransformContext( const QgsCoordinateTransformContext &context ) override;
    bool readXml( const QDomNode &layerNode, QgsReadWriteContext &context ) override;
    bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &, StyleCategories categories = AllStyleCategories ) const override;
    bool readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;
    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories ) const override;
    bool readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories ) override;
    bool isEditable() const override;
    bool supportsEditing() const override;
    QgsDataProvider *dataProvider() override;
    const QgsDataProvider *dataProvider() const override SIP_SKIP;
    QString htmlMetadata() const override;

    /**
     * Returns the current paint effect for the layer.
     * \see setPaintEffect()
     * \since QGIS 3.22
     */
    QgsPaintEffect *paintEffect() const;

    /**
     * Sets the current paint \a effect for the layer.
     *
     * Ownership is transferred to the renderer.
     *
     * \see paintEffect()
     * \since QGIS 3.22
     */
    void setPaintEffect( QgsPaintEffect *effect SIP_TRANSFER );

  private:

    QStringList queryIndex( const QgsRectangle &bounds, QgsFeedback *feedback = nullptr ) const;
    bool writeItems( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) const;
    bool readItems( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories );

    QMap<QString, QgsAnnotationItem *> mItems;
    QgsCoordinateTransformContext mTransformContext;

    std::unique_ptr< QgsAnnotationLayerSpatialIndex > mSpatialIndex;
    QSet< QString > mNonIndexedItems;

    QgsDataProvider *mDataProvider = nullptr;

    std::unique_ptr< QgsPaintEffect > mPaintEffect;

    friend class QgsAnnotationLayerRenderer;

};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * A minimal data provider for annotation layers.
 *
 * \since QGIS 3.22
 */
class QgsAnnotationLayerDataProvider : public QgsDataProvider
{
    Q_OBJECT

  public:
    QgsAnnotationLayerDataProvider( const QgsDataProvider::ProviderOptions &providerOptions,
                                    QgsDataProvider::ReadFlags flags );
    QgsCoordinateReferenceSystem crs() const override;
    QString name() const override;
    QString description() const override;
    QgsRectangle extent() const override;
    bool isValid() const override;

};
///@endcond
#endif

#endif // QGSANNOTATIONLAYER_H
