/***************************************************************************
                          qgsmarkersymbol.h  -  description
                             -------------------
    begin                : March 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
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

#ifndef QGSMARKERSYMBOL_H
#define QGSMARKERSYMBOL_H

#include "qgssymbol.h"
#include <qpicture.h>

/**Representation of a marker symbol*/
class QgsMarkerSymbol : public QgsSymbol
{
 public:
    /**Constructor*/
    QgsMarkerSymbol();
    /**Destructor*/
    virtual ~QgsMarkerSymbol();
    /**Loads the QPainter commands from an svg file
       @param svgpath the pathe to the svg file which stores the picture*/
    void setPicture(const QString& svgpath);
    /**Sets the scale factor*/
    void setScaleFactor(double factor);
    /**Returns the path of the picture object*/
    const QString& picture() const;
    /**Returns the scale factor*/
    double scaleFactor();
 protected:
    /**Path to the SVG image*/
    QString mSvgPath;
    /**Scale factor. 1 keeps the size as it is, 2 doubles the size, etc.*/
    double mScaleFactor;
};

inline QgsMarkerSymbol::QgsMarkerSymbol(): QgsSymbol(), mScaleFactor(1), 
     mSvgPath("")
{

}

inline QgsMarkerSymbol::~QgsMarkerSymbol()
{

}

inline void QgsMarkerSymbol::setScaleFactor(double factor)
{
    mScaleFactor=factor;
}

inline const QString& QgsMarkerSymbol::picture() const
{
    return mSvgPath;
}

inline double QgsMarkerSymbol::scaleFactor()
{
    return mScaleFactor;
}

#endif
