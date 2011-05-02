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
/* $Id$ */

#ifndef QGSSYMBOLOGYUTILS_H
#define QGSSYMBOLOGYUTILS_H

#include <Qt>

class QPixmap;
class QString;

/**Namespace containing static methods which are useful for the symbology widgets*/
namespace QgsSymbologyUtils
{
  QPixmap CORE_EXPORT qString2LinePixmap( QString string );
  QPixmap CORE_EXPORT char2LinePixmap( const char* c );
  QPixmap CORE_EXPORT qString2PatternPixmap( QString string );
  QPixmap CORE_EXPORT char2PatternPixmap( const char* c );
  QString CORE_EXPORT penStyle2QString( Qt::PenStyle penstyle );
  const char CORE_EXPORT * penStyle2Char( Qt::PenStyle penstyle );
  QPixmap CORE_EXPORT penStyle2Pixmap( Qt::PenStyle penstyle );
  Qt::PenStyle CORE_EXPORT qString2PenStyle( QString string );
  Qt::PenStyle CORE_EXPORT char2PenStyle( const char* c );
  QString CORE_EXPORT brushStyle2QString( Qt::BrushStyle brushstyle );
  const char CORE_EXPORT * brushStyle2Char( Qt::BrushStyle brushstyle );
  QPixmap CORE_EXPORT brushStyle2Pixmap( Qt::BrushStyle brushstyle );
  Qt::BrushStyle CORE_EXPORT qString2BrushStyle( QString string );
  Qt::BrushStyle CORE_EXPORT char2BrushStyle( const char* c );
}

#endif
