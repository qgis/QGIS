/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerpolygon.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsComposerPolygon: QgsComposerNodesItem
{
%Docstring
 Composer item for polygons.
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgscomposerpolygon.h"
%End
  public:

    QgsComposerPolygon( QgsComposition *c );
%Docstring
 Constructor
 \param c parent composition
%End

    QgsComposerPolygon( const QPolygonF &polygon, QgsComposition *c );
%Docstring
 Constructor
 \param polygon nodes of the shape
 \param c parent composition
%End

    virtual QString displayName() const;
%Docstring
Overridden to return shape name
 :rtype: str
%End

    QgsFillSymbol *polygonStyleSymbol();
%Docstring
Returns the QgsSymbol used to draw the shape.
 :rtype: QgsFillSymbol
%End

    void setPolygonStyleSymbol( QgsFillSymbol *symbol );
%Docstring
Set the QgsSymbol used to draw the shape.
%End

    virtual int type() const;
%Docstring
Return correct graphics item type.
 :rtype: int
%End

  protected:


    virtual bool _addNode( const int indexPoint, QPointF newPoint, const double radius );

%Docstring
 Add the node newPoint at the given position according to some
 criteres. *
 :rtype: bool
%End

    virtual bool _removeNode( const int nodeIndex );


    virtual void _draw( QPainter *painter );

%Docstring
Draw nodes for the current shape.
%End

    virtual void _readXmlStyle( const QDomElement &elmt );

%Docstring
Read symbol in XML.
%End

    virtual void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt ) const;

%Docstring
Write the symbol in an XML document.
%End

    void createDefaultPolygonStyleSymbol();
%Docstring
Create a default symbol.
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerpolygon.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
