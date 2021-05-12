/***************************************************************************
  qgsqueryresultwidget.h - QgsQueryResultWidget

 ---------------------
 begin                : 14.1.2021
 copyright            : (C) 2021 by elpaso
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSQUERYRESULTWIDGET_H
#define QGSQUERYRESULTWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "ui_qgsqueryresultwidgetbase.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsqueryresultmodel.h"
#include "qgsfeedback.h"

#include <QWidget>
#include <QThread>
#include <QtConcurrent>

///@cond private

#ifndef SIP_RUN

/**
 * \brief The QgsConnectionsApiFetcher class fetches tokens (table and field names) of a connection from a separate thread.
 */
class GUI_EXPORT QgsConnectionsApiFetcher: public QObject
{
    Q_OBJECT

  public:

    //! Constructs a result fetcher from \a queryResult
    QgsConnectionsApiFetcher( const QgsAbstractDatabaseProviderConnection *conn )
      : mConnection( conn )
    {}

    //! Start fetching
    void fetchTokens();

    //! Stop fetching
    void stopFetching();

  signals:

    //!! Emitted when \a newTokens have been fetched
    void tokensReady( const QStringList &newTokens );

    //! Emitted when fetching of tokes has finished or has been interrupted.
    void fetchingFinished();

  private:

    const QgsAbstractDatabaseProviderConnection *mConnection;
    QAtomicInt mStopFetching = 0;

};

#endif

///@endcond private

/**
 * \brief The QgsQueryResultWidget class allows users to enter and run an SQL query on a
 * DB connection (an instance of QgsAbstractDatabaseProviderConnection).
 *
 * Query results are displayed in a table view.
 * Query execution and result fetching can be interrupted by pressing the "Stop" push button.
 *
 * \note the ownership of the connection is transferred to the widget.
 *
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsQueryResultWidget: public QWidget, private Ui::QgsQueryResultWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Creates a QgsQueryResultWidget with the given \a connection, ownership is transferred to the widget.
     */
    QgsQueryResultWidget( QWidget *parent = nullptr, QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER = nullptr );

    virtual ~QgsQueryResultWidget();

    /**
     * Set the connection to \a connection, ownership is transferred to the widget.
     */
    void setConnection( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER );

    /**
     * Convenience method to set the SQL editor text to \a sql.
     */
    void setQuery( const QString &sql );

    /**
     * Set the SQL layer \a options. This method automatically populates and shows the "Load as new layer" panel.
     */
    void setSqlVectorLayerOptions( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options );

    /**
     * Returns the sqlVectorLayerOptions
     */
    QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions sqlVectorLayerOptions() const;

  public slots:

    /**
     * Starts executing the query.
     */
    void executeQuery();

    /**
      * Hides the result table and shows the error \a title and \a message in the message bar or
      * in the SQL error panel is \a isSqlError is set.
      */
    void showError( const QString &title, const QString &message, bool isSqlError = false );

    /**
     * Triggered when the threaded API fetcher has new \a tokens to add.
     */
    void tokensReady( const QStringList &tokens );

  private slots:

    void syncSqlOptions();

  signals:

    /**
     * Emitted when a new vector SQL (query) layer must be created.
     * \param providerKey name of the data provider
     * \param connectionUri the connection URI as returned by QgsAbstractProviderConnection::uri()
     * \param options
     */
    void createSqlVectorLayer( const QString &providerKey, const QString &connectionUri, const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options );

    /**
     * Emitted when the first batch of results has been fetched.
     * \note If the query returns no results this signal is not emitted.
     */
    void firstResultBatchFetched();

  private:

    std::unique_ptr<QgsAbstractDatabaseProviderConnection> mConnection;
    std::unique_ptr<QgsQueryResultModel> mModel;
    std::unique_ptr<QgsFeedback> mFeedback;
    QgsConnectionsApiFetcher *mApiFetcher = nullptr;
    QThread mWorkerThread;
    bool mWasCanceled = false;
    QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions mSqlVectorLayerOptions;
    bool mFirstRowFetched = false;
    QFutureWatcher<QgsAbstractDatabaseProviderConnection::QueryResult> mQueryResultWatcher;
    QString mSqlErrorMessage;

    /**
     * Updates buttons status
     */
    void updateButtons();

    /**
     * Updates SQL layer columns
     */
    void updateSqlLayerColumns();

    /**
     * Cancel and wait for finish
     */
    void cancelRunningQuery();


    /**
     * Starts the model population after initial query run
     */
    void startFetching();

    friend class TestQgsQueryResultWidget;

};

#endif // QGSQUERYRESULTWIDGET_H
