/***************************************************************************
  QgsAttributeTableFilterModel.h - Filter Model for attribute table
  -------------------
         date                 : Feb 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLEFILTERMODEL_H
#define QGSATTRIBUTETABLEFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <QModelIndex>

//QGIS Includes
#include "qgsvectorlayer.h" //QgsAttributeList

class QgsAttributeTableFilterModel: public QSortFilterProxyModel
{
  public:
    bool mHideUnselected;
    /**
     * Constructor
     * @param theLayer initializing layer pointer
     */
    QgsAttributeTableFilterModel( QgsVectorLayer* theLayer );
    /**
     * Sorts model by the column
     * @param column column to sort by
     * @param order sorting order
     */
    virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
    //QModelIndex mapToSource ( const QModelIndex & filterIndex ) const;
    //QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const;
  protected:
    /**
     * Returns true if the source row will be accepted
     * @param sourceRow row from the source model
     * @param sourceParent parent index in the source model
     */
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;
  private:
    QgsVectorLayer* mLayer;
};

#endif
