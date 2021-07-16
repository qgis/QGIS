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
#include <QStyledItemDelegate>

///@cond private

#ifndef SIP_RUN

/**
 * The QgsQueryResultItemDelegate class shows results truncated to 255 characters and using current locale
 */
class GUI_EXPORT QgsQueryResultItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

    // QStyledItemDelegate interface
  public:

    explicit QgsQueryResultItemDelegate( QObject *parent = nullptr );

    QString displayText( const QVariant &value, const QLocale &locale ) const override;
};

/**
 * The QgsConnectionsApiFetcher class fetches tokens (schema, table and field names) of a connection from a separate thread.
 * WARNING: this class is an implementation detail and it is not part of public API!
 */
class GUI_EXPORT QgsConnectionsApiFetcher: public QObject
{
    Q_OBJECT

  public:

    //! Constructs a result fetcher from \a connection.
    QgsConnectionsApiFetcher( const QgsAbstractDatabaseProviderConnection *connection )
      : mConnection( connection )
    {}

    //! Start fetching
    void fetchTokens();

    //! Stop fetching
    void stopFetching();

  signals:

    //!! Emitted when \a newTokens have been fetched.
    void tokensReady( const QStringList &newTokens );

    //! Emitted when fetching of tokes has finished or has been interrupted.
    void fetchingFinished();

  private:

    const QgsAbstractDatabaseProviderConnection *mConnection = nullptr;
    QAtomicInt mStopFetching = 0;

};

#endif

///@endcond private

/**
 * \ingroup gui
 * \brief The QgsQueryResultWidget class allows users to enter and run an SQL query on a
 * DB connection (an instance of QgsAbstractDatabaseProviderConnection).
 *
 * Query results are displayed in a table view.
 * Query execution and result fetching can be interrupted by pressing the "Stop" push button.
 *
 * \note the ownership of the connection is transferred to the widget.
 *
 * \since QGIS 3.22
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
     * Sets the connection to \a connection, ownership is transferred to the widget.
     */
    void setConnection( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER );

    /**
     * Convenience method to set the SQL editor text to \a sql.
     */
    void setQuery( const QString &sql );


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

  private slots:

    void syncSqlOptions();

    /**
     * Updates buttons status.
     */
    void updateButtons();

  private:

    std::unique_ptr<QgsAbstractDatabaseProviderConnection> mConnection;
    std::unique_ptr<QgsQueryResultModel> mModel;
    std::unique_ptr<QgsFeedback> mFeedback;
    std::unique_ptr<QgsConnectionsApiFetcher> mApiFetcher;
    QThread mApiFetcherWorkerThread;
    bool mWasCanceled = false;
    QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions mSqlVectorLayerOptions;
    bool mFirstRowFetched = false;
    QFutureWatcher<QgsAbstractDatabaseProviderConnection::QueryResult> mQueryResultWatcher;
    QString mSqlErrorMessage;
    long long mActualRowCount = -1;
    long long mFetchedRowsBatchCount = 0;

    /**
     * Updates SQL layer columns.
     */
    void updateSqlLayerColumns();

    /**
     * Cancel and wait for finish.
     */
    void cancelRunningQuery();

    /**
     * Cancel API fetching.
     */
    void cancelApiFetcher();

    /**
     * Starts the model population after initial query run.
     */
    void startFetching();

    /**
     * Returns the sqlVectorLayerOptions
     */
    QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions sqlVectorLayerOptions() const;

    friend class TestQgsQueryResultWidget;

};

#endif // QGSQUERYRESULTWIDGET_H
