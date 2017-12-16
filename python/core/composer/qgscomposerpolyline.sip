/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerpolyline.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerPolyline: QgsComposerNodesItem
{
%Docstring
Composer item for polylines.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgscomposerpolyline.h"
%End
  public:

    QgsComposerPolyline( QgsComposition *c );
%Docstring
Constructor

:param c: parent composition
%End

    QgsComposerPolyline( const QPolygonF &polyline, QgsComposition *c );
%Docstring
Constructor

:param polyline: nodes of the shape
:param c: parent composition
%End

    virtual QString displayName() const;

%Docstring
Overridden to return shape name
%End

    QgsLineSymbol *polylineStyleSymbol();
%Docstring
Returns the QgsSymbol used to draw the shape.
%End

    void setPolylineStyleSymbol( QgsLineSymbol *symbol );
%Docstring
Set the QgsSymbol used to draw the shape.
%End

    virtual int type() const;
%Docstring
Overridden to return shape type
%End

  protected:


    virtual bool _addNode( const int indexPoint, QPointF newPoint, const double radius );

%Docstring
Add the node newPoint at the given position according to some
criteres. *
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

    void createDefaultPolylineStyleSymbol();
%Docstring
Create a default symbol.
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerpolyline.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
