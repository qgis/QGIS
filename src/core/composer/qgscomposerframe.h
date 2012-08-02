/***************************************************************************
                              qgscomposerframe.h
    ------------------------------------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERFRAME_H
#define QGSCOMPOSERFRAME_H

#include "qgscomposeritem.h"

class QgsComposition;
class QgsComposerMultiFrame;

/**Frame for html, table, text which can be divided onto several frames*/
class QgsComposerFrame: public QgsComposerItem
{
  public:
    QgsComposerFrame( QgsComposition* c, QgsComposerMultiFrame* mf, qreal x, qreal y, qreal width, qreal height );
    ~QgsComposerFrame();

    /**Sets the part of this frame (relative to the total multiframe extent in mm)*/
    void setContentSection( const QRectF& section ) { mSection = section; }

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    int type() const { return ComposerFrame; }

    QgsComposerMultiFrame* multiFrame() { return mMultiFrame; }

  private:
    QgsComposerFrame(); //forbidden

    QgsComposition* mComposition;
    QgsComposerMultiFrame* mMultiFrame;
    QRectF mSection;
};

#endif // QGSCOMPOSERFRAME_H
