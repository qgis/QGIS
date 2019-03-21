/***************************************************************************
  qgslegendrenderer.h
  --------------------------------------
  Date                 : July 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLEGENDRENDERER_H
#define QGSLEGENDRENDERER_H

#include "qgis_core.h"
#include <QPointF>

class QRectF;
class QStandardItem;
class QJsonObject;

class QgsLayerTreeGroup;
class QgsLayerTreeLayer;
class QgsLayerTreeModel;
class QgsLayerTreeModelLegendNode;
class QgsLayerTreeNode;
class QgsSymbol;
class QgsRenderContext;

#include "qgslegendsettings.h"

/**
 * \ingroup core
 * \brief The QgsLegendRenderer class handles automatic layout and rendering of legend.
 * The content is given by QgsLayerTreeModel instance. Various layout properties can be configured
 * within QgsLegendRenderer.
 *
 * All spacing and sizes are in millimeters.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsLegendRenderer
{
  public:

    /**
     * Constructor for QgsLegendRenderer. The ownership of the legend model is not changed,
     * and the model must exist for the lifetime of this renderer.
     */
    QgsLegendRenderer( QgsLayerTreeModel *legendModel, const QgsLegendSettings &settings );

    /**
     * Runs the layout algorithm and returns the minimum size required for the legend.
     * \see setLegendSize()
     * \see legendSize()
     */
    QSizeF minimumSize( QgsRenderContext *renderContext = nullptr );

    /**
     * Sets the preferred resulting legend size.
     *
     * If the size is null, the legend will be drawn with the minimum possible size to fit its content.
     *
     * \see legendSize()
     * \see minimumSize()
     */
    void setLegendSize( QSizeF s ) { mLegendSize = s; }

    /**
     * Returns the preferred legend size set by the client.
     *
     * If the returned size is null, the legend will be drawn with the minimum possible size to fit its content.
     *
     * \see minimumSize()
     * \see setLegendSize()
     */
    QSizeF legendSize() const { return mLegendSize; }

    /**
     * Draws the legend with given \a painter. The legend will occupy the area reported in legendSize().
     * The \a painter should be scaled beforehand so that units correspond to millimeters.
     */
    void drawLegend( QPainter *painter );

    /**
     * Draws the legend using a given render \a context. The legend will occupy the area reported in legendSize().
     *
     * \since QGIS 3.6
     */
    void drawLegend( QgsRenderContext &context );

    /**
     * Renders the legend in a \a json object.
     *
     * \since QGIS 3.8
     */
    void exportLegendToJson( const QgsRenderContext &context, QJsonObject &json );

    /**
     * Sets the \a style of a \a node.
     *
     * \see nodeLegendStyle()
     */
    static void setNodeLegendStyle( QgsLayerTreeNode *node, QgsLegendStyle::Style style );

    /**
     * Returns the style for the given \a node, within the specified \a model.
     *
     * \see setNodeLegendStyle()
     */
    static QgsLegendStyle::Style nodeLegendStyle( QgsLayerTreeNode *node, QgsLayerTreeModel *model );

  private:

#ifndef SIP_RUN

    /**
     * A legend Nucleon is either a group title, a layer title or a layer child item.
     *
     * E.g. a layer title nucleon is just the layer's title, it does not
     * include all of that layer's subitems. Similarly, a group's title nucleon is just
     * the group title, and does not include the actual content of that group.
     */
    class Nucleon
    {
      public:
        //! Constructor for Nuclean
        Nucleon() = default;

        QObject *item = nullptr;

        //! Symbol size, not including any preset padding space around the symbol
        QSizeF symbolSize;

        //! Label size, not including any preset padding space around the label
        QSizeF labelSize;

        //! Nucleon size
        QSizeF size;

        /**
         * Horizontal offset for the symbol label.
         *
         * This offset is the same for all symbol labels belonging to the same layer,
         * within the same legend column.
         */
        double labelXOffset = 0.0;
    };

    /**
     * An Atom is indivisible set of legend Nucleons (i.e. it is indivisible into more columns).
     *
     * An Atom may consist of one or more Nucleon(s), depending on the layer splitting mode:
     *
     *  1) no layer split: [group_title ...] layer_title layer_item [layer_item ...]
     *  2) layer split:    [group_title ...] layer_title layer_item
     *              or:    layer_item
     *
     * This means that group titles must not be split from layer titles and layer titles
     * must not be split from the first layer item, because this results in a poor layout
     * and it would not be logical to leave a group or layer title at the bottom of a column,
     * separated from the actual content of that group or layer.
     */
    class Atom
    {
      public:

        //! List of child Nucleons belonging to this Atom.
        QList<Nucleon> nucleons;

        //! Atom size, including internal spacing between Nucleons, but excluding any padding space around the Atom itself.
        QSizeF size = QSizeF( 0, 0 );

        //! Corresponding column index
        int column = 0;
    };

    /**
     * Draws the legend and returns the actual size of the legend.
     *
     * If \a painter is NULLPTR, only the size of the legend will be calculated and no
     * painting will be attempted.
     */
    QSizeF paintAndDetermineSize( QPainter *painter = nullptr );

    /**
     * Returns a list of Atoms for the specified \a parentGroup, respecting the current layer's
     * splitting settings.
     */
    QList<Atom> createAtomList( QgsLayerTreeGroup *parentGroup, bool splitLayer );

    /**
     * Divides a list of Atoms into columns, and sets the column index for each atom in the list.
     */
    void setColumns( QList<Atom> &atomList );

    /**
     * Draws a title in the legend using the title font and the specified alignment settings.
     *
     * Returns the size required to draw the complete title.
     *
     * If \a painter is NULLPTR, no painting will be attempted, but the required size will still be calculated and returned.
     */
    QSizeF drawTitle( QPainter *painter = nullptr, QPointF point = QPointF(), Qt::AlignmentFlag halignment = Qt::AlignLeft, double legendWidth = 0 );

    /**
     * Returns the calculated padding space required above the given \a atom.
     */
    double spaceAboveAtom( const Atom &atom );

    /**
     * Draws an \a atom and return its actual size.
     *
     * The \a atom is drawn with the space above it, so that the first atoms in column are all
     * aligned to the same line regardless of their style top spacing.
    */
    QSizeF drawAtom( const Atom &atom, QPainter *painter = nullptr, QPointF point = QPointF() );

    /**
     * Draws the symbol of a given symbol QgsLayerTreeModelLegendNode.
     */
    Nucleon drawSymbolItem( QgsLayerTreeModelLegendNode *symbolItem, QPainter *painter = nullptr, QPointF point = QPointF(), double labelXOffset = 0 );

    /**
     * Draws the title of a layer, given a QgsLayerTreeLayer, and a destination \a painter.
     *
     * Returns the size of the title.
     *
     * The \a painter may be NULLPTR, in which case on the size is calculated and no painting is attempted.
     */
    QSizeF drawLayerTitle( QgsLayerTreeLayer *nodeLayer, QPainter *painter = nullptr, QPointF point = QPointF() );

    /**
     * Draws a group item.
     * Returns the size of the title.
     */
    QSizeF drawGroupTitle( QgsLayerTreeGroup *nodeGroup, QPainter *painter = nullptr, QPointF point = QPointF() );

    /**
     * Renders a group item in a \a json object.
     *
     * \since QGIS 3.8
     */
    void exportLegendToJson( const QgsRenderContext &context, QgsLayerTreeGroup *nodeGroup, QJsonObject &json );

    /**
     * Draws the legend using the specified render \a context, and returns the actual size of the legend.
     *
     * If \a context is NULLPTR, only the size of the legend will be calculated and no
     * painting will be attempted.
     */
    QSizeF paintAndDetermineSize( QgsRenderContext *context );

    /**
     * Draws a title in the legend using the specified render \a context, with the title font and the specified alignment settings.
     *
     * Returns the size required to draw the complete title.
     *
     * If \a context is NULLPTR, no painting will be attempted, but the required size will still be calculated and returned.
     */
    QSizeF drawTitle( QgsRenderContext *context, QPointF point = QPointF(), Qt::AlignmentFlag halignment = Qt::AlignLeft, double legendWidth = 0 );

    /**
     * Draws an \a atom and return its actual size, using the specified render \a context.
     *
     * The \a atom is drawn with the space above it, so that the first atoms in column are all
     * aligned to the same line regardless of their style top spacing.
     *
     * If \a context is NULLPTR, no painting will be attempted, but the required size will still be calculated and returned.
    */
    QSizeF drawAtom( const Atom &atom, QgsRenderContext *rendercontext, QPointF point = QPointF() );

    /**
     * Draws the symbol of a given symbol QgsLayerTreeModelLegendNode, using the specified render \a context.
     */
    Nucleon drawSymbolItem( QgsLayerTreeModelLegendNode *symbolItem, QgsRenderContext *context, QPointF point = QPointF(), double labelXOffset = 0 );

    /**
     * Draws the title of a layer, given a QgsLayerTreeLayer, and a destination render \a context.
     *
     * Returns the size of the title.
     *
     * The \a context may be NULLPTR, in which case on the size is calculated and no painting is attempted.
     */
    QSizeF drawLayerTitle( QgsLayerTreeLayer *nodeLayer, QgsRenderContext *context, QPointF point = QPointF() );

    /**
     * Draws a group's title, using the specified render \a context.
     *
     * Returns the size of the title.
     */
    QSizeF drawGroupTitle( QgsLayerTreeGroup *nodeGroup, QgsRenderContext *context, QPointF point = QPointF() );

    /**
     * Returns the style of the given \a node.
     */
    QgsLegendStyle::Style nodeLegendStyle( QgsLayerTreeNode *node );

  private:
    QgsLayerTreeModel *mLegendModel = nullptr;

    QgsLegendSettings mSettings;

    QSizeF mLegendSize;

#endif
    QSizeF drawTitleInternal( QgsRenderContext *context, QPainter *painter, QPointF point, Qt::AlignmentFlag halignment, double legendWidth );
    QSizeF drawAtomInternal( const Atom &atom, QgsRenderContext *context, QPainter *painter, QPointF point );
    QgsLegendRenderer::Nucleon drawSymbolItemInternal( QgsLayerTreeModelLegendNode *symbolItem, QgsRenderContext *context, QPainter *painter, QPointF point, double labelXOffset );
    QSizeF drawLayerTitleInternal( QgsLayerTreeLayer *nodeLayer, QgsRenderContext *context, QPainter *painter, QPointF point );
    QSizeF drawGroupTitleInternal( QgsLayerTreeGroup *nodeGroup, QgsRenderContext *context, QPainter *painter, QPointF point );
    QSizeF paintAndDetermineSizeInternal( QgsRenderContext *context, QPainter *painter );
};

#endif // QGSLEGENDRENDERER_H
