/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposernodesitem.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerNodesItem: QgsComposerItem
{
%Docstring
An abstract composer item that provides generic methods for nodes based
shapes such as polygon or polylines.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgscomposernodesitem.h"
%End
  public:

    QgsComposerNodesItem( const QString &mTagName, QgsComposition *c );
%Docstring
Constructor

:param mTagName: tag used in XML file
:param c: parent composition
%End

    QgsComposerNodesItem( const QString &mTagName, const QPolygonF &polygon, QgsComposition *c );
%Docstring
Constructor

:param mTagName: tag used in XML file
:param polygon: nodes of the shape
:param c: parent composition
%End

    bool addNode( QPointF pt, const bool checkArea = true, const double radius = 10 );
%Docstring
Add a node in current shape.

:param pt: is the location of the new node
:param checkArea: is a flag to indicate if there's a space constraint.
:param radius: is the space contraint and is used only if checkArea is
true. Typically, if this flag is true, the new node has to be nearest
than radius to the shape to be added.
%End

    void setDisplayNodes( const bool display = true );
%Docstring
Set a tag to indicate if we want to draw or not the shape's nodes.

:param display:
%End

    bool moveNode( const int index, QPointF node );
%Docstring
Move a node to a new position.

:param index: the index of the node to move
:param node: is the new position in scene coordinate
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint - draw on canvas
%End

    int nodeAtPosition( QPointF node, const bool searchInRadius = true, const double radius = 10 );
%Docstring
Search the nearest node in shape within a maximal area. Returns the
index of the nearest node or -1.

:param node: is where a shape's node is searched
:param searchInRadius: is a flag to indicate if the area of research is
limited in space.
:param radius: is only used if searchInRadius is true
%End

    bool nodePosition( const int index, QPointF &position );
%Docstring
Gets the position of a node in scene coordinate.

:param index: of the node
:param position: the position of the node

:return: true if the index is valid and the position is set, false otherwise
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets state from Dom document

:param itemElem: is Dom node corresponding to item tag
:param doc: is Dom document
%End

    bool removeNode( const int index );
%Docstring
Remove a node from the shape.

:param index: of the node to delete
%End

    int nodesSize();
%Docstring
Returns the number of nodes in the shape.
%End

    bool setSelectedNode( const int index );
%Docstring
Select a node.

:param index: the node to select
%End

    int selectedNode();
%Docstring
Returns the currently selected node.

:return: the index of the selected node, -1 otherwise
%End

    void deselectNode();
%Docstring
Deselect a node.
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores state in Dom element

:param elem: is Dom element corresponding to 'Composer' tag
:param doc: write template file
%End

  protected:


    virtual bool _addNode( const int nodeIndex, QPointF newNode, const double radius ) = 0;
%Docstring
Method called in addNode.
%End

    virtual bool _removeNode( const int nodeIndex ) = 0;
%Docstring
Method called in removeNode.
%End

    virtual void _draw( QPainter *painter ) = 0;
%Docstring
Method called in paint.
%End

    virtual void _readXmlStyle( const QDomElement &elmt ) = 0;
%Docstring
Method called in readXml.
%End

    virtual void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt ) const = 0;
%Docstring
Method called in writeXml.
%End

    void rescaleToFitBoundingBox();
%Docstring
Rescale the current shape according to the boudning box. Useful when
the shape is resized thanks to the rubber band. *
%End

    double computeDistance( QPointF pt1, QPointF pt2 ) const;
%Docstring
Compute an euclidian distance between 2 nodes.
%End

    void updateSceneRect();
%Docstring
Update the current scene rectangle for this item.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposernodesitem.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
