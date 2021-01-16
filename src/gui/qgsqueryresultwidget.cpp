/***************************************************************************
  qgsqueryresultwidget.cpp - QgsQueryResultWidget

 ---------------------
 begin                : 14.1.2021
 copyright            : (C) 2021 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsqueryresultwidget.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgscodeeditorsql.h"
#include "qgsmessagelog.h"
#include <QPushButton>

QgsQueryResultWidget::QgsQueryResultWidget( QWidget *parent, QgsAbstractDatabaseProviderConnection *connection )
  : QWidget::QWidget( parent )
{
  setupUi( this );

  connect( mExecuteButton, &QPushButton::pressed, this, &QgsQueryResultWidget::executeQuery );
  connect( mCodeEditor, &QgsCodeEditorSQL::textChanged, this, [ = ] { updateButtons(); } );

  mStatusLabel->hide();
  setConnection( connection );
}

QgsQueryResultWidget::~QgsQueryResultWidget()
{
  if ( mApiFetcher )
  {
    mApiFetcher->stopFetching();
    mWorkerThread.quit();
    mWorkerThread.wait();
  }
}

void QgsQueryResultWidget::executeQuery()
{
  if ( mConnection )
  {
    const auto sql = mCodeEditor->text( );
    try
    {
      mWasCanceled = false;
      mFeedback = qgis::make_unique<QgsFeedback>();
      connect( mStopButton, &QPushButton::pressed, mFeedback.get(), &QgsFeedback::cancel );
      mModel = qgis::make_unique<QgsQueryResultModel>( mConnection->execSql( sql, mFeedback.get() ) );
      connect( mFeedback.get(), &QgsFeedback::canceled, mModel.get(), [ = ]
      {
        mModel->cancel();
        mWasCanceled = true;
      } );
      mStatusLabel->show();
      mQueryResultsTableView->show();
      mStatusLabel->setText( tr( "Running⋯" ) );
      connect( mModel.get(), &QgsQueryResultModel::rowsInserted, this, [ = ]( const QModelIndex &, int, int )
      {
        mStatusLabel->setText( tr( "Fetched rows: %1 %2" )
                               .arg( mModel->rowCount( mModel->index( -1, -1 ) ) )
                               .arg( mWasCanceled ? QStringLiteral( "(stopped)" ) : QString() ) );
      } );
      mQueryResultsTableView->setModel( mModel.get() );
      mStopButton->setEnabled( true );
      connect( mModel.get(), &QgsQueryResultModel::fetchingComplete, mStopButton, [ = ]
      {
        mStopButton->setEnabled( false );
        if ( ! mWasCanceled )
        {
          mStatusLabel->setText( "Query executed successfully." );
        }
      } );
    }
    catch ( QgsProviderConnectionException &ex )
    {
      showError( tr( "SQL error" ), ex.what() );
    }
  }
  else
  {
    showError( tr( "Connection error" ), tr( "Cannot execute query: connection to the database not set." ) );
  }
}

void QgsQueryResultWidget::updateButtons()
{
  mExecuteButton->setEnabled( ! mCodeEditor->text().isEmpty() );
}

void QgsQueryResultWidget::showError( const QString &title, const QString &message )
{
  mStatusLabel->show();
  mStatusLabel->setText( tr( "There was an error executing the query." ) );
  mMessageBar->pushCritical( title, message );
}

void QgsQueryResultWidget::tokensReady( const QStringList &tokens )
{
  mCodeEditor->setFieldNames( mCodeEditor->fieldNames() + tokens );
}

void QgsQueryResultWidget::setConnection( QgsAbstractDatabaseProviderConnection *connection )
{
  mConnection.reset( connection );

  if ( mApiFetcher )
  {
    mApiFetcher->stopFetching();
    mWorkerThread.quit();
    mWorkerThread.wait();
    mApiFetcher->deleteLater();
    mApiFetcher = nullptr;
  }

  if ( connection )
  {
    mCodeEditor->setFieldNames( QStringList( ) );
    mApiFetcher = new QgsConnectionsApiFetcher( connection );
    mApiFetcher->moveToThread( &mWorkerThread );
    connect( &mWorkerThread, &QThread::started, mApiFetcher, &QgsConnectionsApiFetcher::fetchTokens );
    connect( &mWorkerThread, &QThread::finished, mApiFetcher, [ = ]
    {
      mApiFetcher->deleteLater();
      mApiFetcher = nullptr;
    } );
    connect( mApiFetcher, &QgsConnectionsApiFetcher::tokensReady, this, &QgsQueryResultWidget::tokensReady );
    mWorkerThread.start();
  }
}

void QgsQueryResultWidget::setQuery( const QString &sql )
{
  mCodeEditor->setText( sql );
}

///@cond private

void QgsConnectionsApiFetcher::fetchTokens()
{
  if ( ! mStopFetching && mConnection )
  {
    QStringList schemas;
    if ( mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Schemas ) )
    {
      try
      {
        schemas = mConnection->schemas();
        emit tokensReady( schemas );
      }
      catch ( QgsProviderConnectionException &ex )
      {
        QgsMessageLog::logMessage( tr( "Error retrieving schemas: %1" ).arg( ex.what() ) );
      }
    }
    else
    {
      schemas.push_back( QString() );  // Fake empty schema for DBs not supporting it
    }

    for ( const auto &schema : qgis::as_const( schemas ) )
    {
      if ( mStopFetching ) { return; }
      QStringList tableNames;
      try
      {
        const auto tables = mConnection->tables( schema );
        for ( const auto &table : qgis::as_const( tables ) )
        {
          if ( mStopFetching ) { return; }
          tableNames.push_back( table.tableName() );
        }
        emit tokensReady( tableNames );
      }
      catch ( QgsProviderConnectionException &ex )
      {
        QgsMessageLog::logMessage( tr( "Error retrieving tables: %1" ).arg( ex.what() ) );
      }

      // Get fields
      for ( const auto &table : qgis::as_const( tableNames ) )
      {
        if ( mStopFetching ) { return; }
        QStringList fieldNames;
        try
        {
          const auto fields( mConnection->fields( schema, table ) );
          if ( mStopFetching ) { return; }
          for ( const auto &field : qgis::as_const( fields ) )
          {
            fieldNames.push_back( field.name() );
            if ( mStopFetching ) { return; }
          }
          emit tokensReady( fieldNames );
        }
        catch ( QgsProviderConnectionException &ex )
        {
          QgsMessageLog::logMessage( tr( "Error retrieving fields for table %1: %2" ).arg( table, ex.what() ) );
        }
      }
    }
  }
  emit fetchingFinished();
}

void QgsConnectionsApiFetcher::stopFetching()
{
  mStopFetching = 1;
}

///@endcond private
