/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgspaperitem.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsPaperGrid: QGraphicsRectItem
{
%Docstring
Item representing a grid. This is drawn separately to the underlying paper item since the grid needs to be
drawn above all other composer items, while the paper item is drawn below all others.
%End

%TypeHeaderCode
#include "qgspaperitem.h"
%End
  public:
    QgsPaperGrid( double x, double y, double width, double height, QgsComposition *composition );

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint
%End

};

class QgsPaperItem : QgsComposerItem
{
%Docstring
Item representing the paper.*
%End

%TypeHeaderCode
#include "qgspaperitem.h"
%End
  public:
    QgsPaperItem( QgsComposition *c /TransferThis/ );
    QgsPaperItem( qreal x, qreal y, qreal width, qreal height, QgsComposition *composition /TransferThis/ );
    ~QgsPaperItem();

    virtual int type() const;
%Docstring
Return correct graphics item type.
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores state in Dom element

:param elem: is Dom element corresponding to 'Composer' tag
:param doc: Dom document
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets state from Dom document

:param itemElem: is Dom node corresponding to item tag
:param doc: is the Dom document
%End

    virtual void setSceneRect( const QRectF &rectangle );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgspaperitem.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
