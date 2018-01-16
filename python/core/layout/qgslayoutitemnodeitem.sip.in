/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemnodeitem.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutNodesItem: QgsLayoutItem
{
%Docstring
An abstract layout item that provides generic methods for node based
shapes such as polygon or polylines.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemnodeitem.h"
%End
  public:

    void setNodes( const QPolygonF &nodes );
%Docstring
Sets the ``nodes`` the shape consists of.

.. seealso:: :py:func:`nodes`
%End

    QPolygonF nodes() const;
%Docstring
Returns the nodes the shape consists of.

.. seealso:: :py:func:`setNodes`
%End

    bool addNode( QPointF point, bool checkArea = true, double radius = 10 );
%Docstring
Add a node in current shape.

:param point: is the location of the new node (in scene coordinates)
:param checkArea: is a flag to indicate if there's a space constraint.
:param radius: is the space contraint and is used only if checkArea is
true. Typically, if this flag is true, the new node has to be nearer
than radius to the shape to be added.
%End

    void setDisplayNodes( bool display = true );
%Docstring
Set whether the item's nodes should be displayed.
%End

    bool moveNode( int index, QPointF node );
%Docstring
Moves a node to a new position.

:param index: the index of the node to move
:param node: is the new position in scene coordinate
%End

    int nodeAtPosition( QPointF point, bool searchInRadius = true, double radius = 10 ) const;
%Docstring
Search for the nearest node in the shape within a maximal area. Returns the
index of the nearest node or -1 if no node was found.

:param point: is the location to search for nodes from (in scene coordinates)
:param searchInRadius: is a flag to indicate if the area of research is
limited in space.
:param radius: is only used if searchInRadius is true
%End

    bool nodePosition( int index, QPointF &position ) const;
%Docstring
Gets the position of a node in scene coordinates.

:param index: of the node
:param position: the position of the node

:return: true if the index is valid and the position is set, false otherwise
%End

    bool removeNode( int index );
%Docstring
Remove a node with specified ``index`` from the shape.
%End

    int nodesSize() const;
%Docstring
Returns the number of nodes in the shape.
%End

    bool setSelectedNode( int index );
%Docstring
Selects a node by ``index``.
%End

    int selectedNode() const;
%Docstring
Returns the currently selected node, or -1 if no node is selected.
%End

    void deselectNode();
%Docstring
Deselects any selected nodes.
%End

    virtual QRectF boundingRect() const;


    virtual double estimatedFrameBleed() const;


  protected:

    QgsLayoutNodesItem( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutNodesItem, attached to the specified ``layout``.
%End

    QgsLayoutNodesItem( const QPolygonF &polygon, QgsLayout *layout );
%Docstring
Constructor for a QgsLayoutNodesItem with the given ``polygon`` nodes, attached to the specified ``layout``.
%End

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );


    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );




    virtual bool _addNode( const int nodeIndex, QPointF newNode, const double radius ) = 0;
%Docstring
Method called in addNode.
%End

    virtual bool _removeNode( const int nodeIndex ) = 0;
%Docstring
Method called in removeNode.
%End

    virtual void _draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 ) = 0;
%Docstring
Method called in paint.
%End

    virtual void _readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context ) = 0;
%Docstring
Method called in readXml.
%End

    virtual void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const = 0;
%Docstring
Method called in writeXml.
%End

    void rescaleToFitBoundingBox();
%Docstring
Rescale the current shape according to the item's bounding box. Useful when
the shape is resized thanks to the rubber band.
%End

    double computeDistance( QPointF pt1, QPointF pt2 ) const;
%Docstring
Compute an euclidian distance between 2 nodes.
%End

    void updateSceneRect();
%Docstring
Update the current scene rectangle for this item.
%End


  protected slots:

    virtual void updateBoundingRect();
%Docstring
Called when the bounding rect of the item should recalculated. Subclasses should update
currentRectangle in their implementations.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemnodeitem.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
