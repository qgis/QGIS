/***************************************************************************
                         qgscomposernodesitem.h
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

#ifndef QGSCOMPOSERNODESITEM_H
#define QGSCOMPOSERNODESITEM_H

#include "qgscomposeritem.h"
#include <QBrush>
#include <QPen>

/** \ingroup core
 * An abstract composer item that provides generic methods for nodes based
 * shapes such as polygon or polylines.
 * @note added in QGIS 2.16
 */
class CORE_EXPORT QgsComposerNodesItem: public QgsComposerItem
{
    Q_OBJECT

  public:

    /** Constructor
     * @param mTagName tag used in XML file
     * @param c parent composition
     */
    QgsComposerNodesItem( QString mTagName, QgsComposition* c );

    /** Constructor
     * @param mTagName tag used in XML file
     * @param polygon nodes of the shape
     * @param c parent composition
     */
    QgsComposerNodesItem( QString mTagName, QPolygonF polygon, QgsComposition* c );

    /** Destructor */
    ~QgsComposerNodesItem();

    /** Add a node in current shape.
     * @param pt is the location of the new node
     * @param checkArea is a flag to indicate if there's a space constraint.
     * @param radius is the space contraint and is used only if checkArea is
     * true. Typically, if this flag is true, the new node has to be nearest
     * than radius to the shape to be added.
     */
    bool addNode( const QPointF &pt, const bool checkArea = true, const double radius = 10 );

    /** Set a tag to indicate if we want to draw or not the shape's nodes.
     * @param display
     */
    void setDisplayNodes( const bool display = true ) { mDrawNodes = display; }

    /** Move a node to a new position.
     * @param index the index of the node to move
     * @param node is the new position in scene coordinate
     */
    bool moveNode( const int index, const QPointF &node );

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget ) override;

    /** Search the nearest node in shape within a maximal area. Returns the
     * index of the nearest node or -1.
     * @param node is where a shape's node is searched
     * @param searchInRadius is a flag to indicate if the area of research is
     * limited in space.
     * @param radius is only used if searchInRadius is true
     */
    int nodeAtPosition( QPointF node, const bool searchInRadius = true, const double radius = 10 );

    /** Gets the position of a node in scene coordinate.
      * @param index of the node
      * @param position the position of the node
      * @return true if the index is valid and the position is set, false otherwise
      */
    bool nodePosition( const int index, QPointF &position );

    /** Sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     * @param doc is Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) override;

    /** Remove a node from the shape.
     * @param index of the node to delete
     */
    bool removeNode( const int index );

    /** Returns the number of nodes in the shape. */
    int nodesSize() { return mPolygon.size(); }

    /** Select a node.
     * @param index the node to select
     */
    bool setSelectedNode( const int index );

    /** Returns the currently selected node.
      * @return the index of the selected node, -1 otherwise
      */
    int selectedNode() { return mSelectedNode; }

    /** Unselect a node.
     */
    void unselectNode() { mSelectedNode = -1; }

    /** Stores state in Dom element
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc write template file
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const override;

  protected:

    /** Storage meaning for shape's nodes. */
    QPolygonF mPolygon;

    /** Method called in addNode. */
    virtual bool _addNode( const int nodeIndex, const QPointF &newNode, const double radius ) = 0;

    /** Method called in removeNode. */
    virtual bool _removeNode( const int nodeIndex ) = 0;

    /** Method called in paint. */
    virtual void _draw( QPainter *painter ) = 0;

    /** Method called in readXML. */
    virtual void _readXMLStyle( const QDomElement &elmt ) = 0;

    /** Method called in writeXML. */
    virtual void _writeXMLStyle( QDomDocument &doc, QDomElement &elmt ) const = 0;

    /** Rescale the current shape according to the boudning box. Useful when
     * the shape is resized thanks to the rubber band. */
    void rescaleToFitBoundingBox();

    /** Compute an euclidian distance between 2 nodes. */
    double computeDistance( const QPointF &pt1, const QPointF &pt2 ) const;

    /** Update the current scene rectangle for this item. */
    void updateSceneRect();

  private:
    /** This tag is used to write the XML document. */
    QString mTagName;

    /** The index of the node currently selected. */
    int mSelectedNode;

    /** This tag is used to indicate if we have to draw nodes or not during
     * the painting. */
    bool mDrawNodes;

    /** Draw nodes */
    void drawNodes( QPainter *painter ) const;
    void drawSelectedNode( QPainter *painter ) const;
};

#endif // QGSCOMPOSERNODESITEM_H
