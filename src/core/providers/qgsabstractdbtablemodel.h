/***************************************************************************
   qgsabstractdbtablemodel.h
    --------------------------------------
   Date                 : 08.11.2021
   Copyright            : (C) 2021 Denis Rouzaud
   Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSABSTRACTDBTABLEMODEL_H
#define QGSABSTRACTDBTABLEMODEL_H

#include "qgis_core.h"

#include <QStandardItemModel>

/**
 * \ingroup gui
 * \brief The QgsAbstractDbTableModel class is a pure virtual model class for results in a database source widget selector
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsAbstractDbTableModel : public QStandardItemModel
{
    Q_OBJECT
  public:
    //! Constructor
    explicit QgsAbstractDbTableModel( QObject *parent = nullptr )
      : QStandardItemModel( parent )
    {}

    //! Returns the list of columns in the table
    virtual QStringList columns() const = 0;

    //! Returns the index of the column used by default to filter the results (probably the table name column if it exists)
    virtual int defaultSearchColumn() const = 0;

    //! Returns if the column should be searchable at the given index
    virtual bool searchableColumn( int column ) const {Q_UNUSED( column ) return true;}

    //! Sets an sql statement that belongs to a cell specified by a model index
    virtual void setSql( const QModelIndex &index, const QString &sql ) = 0;
};

#endif // QGSABSTRACTDBTABLEMODEL_H
