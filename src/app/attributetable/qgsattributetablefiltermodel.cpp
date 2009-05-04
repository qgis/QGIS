/***************************************************************************
     QgsAttributeTableFilterModel.cpp
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

#include "qgsattributetablefiltermodel.h"
#include "qgsattributetablemodel.h"
#include "qgsvectorlayer.h"

//////////////////
// Filter Model //
//////////////////

void QgsAttributeTableFilterModel::sort( int column, Qt::SortOrder order )
{
  (( QgsAttributeTableModel * )sourceModel() )->sort( column, order );
}

QgsAttributeTableFilterModel::QgsAttributeTableFilterModel( QgsVectorLayer* theLayer )
{
  mLayer = theLayer;
  mHideUnselected = false;
  setDynamicSortFilter( true );
}

bool QgsAttributeTableFilterModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( mHideUnselected )
    // unreadable? yes, i agree :-)
    return mLayer->selectedFeaturesIds().contains((( QgsAttributeTableModel * )sourceModel() )->rowToId( sourceRow ) );

  return true;
}

/*
QModelIndex QgsAttributeTableFilterModel::mapFromSource ( const QModelIndex& sourceIndex ) const
{
  return sourceIndex;
}

QModelIndex QgsAttributeTableFilterModel::mapToSource ( const QModelIndex& filterIndex ) const
{
  return filterIndex;
}
*/

