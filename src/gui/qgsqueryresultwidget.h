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


///@cond private

#ifndef SIP_RUN

/**
 * The QgsConnectionsApiFetcher class fetches tokens (table and field names) of a connection from a separate thread.
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
 * The QgsQueryResultWidget class allow users to enter and run an SQL query on a
 * DB connection (an instance of QgsAbstractDatabaseProviderConnection).
 *
 * Query results are displayed in a table view.
 * Query execution and result
 * fetching can be interrupted by pressing the "Stop" push button.
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
    void setConnection( QgsAbstractDatabaseProviderConnection *connection );

    /**
     * Convenience method to set the SQL editor test to \a sql.
     */
    void setQuery( const QString &sql );

  public slots:

    /**
     * Executes the query
     */
    void executeQuery();

    /**
     * Updates buttons status
     */
    void updateButtons();

    /**
     * Hides the result table and shows the error \a title and \a message in the message bar.
     */
    void showError( const QString &title, const QString &message );

    /**
     * Triggered when the threaded API fetcher has new \a tokens to add.
     */
    void tokensReady( const QStringList &tokens );

  private:

    std::unique_ptr<QgsAbstractDatabaseProviderConnection> mConnection;
    std::unique_ptr<QgsQueryResultModel> mModel;
    std::unique_ptr<QgsFeedback> mFeedback;
    QgsConnectionsApiFetcher *mApiFetcher = nullptr;
    QThread mWorkerThread;
    bool mWasCanceled = false;

    friend class TestQgsQueryResultWidget;

};

#endif // QGSQUERYRESULTWIDGET_H
