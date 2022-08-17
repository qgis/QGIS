/***************************************************************************
                             qgstreewidgetitem.cpp
                             ---------------------
    begin                : 06 Nov, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstreewidgetitem.h"
#include "qgsvariantutils.h"
#include "qgis.h"

QgsTreeWidgetItem::QgsTreeWidgetItem( QTreeWidget *parent, int type )
  : QTreeWidgetItem( parent, type )
{}

QgsTreeWidgetItem::QgsTreeWidgetItem( int type )
  : QTreeWidgetItem( type )
{}

QgsTreeWidgetItem::QgsTreeWidgetItem( const QStringList &strings, int type )
  : QTreeWidgetItem( strings, type )
{}

QgsTreeWidgetItem::QgsTreeWidgetItem( QTreeWidget *view, const QStringList &strings, int type )
  : QTreeWidgetItem( view, strings, type )
{}

QgsTreeWidgetItem::QgsTreeWidgetItem( QTreeWidget *view, QTreeWidgetItem *after, int type )
  : QTreeWidgetItem( view, after, type )
{}

QgsTreeWidgetItem::QgsTreeWidgetItem( QTreeWidgetItem *parent, int type )
  : QTreeWidgetItem( parent, type )
{}

QgsTreeWidgetItem::QgsTreeWidgetItem( QTreeWidgetItem *parent, const QStringList &strings, int type )
  : QTreeWidgetItem( parent, strings, type )
{}

QgsTreeWidgetItem::QgsTreeWidgetItem( QTreeWidgetItem *parent, QTreeWidgetItem *after, int type )
  : QTreeWidgetItem( parent, after, type )
{}

void QgsTreeWidgetItem::setSortData( int column, const QVariant &value )
{
  setData( column, CustomSortRole, value );
}

QVariant QgsTreeWidgetItem::sortData( int column ) const
{
  return data( column, CustomSortRole );
}

void QgsTreeWidgetItem::setAlwaysOnTopPriority( int priority )
{
  setData( 0, AlwaysOnTopPriorityRole, priority );
}

int QgsTreeWidgetItem::alwaysOnTopPriority() const
{
  const QVariant val = data( 0, AlwaysOnTopPriorityRole );
  return val.isValid() ? val.toInt() : -1;
}

bool QgsTreeWidgetItem::operator<( const QTreeWidgetItem &other ) const
{
  const int column = treeWidget()->sortColumn();

  // check always on top priority - note - no way of determining sort order from tree widget, so
  // these will sometimes be incorrectly placed at the bottom
  const QVariant priority1 = data( 0, AlwaysOnTopPriorityRole );
  const QVariant priority2 = other.data( 0, AlwaysOnTopPriorityRole );
  if ( priority1.isValid() && priority2.isValid() )
  {
    const int i1 = priority1.toInt();
    const int i2 = priority2.toInt();
    if ( i1 != i2 )
      return priority1.toInt() < priority2.toInt();
  }
  else if ( priority1.isValid() )
  {
    return true;
  }
  else if ( priority2.isValid() )
  {
    return false;
  }

  // no always on top priorities, check data
  bool ok1, ok2, val;

  // prefer the custom sort role, but fall back to display text
  QVariant val1 = data( column, CustomSortRole );
  if ( !val1.isValid() )
    val1 = text( column );
  QVariant val2 = other.data( column, CustomSortRole );
  if ( !val2.isValid() )
    val2 = other.text( column );

  if ( !QgsVariantUtils::isNull( val1 ) && !QgsVariantUtils::isNull( val2 ) )
  {
    val = val1.toDouble( &ok1 ) < val2.toDouble( &ok2 );
    if ( ok1 && ok2 )
    {
      return val;
    }
    else if ( ok1 || ok2 )
    {
      // sort numbers before strings
      return ok1;
    }
  }

  return qgsVariantLessThan( val1, val2 );
}

//
// QgsTreeWidgetItemObject
//

QgsTreeWidgetItemObject::QgsTreeWidgetItemObject( int type )
  : QgsTreeWidgetItem( type )
{}

QgsTreeWidgetItemObject::QgsTreeWidgetItemObject( QTreeWidget *parent, int type )
  : QgsTreeWidgetItem( parent, type )
{}

// override setData to emit signal when edited. By default the itemChanged signal fires way too often
void QgsTreeWidgetItemObject::setData( int column, int role, const QVariant &value )
{
  QgsTreeWidgetItem::setData( column, role, value );
  if ( role == Qt::EditRole )
  {
    emit itemEdited( this, column );
  }
}
