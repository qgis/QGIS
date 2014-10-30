/***************************************************************************
                         qgspaperitem.h
                       -------------------
    begin                : September 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPAPERITEM_H
#define QGSPAPERITEM_H

#include "qgscomposeritem.h"
#include <QGraphicsRectItem>

/**Item representing a grid. This is drawn separately to the underlying paper item since the grid needs to be
 * drawn above all other composer items, while the paper item is drawn below all others.*/
class CORE_EXPORT QgsPaperGrid: public QGraphicsRectItem
{
  public:
    QgsPaperGrid( double x, double y, double width, double height, QgsComposition* composition );
    ~QgsPaperGrid();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

  private:
    QgsComposition* mComposition;
};

/**Item representing the paper.*/
class CORE_EXPORT QgsPaperItem : public QgsComposerItem
{
  public:
    QgsPaperItem( QgsComposition* c );
    QgsPaperItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition );
    ~QgsPaperItem();

    /** return correct graphics item type. */
    virtual int type() const { return ComposerPaper; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     * @param doc is the Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    virtual void setSceneRect( const QRectF& rectangle );

  private:
    QgsPaperItem();
    /**Set flags and z-value*/
    void initialize();

    void calculatePageMargin();

    QgsPaperGrid* mPageGrid;
    double mPageMargin;
};

#endif
