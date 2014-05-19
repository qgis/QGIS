/***************************************************************************
   qgsfieldproxymodel.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSFIELDPROXYMODEL_H
#define QGSFIELDPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "qgsfieldmodel.h"

/**
 * @brief The QgsFieldProxyModel class provides an easy to use model to display the list of fields of a layer.
 * @note added in 2.3
 */
class GUI_EXPORT QgsFieldProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_FLAGS( Filters )

  public:
    enum Filter
    {
      String = 1,
      Int = 2,
      LongLong = 4,
      Double = 8,
      Numeric = Int | LongLong | Double,
      Date = 16,
      All = Numeric | Date | String
    };
    Q_DECLARE_FLAGS( Filters, Filter )

    /**
     * @brief QgsFieldProxModel creates a proxy model with a QgsFieldModel as source model.
     * It can be used to filter the fields based on their types.
     */
    explicit QgsFieldProxyModel( QObject *parent = 0 );

    //! sourceFieldModel returns the QgsFieldModel used in this QSortFilterProxyModel
    QgsFieldModel* sourceFieldModel() { return mModel; }

    /**
     * @brief setFilters set flags that affect how fields are filtered
     * @param filters are Filter flags
     * @note added in 2.3
     */
    QgsFieldProxyModel* setFilters( Filters filters );
    const Filters& filters() const { return mFilters; }

  private:
    Filters mFilters;
    QgsFieldModel* mModel;

    // QSortFilterProxyModel interface
  public:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFieldProxyModel::Filters )

#endif // QGSFIELDPROXYMODEL_H
