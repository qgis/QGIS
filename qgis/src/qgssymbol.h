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
/* $Id$ */

#ifndef QGSSYMBOL_H
#define QGSSYMBOL_H

#include <iostream>

#include <qbrush.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpicture.h>
#include <qdom.h>

class QString;

/**Encapsulates settings for drawing*/
class QgsSymbol{

 public:
    /**Hardcoded Point symbol*/
    enum PointSymbol {
	Circle,    // 'circle'
	Rectangle, // 'rectangle'  
	Diamond,   // 'diamond'
	Cross,     // 'cross'     +
	Cross2,    // 'cross2'    x
	NPointSymbols // number of available point symbols
    };
     
    /**Constructor*/
    QgsSymbol();
    /**Constructor*/
    QgsSymbol(QColor c);
    /**Sets the brush*/
    virtual void setBrush(QBrush b);
    /**Gets a reference to m_brush, Don't use the brush to change color/style */
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
    /**Gets a reference to m_pen. Don't use the pen to change color/style  */
    virtual QPen& pen();

    /**Set the line (pen) style*/
    virtual void setLineStyle(Qt::PenStyle s);
    /**Set the fill (brush) style*/
    virtual void setFillStyle(Qt::BrushStyle s);

    /**Set point symbol*/
    virtual void setPointSymbol(PointSymbol ps);
    /**Set point symbol from name*/
    virtual void setNamedPointSymbol(QString name);
    /**Get point symbol*/
    virtual PointSymbol pointSymbol() const;
    /**Get point symbol*/
    virtual QString pointSymbolName() const;
    /**Set size*/
    virtual void setPointSize(int s);
    /**Get size*/
    virtual int pointSize() const;
    //! Destructor
    virtual ~QgsSymbol();
    //! Get a little icon / image representation of this symbol
    virtual QPixmap getSymbolAsPixmap(int xDim, int yDim);
    //! Get a little icon / image representation of point symbol with current settings
    virtual QPixmap getPointSymbolAsPixmap();
    //! Get QPicture representation of point symbol with current settings
    virtual QPicture getPointSymbolAsPicture();
    /**Writes the contents of the symbol to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & item, QDomDocument & document );
    /**Reads the contents of the symbol from a configuration file
     @ return true in case of success*/
    virtual bool readXML( QDomNode & symbol );
 protected:
    QPen mPen;
    QBrush mBrush;
    /* Point symbol */
    PointSymbol mPointSymbol;
    /* Point size */
    int mPointSize; 
    /* Point symbol cache */
    QPixmap mPointSymbolPixmap;
    /* Point symbol cache */
    QPicture mPointSymbolPicture;
    /* Create point symbol cache */
    void cache(void);
    /* Cache updated */
    bool mCacheUpToDate;
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


