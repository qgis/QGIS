/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposershape.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerShape: QgsComposerItem
{
%Docstring
A composer items that draws common shapes (ellipse, triangle, rectangle)*
%End

%TypeHeaderCode
#include "qgscomposershape.h"
%End
  public:

    enum Shape
    {
      Ellipse,
      Rectangle,
      Triangle
    };

    QgsComposerShape( QgsComposition *composition /TransferThis/ );
    QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition *composition /TransferThis/ );
    ~QgsComposerShape();

    virtual int type() const;
%Docstring
Return correct graphics item type.

:rtype: int
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint - draw on canvas
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores state in Dom element
:param elem: is Dom element corresponding to 'Composer' tag
:param doc: write template file

:rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets state from Dom document
:param itemElem: is Dom node corresponding to item tag
:param doc: is Dom document

:rtype: bool
%End

    QgsComposerShape::Shape shapeType() const;
%Docstring

:rtype: QgsComposerShape.Shape
%End
    void setShapeType( QgsComposerShape::Shape s );

    void setCornerRadius( double radius );
%Docstring
Sets radius for rounded rectangle corners. Added in v2.1
%End
    double cornerRadius() const;
%Docstring
Returns the radius for rounded rectangle corners

:rtype: float
%End

    void setShapeStyleSymbol( QgsFillSymbol *symbol );
%Docstring
Sets the QgsFillSymbol used to draw the shape. Must also call setUseSymbol( true ) to
enable drawing with a symbol.
Note: added in version 2.1*
%End

    QgsFillSymbol *shapeStyleSymbol();
%Docstring
Returns the QgsFillSymbol used to draw the shape.
Note: added in version 2.1*

:rtype: QgsFillSymbol
%End

    void setUseSymbol( bool useSymbol );
%Docstring
Controls whether the shape should be drawn using a QgsFillSymbol.
Note: Added in v2.1 *
%End

    virtual QRectF boundingRect() const;

%Docstring
Depending on the symbol style, the bounding rectangle can be larger than the shape

:rtype: QRectF

.. versionadded:: 2.3
%End

    virtual void setSceneRect( const QRectF &rectangle );

%Docstring
Sets new scene rectangle bounds and recalculates hight and extent. Reimplemented from
QgsComposerItem as it needs to call updateBoundingRect after the shape's size changes
%End

    virtual QString displayName() const;


  protected:
    virtual void drawFrame( QPainter *p );

%Docstring
reimplement drawFrame, since it's not a rect, but a custom shape */
%End
    virtual void drawBackground( QPainter *p );

%Docstring
reimplement drawBackground, since it's not a rect, but a custom shape */
%End

    virtual double estimatedFrameBleed() const;

%Docstring
Reimplement estimatedFrameBleed, since frames on shapes are drawn using symbology
rather than the item's pen *

:rtype: float
%End

  public slots:

    void refreshSymbol();
%Docstring
Should be called after the shape's symbol is changed. Redraws the shape and recalculates
its selection bounds.
Note: added in version 2.1*
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposershape.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
