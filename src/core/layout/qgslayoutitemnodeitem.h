/***************************************************************************
                         qgslayoutitemnodeitem.h
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMNODEITEM_H
#define QGSLAYOUTITEMNODEITEM_H

#include "qgis_core.h"
#include "qgslayoutitem.h"

/**
 * \ingroup core
 * An abstract layout item that provides generic methods for node based
 * shapes such as polygon or polylines.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutNodesItem: public QgsLayoutItem
{
    Q_OBJECT

  public:

    /**
     * Sets the \a nodes the shape consists of.
     * \see nodes()
     */
    void setNodes( const QPolygonF &nodes );

    /**
     * Returns the nodes the shape consists of.
     * \see setNodes()
     */
    QPolygonF nodes() const { return mPolygon; }

    /**
     * Add a node in current shape.
     * \param point is the location of the new node (in scene coordinates)
     * \param checkArea is a flag to indicate if there's a space constraint.
     * \param radius is the space contraint and is used only if checkArea is
     * true. Typically, if this flag is TRUE, the new node has to be nearer
     * than radius to the shape to be added.
     */
    bool addNode( QPointF point, bool checkArea = true, double radius = 10 );

    /**
     * Set whether the item's nodes should be displayed.
     */
    void setDisplayNodes( bool display = true ) { mDrawNodes = display; }

    /**
     * Moves a node to a new position.
     * \param index the index of the node to move
     * \param node is the new position in scene coordinate
     */
    bool moveNode( int index, QPointF node );

    /**
     * Search for the nearest node in the shape within a maximal area. Returns the
     * index of the nearest node or -1 if no node was found.
     * \param point is the location to search for nodes from (in scene coordinates)
     * \param searchInRadius is a flag to indicate if the area of research is
     * limited in space.
     * \param radius is only used if searchInRadius is TRUE
     */
    int nodeAtPosition( QPointF point, bool searchInRadius = true, double radius = 10 ) const;

    /**
     * Gets the position of a node in scene coordinates.
      * \param index of the node
      * \param position the position of the node
      * \returns TRUE if the index is valid and the position is set, FALSE otherwise
      */
    bool nodePosition( int index, QPointF &position ) const;

    /**
     * Remove a node with specified \a index from the shape.
     */
    bool removeNode( int index );

    //! Returns the number of nodes in the shape.
    int nodesSize() const { return mPolygon.size(); }

    /**
     * Selects a node by \a index.
     */
    bool setSelectedNode( int index );

    /**
     * Returns the currently selected node, or -1 if no node is selected.
      */
    int selectedNode() const { return mSelectedNode; }

    /**
     * Deselects any selected nodes.
     */
    void deselectNode() { mSelectedNode = -1; }

    // Depending on the symbol style, the bounding rectangle can be larger than the shape
    QRectF boundingRect() const override;

    // Reimplement estimatedFrameBleed, since frames on shapes are drawn using symbology
    // rather than the item's pen
    double estimatedFrameBleed() const override;

  protected:

    /**
     * Constructor for QgsLayoutNodesItem, attached to the specified \a layout.
     */
    QgsLayoutNodesItem( QgsLayout *layout );

    /**
     * Constructor for a QgsLayoutNodesItem with the given \a polygon nodes, attached to the specified \a layout.
     */
    QgsLayoutNodesItem( const QPolygonF &polygon, QgsLayout *layout );

    void draw( QgsLayoutItemRenderContext &context ) override;

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

    //! Shape's nodes.
    QPolygonF mPolygon;

    //! Max symbol bleed
    double mMaxSymbolBleed = 0.0;

    //! Method called in addNode.
    virtual bool _addNode( int nodeIndex, QPointF newNode, double radius ) = 0;

    //! Method called in removeNode.
    virtual bool _removeNode( int nodeIndex ) = 0;

    //! Method called in paint.
    virtual void _draw( QgsLayoutItemRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) = 0;

    //! Method called in readXml.
    virtual void _readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context ) = 0;

    //! Method called in writeXml.
    virtual void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const = 0;

    /**
     * Rescale the current shape according to the item's bounding box. Useful when
     * the shape is resized thanks to the rubber band.
    */
    void rescaleToFitBoundingBox();

    //! Compute an euclidian distance between 2 nodes.
    double computeDistance( QPointF pt1, QPointF pt2 ) const;

    //! Update the current scene rectangle for this item.
    void updateSceneRect();

    //! Current bounding rectangle of shape
    QRectF mCurrentRectangle;

  protected slots:

    /**
     * Called when the bounding rect of the item should recalculated. Subclasses should update
     * currentRectangle in their implementations.
     */
    virtual void updateBoundingRect();

  private:

    void init();

    //! The index of the node currently selected.
    int mSelectedNode = -1;

    /**
     * This tag is used to indicate if we have to draw nodes or not during
     * the painting. */
    bool mDrawNodes = false;

    //! Draw nodes
    void drawNodes( QgsLayoutItemRenderContext &context ) const;
    void drawSelectedNode( QgsLayoutItemRenderContext &context ) const;

};

#endif // QGSLAYOUTITEMNODEITEM_H
