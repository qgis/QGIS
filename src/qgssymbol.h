/***************************************************************************
                          qgssymbol.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#ifndef QGSSYMBOL_H
#define QGSSYMBOL_H

#include <qbrush.h>
#include <qpen.h>

class QString;

/**Encapsulates settings for drawing*/
class QgsSymbol{

 public:
    /**Constructor*/
    QgsSymbol(QColor c = QColor(0,0,0));
    /**Sets the brush*/
    void setBrush(QBrush b);
    /**Gets a reference to m_brush*/
    QBrush& brush();
    /**Set the color*/
    void setColor(QColor c);
    /**Get the current color*/
    QColor color() const;
    /**Get the fill color*/
    QColor fillColor() const;
    /**Sets the fill color*/
    void setFillColor(QColor c);
    /**Get the line width*/
    int lineWidth() const;
    /**Sets the line width*/
    void setLineWidth(int w);
    /**Sets the pen*/
    void setPen(QPen p);
    /**Gets a reference to m_pen*/
    QPen& pen();
    //! Destructor
    ~QgsSymbol();

 protected:
    QPen m_pen;
    QBrush m_brush;
};

inline void QgsSymbol::setBrush(QBrush b)
{
    m_brush=b;
}

inline QBrush& QgsSymbol::brush()
{
    return m_brush;
}

inline void QgsSymbol::setPen(QPen p)
{
    m_pen=p;
}

inline QPen& QgsSymbol::pen()
{
    return m_pen;
}

#endif // QGSSYMBOL_H

       
