/***************************************************************************
  qgsqueryresultmodel.h - QgsQueryResultModel

 ---------------------
 begin                : 24.12.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QgsQueryResultModel_H
#define QgsQueryResultModel_H

#include <QAbstractListModel>

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsabstractdatabaseproviderconnection.h"

/**
 * The QgsQueryResultModel class is a model for QgsAbstractDatabaseProviderConnection::QueryResult
 *
 * \ingroup core
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsQueryResultModel : public QAbstractListModel
{
    Q_OBJECT
  public:

    QgsQueryResultModel( const QgsAbstractDatabaseProviderConnection::QueryResult &queryResult, QObject *parent = nullptr );

    // QAbstractItemModel interface
  public:

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    void fetchMore( const QModelIndex &parent ) override;
    bool canFetchMore( const QModelIndex &parent ) const override;

  private:

    QgsAbstractDatabaseProviderConnection::QueryResult mQueryResult;
    qlonglong mRowCount = 0;
    // Batch of rows to fetch
    static const int ROWS_TO_FETCH;
};

#endif // qgsqueryresultmodel.h
