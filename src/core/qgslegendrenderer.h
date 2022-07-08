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
     *
     * \deprecated Use the variant which accepts a QgsRenderContext instead.
     */
    Q_DECL_DEPRECATED void drawLegend( QPainter *painter ) SIP_DEPRECATED;

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
    QJsonObject exportLegendToJson( const QgsRenderContext &context );

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
     * A legend component is either a group title, a layer title or a layer child item.
     *
     * E.g. a layer title component is just the layer's title, it does not
     * include all of that layer's subitems. Similarly, a group's title component is just
     * the group title, and does not include the actual content of that group.
     */
    class LegendComponent
    {
      public:

        LegendComponent() = default;

        QObject *item = nullptr;

        //! Symbol size, not including any preset padding space around the symbol
        QSizeF symbolSize;

        //! Label size, not including any preset padding space around the label
        QSizeF labelSize;

        //! Component size
        QSizeF size;

        /**
         * Starting indent for groups/subgroups nested in other groups/subgroups.
         * This value is the sum of the indents of all parent groups/subgroups.
         */
        double indent = 0;

        /**
         * Horizontal offset for the symbol label.
         *
         * This offset is the same for all symbol labels belonging to the same layer,
         * within the same legend column.
         */
        double labelXOffset = 0.0;

        /**
         * Largest symbol width, considering all other sibling components associated with
         * this component.
         */
        double maxSiblingSymbolWidth = 0.0;
    };

    /**
     * An component group is an indivisible set of legend components (i.e. it is indivisible into more columns).
     *
     * A group may consist of one or more component(s), depending on the layer splitting mode:
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
    class LegendComponentGroup
    {
      public:

        //! List of child components belonging to this group.
        QList<LegendComponent> components;

        //! Group size, including internal spacing between components, but excluding any padding space around the group itself.
        QSizeF size = QSizeF( 0, 0 );

        //! Corresponding column index
        int column = 0;

        /**
         * TRUE if a forced column break should be placed just before the group
         */
        bool placeColumnBreakBeforeGroup = false;

    };

    /**
     * Contains contextual information about the current column being rendered
     */
    class ColumnContext
    {
      public:

        ColumnContext()
          : left( 0 )
          , right( 0 )
        {}

        //! Left edge of column
        double left = 0;
        //! Right edge of column
        double right = 0;
    };

    /**
     * Returns a list of component groups for the specified \a parentGroup, respecting the current layer's
     * splitting settings.
     */
    QList<LegendComponentGroup> createComponentGroupList( QgsLayerTreeGroup *parentGroup, QgsRenderContext &context, double indent = 0 );

    /**
     * Divides a list of component groups into columns, and sets the column index for each group in the list.
     *
     * Returns the calculated number of columns.
     */
    int setColumns( QList<LegendComponentGroup> &groupList );

    /**
     * Returns the calculated padding space required above the given component \a group.
     */
    double spaceAboveGroup( const LegendComponentGroup &group );

    /**
     * Renders a group item in a \a json object.
     *
     * \since QGIS 3.8
     */
    QJsonObject exportLegendToJson( const QgsRenderContext &context, QgsLayerTreeGroup *nodeGroup );

    /**
     * Draws the legend using the specified render \a context, and returns the actual size of the legend.
     *
     * If \a context is NULLPTR, only the size of the legend will be calculated and no
     * painting will be attempted.
     */
    QSizeF paintAndDetermineSize( QgsRenderContext &context );

    /**
     * Draws a title in the legend using the specified render \a context, with the title font and the specified alignment settings.
     *
     * Returns the size required to draw the complete title.
     *
     * If \a context is NULLPTR, no painting will be attempted, but the required size will still be calculated and returned.
     */
    QSizeF drawTitle( QgsRenderContext &context, double top, Qt::AlignmentFlag halignment = Qt::AlignLeft, double legendWidth = 0 );

    /**
     * Draws an \a group and return its actual size, using the specified render \a context.
     *
     * The \a group is drawn with the space above it, so that the first groups in a column are all
     * aligned to the same line regardless of their style top spacing.
     *
     * If \a context is NULLPTR, no painting will be attempted, but the required size will still be calculated and returned.
    */
    QSizeF drawGroup( const LegendComponentGroup &group, QgsRenderContext &context, ColumnContext columnContext, double top = 0 );

    /**
     * Draws the symbol of a given symbol QgsLayerTreeModelLegendNode, using the specified render \a context.
     */
    LegendComponent drawSymbolItem( QgsLayerTreeModelLegendNode *symbolItem, QgsRenderContext &context, ColumnContext columnContext, double top, double maxSiblingSymbolWidth = 0 );

    /**
     * Draws the title of a layer, given a QgsLayerTreeLayer, and a destination render \a context.
     *
     * Returns the size of the title.
     *
     * The \a context may be NULLPTR, in which case on the size is calculated and no painting is attempted.
     */
    QSizeF drawLayerTitle( QgsLayerTreeLayer *nodeLayer, QgsRenderContext &context, ColumnContext columnContext = ColumnContext(), double top = 0 );

    /**
     * Draws a group's title, using the specified render \a context.
     *
     * Returns the size of the title.
     */
    QSizeF drawGroupTitle( QgsLayerTreeGroup *nodeGroup, QgsRenderContext &context, ColumnContext columnContext = ColumnContext(), double top = 0 );

    /**
     * Returns the style of the given \a node.
     */
    QgsLegendStyle::Style nodeLegendStyle( QgsLayerTreeNode *node );

    QgsLayerTreeModel *mLegendModel = nullptr;

    QgsLegendSettings mSettings;

    QSizeF mLegendSize;

#endif

    void widthAndOffsetForTitleText( const Qt::AlignmentFlag halignment, double legendWidth, double &width, double &offset ) const;
};

#endif // QGSLEGENDRENDERER_H
