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

/**Item representing the paper. May draw the snapping grid lines if composition is in
 preview mode*/
class CORE_EXPORT QgsPaperItem: public QgsComposerItem
{
  public:
    QgsPaperItem( QgsComposition* c );
    QgsPaperItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition );
    ~QgsPaperItem();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** stores state in Dom node
       * @param node is Dom node corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  private:
    QgsPaperItem();
    /**Set flags and z-value*/
    void initialize();
};

#endif
