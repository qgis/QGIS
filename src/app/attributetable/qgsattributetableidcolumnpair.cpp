/***************************************************************************
     QgsAttributeTableIdColumnPair.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributetableidcolumnpair.h"
#include "qgsfield.h"

#include <QVariant>

//could be faster when type guessed before sorting
bool QgsAttributeTableIdColumnPair::operator<( const QgsAttributeTableIdColumnPair &b ) const
{
  //QVariant thinks gid is a string!
  QVariant::Type columnType = columnItem.type();

  if ( columnType == QVariant::Int || columnType == QVariant::UInt || columnType == QVariant::LongLong || columnType == QVariant::ULongLong )
    return columnItem.toLongLong() < b.columnItem.toLongLong();

  if ( columnType == QVariant::Double )
    return columnItem.toDouble() < b.columnItem.toDouble();

  return columnItem.toString() < b.columnItem.toString();
}

