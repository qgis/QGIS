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
    QgsSymbol();
    /**Constructor*/
    QgsSymbol(QColor c);
    /**Sets the brush*/
    virtual void setBrush(QBrush b);
    /**Gets a reference to m_brush*/
    virtual QBrush& brush();
    /**Set the color*/
    virtual void setColor(QColor c);
    /**Get the current color*/
    virtual QColor color() const;
    /**Get the fill color*/
    virtual QColor fillColor() const;
    /**Sets the fill color*/
    virtual void setFillColor(QColor c);
    /**Get the line width*/
    virtual int lineWidth() const;
    /**Sets the line width*/
    virtual void setLineWidth(int w);
    /**Sets the pen*/
    virtual void setPen(QPen p);
    /**Gets a reference to m_pen*/
    virtual QPen& pen();
    //! Destructor
    virtual ~QgsSymbol();

 protected:
    QPen mPen;
    QBrush mBrush;
};

inline void QgsSymbol::setBrush(QBrush b)
{
    mBrush=b;
}

inline QBrush& QgsSymbol::brush()
{
    return mBrush;
}

inline void QgsSymbol::setPen(QPen p)
{
    mPen=p;
}

inline QPen& QgsSymbol::pen()
{
    return mPen;
}

#endif // QGSSYMBOL_H

       
