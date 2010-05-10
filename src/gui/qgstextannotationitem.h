/***************************************************************************
                              qgstextannotationitem.h
                              ------------------------
  begin                : February 9, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTANNOTATIONITEM_H
#define QGSTEXTANNOTATIONITEM_H

#include "qgsannotationitem.h"
#include <QTextDocument>

/**An annotation item that displays formated text*/
class GUI_EXPORT QgsTextAnnotationItem: public QgsAnnotationItem
{
  public:
    QgsTextAnnotationItem( QgsMapCanvas* canvas );
    ~QgsTextAnnotationItem();

    /**Returns document (caller takes ownership)*/
    QTextDocument* document() const;
    /**Sets document (does not take ownership)*/
    void setDocument( const QTextDocument* doc );

    void writeXML( QDomDocument& doc ) const;
    void readXML( const QDomDocument& doc, const QDomElement& itemElem );

    void paint( QPainter* painter );

  private:
    QTextDocument* mDocument;
};

#endif // QGSTEXTANNOTATIONITEM_H
