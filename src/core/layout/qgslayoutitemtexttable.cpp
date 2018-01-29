/***************************************************************************
                         qgslayoutitemtexttable.cpp
                         --------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemtexttable.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoutframe.h"
#include "qgslayout.h"

QgsLayoutItemTextTable::QgsLayoutItemTextTable( QgsLayout *layout )
  : QgsLayoutTable( layout )
{

}

int QgsLayoutItemTextTable::type() const
{
  return QgsLayoutItemRegistry::LayoutTextTable;
}

QString QgsLayoutItemTextTable::displayName() const
{
  return tr( "<Text table frame>" );
}

QgsLayoutItemTextTable *QgsLayoutItemTextTable::create( QgsLayout *layout )
{
  return new QgsLayoutItemTextTable( layout );
}

void QgsLayoutItemTextTable::addRow( const QStringList &row )
{
  mRowText.append( row );
  refreshAttributes();
}

void QgsLayoutItemTextTable::setContents( const QVector<QStringList> &contents )
{
  mRowText = contents;
  refreshAttributes();
}

bool QgsLayoutItemTextTable::getTableContents( QgsLayoutTableContents &contents )
{
  contents.clear();

  for ( const QStringList &row : qgis::as_const( mRowText ) )
  {
    QgsLayoutTableRow currentRow;

    for ( int i = 0; i < mColumns.count(); ++i )
    {
      if ( i < row.count() )
      {
        currentRow << row.at( i );
      }
      else
      {
        currentRow << QString();
      }
    }
    contents << currentRow;
  }

  recalculateTableSize();
  return true;
}
