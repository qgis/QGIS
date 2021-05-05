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

#include <QAbstractTableModel>
#include <QThread>

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsabstractdatabaseproviderconnection.h"

///@cond private

#ifndef SIP_RUN

/**
 * The QgsQueryResultFetcher class fetches query results from a separate thread
 */
class QgsQueryResultFetcher: public QObject
{
    Q_OBJECT

  public:

    //! Constructs a result fetcher from \a queryResult
    QgsQueryResultFetcher( const QgsAbstractDatabaseProviderConnection::QueryResult *queryResult )
      : mQueryResult( queryResult )
    {}

    //! Start fetching
    void fetchRows();

    //! Stop fetching
    void stopFetching();

  signals:

    //! Emitted when \a newRows have been fetched
    void rowsReady( const QList<QList<QVariant>> &newRows );

  private:

    const QgsAbstractDatabaseProviderConnection::QueryResult *mQueryResult = nullptr;
    QAtomicInt mStopFetching = 0;
    // Batch of rows to fetch before emitting rowsReady
    static const int ROWS_TO_FETCH;

};

#endif

///@endcond private


/**
 * \brief The QgsQueryResultModel class is a model for QgsAbstractDatabaseProviderConnection::QueryResult
 *
 * \ingroup core
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsQueryResultModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    /**
     * Constructs a QgsQueryResultModel from a \a queryResult with optional \a parent
     */
    QgsQueryResultModel( const QgsAbstractDatabaseProviderConnection::QueryResult &queryResult, QObject *parent = nullptr );

    ~QgsQueryResultModel();

    // QAbstractItemModel interface
  public:

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

  public slots:

    /**
     * Triggered when \a newRows have been fetched and can be added to the model
     */
    void rowsReady( const QList<QList<QVariant> > &rows );

    /**
     * Cancels the row fetching.
     */
    void cancel();

  private:

    QgsAbstractDatabaseProviderConnection::QueryResult mQueryResult;
    QStringList mColumns;
    QThread mWorkerThread;
    QgsQueryResultFetcher *mWorker = nullptr;
    QList<QVariantList> mRows;

};

#endif // qgsqueryresultmodel.h
