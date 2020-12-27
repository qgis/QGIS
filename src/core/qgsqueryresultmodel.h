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
#include <QThread>

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsabstractdatabaseproviderconnection.h"



///@cond private

#ifndef SIP_RUN

class ResultWorker: public QObject
{
    Q_OBJECT


  public:

    ResultWorker( const QgsAbstractDatabaseProviderConnection::QueryResult* queryResult )
      : mQueryResult( queryResult )
    {}

    void fetchRows();

    void stopFetching();

  signals:

    void rowsReady(int newRowsCount);

  private:

      const QgsAbstractDatabaseProviderConnection::QueryResult* mQueryResult = nullptr;
      QAtomicInt mStopFetching = 0;
      // Batch of rows to fetch before emitting rowsReady
      static const int ROWS_TO_FETCH;

};

#endif

///@encond private


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

    /**
     * Constructs a QgsQueryResultModel from a \a queryResult with optional \a parent
     */
    QgsQueryResultModel(const QgsAbstractDatabaseProviderConnection::QueryResult& queryResult, QObject *parent = nullptr );

    ~QgsQueryResultModel();

    // QAbstractItemModel interface
  public:

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

  public slots:

    void newRowsReady(int newRowsCount );

  private:

    const QgsAbstractDatabaseProviderConnection::QueryResult& mQueryResult;
    QStringList mColumns;
    qlonglong mRowCount = 0;
    QThread mWorkerThread;

};

#endif // qgsqueryresultmodel.h
