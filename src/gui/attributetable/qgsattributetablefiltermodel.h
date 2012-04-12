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

class QgsAttributeTableModel;
class QgsVectorLayer;

class QgsAttributeTableFilterModel: public QSortFilterProxyModel
{
  public:
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

    QgsVectorLayer *layer() const { return mLayer; }
    QgsAttributeTableModel *tableModel() const { return reinterpret_cast<QgsAttributeTableModel*>( sourceModel() ); }

    void setHideUnselected( bool theFlag ) { mHideUnselected = theFlag; }

  protected:
    /**
     * Returns true if the source row will be accepted
     * @param sourceRow row from the source model
     * @param sourceParent parent index in the source model
     */
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;
  private:
    QgsVectorLayer* mLayer;
    bool mHideUnselected;
};

#endif
