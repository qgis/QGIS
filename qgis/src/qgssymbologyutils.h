/***************************************************************************
                         qgssymbologyutils.h  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
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

#ifndef QGSSYMBOLOGYUTILS_H
#define QGSSYMBOLOGYUTILS_H

#include <qnamespace.h> 
#include <qstring.h>
#include <qpixmap.h>

/**Namespace containing static methods which are useful for the symbology widgets*/
namespace QgsSymbologyUtils
{
    QPixmap qString2LinePixmap(QString string);
    QPixmap char2LinePixmap(const char* c);
    QPixmap qString2PatternPixmap(QString string);
    QPixmap char2PatternPixmap(const char* c);
    QString penStyle2QString(Qt::PenStyle penstyle);
    const char* penStyle2Char(Qt::PenStyle penstyle);
    QPixmap penStyle2Pixmap(Qt::PenStyle penstyle);
    Qt::PenStyle qString2PenStyle(QString string);
    Qt::PenStyle char2PenStyle(const char* c);
    QString brushStyle2QString(Qt::BrushStyle brushstyle);
    const char* brushStyle2Char(Qt::BrushStyle brushstyle);
    QPixmap brushStyle2Pixmap(Qt::BrushStyle brushstyle);
    Qt::BrushStyle qString2BrushStyle(QString string);
    Qt::BrushStyle char2BrushStyle(const char* c);
}

#endif
