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
#include "qgsquerybuilder.h"
#include "qgsvectorlayer.h"
#include <QStandardItem>



QgsQueryResultWidget::QgsQueryResultWidget( QWidget *parent, QgsAbstractDatabaseProviderConnection *connection )
  : QWidget::QWidget( parent )
{
  setupUi( this );

  connect( mExecuteButton, &QPushButton::pressed, this, &QgsQueryResultWidget::executeQuery );
  connect( mClearButton, &QPushButton::pressed, this, [ = ] { mSqlEditor->setText( QString() ); } );
  connect( mLoadLayerPushButton, &QPushButton::pressed, this, [ = ]
  {
    if ( mConnection )
    {
      emit createSqlVectorLayer( mConnection->providerKey(), mConnection->uri(), sqlVectorLayerOptions() );
    }
  }
         );
  connect( mSqlEditor, &QgsCodeEditorSQL::textChanged, this, [ = ] { updateButtons(); } );
  connect( mFilterToolButton, &QToolButton::pressed, this, [ = ]
  {
    if ( mConnection )
    {
      std::unique_ptr<QgsVectorLayer> vlayer { mConnection->createSqlVectorLayer( sqlVectorLayerOptions() ) };
      QgsQueryBuilder builder{ vlayer.get() };
      if ( builder.exec() == QDialog::Accepted )
      {
        mFilterLineEdit->setText( builder.sql() );
      }
    }
  } );

  // Sync all SQL layer options
  connect( mSqlEditor, &QgsCodeEditorSQL::textChanged, this, &QgsQueryResultWidget::syncSqlOptions );
  connect( mFilterLineEdit, &QLineEdit::textChanged, this, &QgsQueryResultWidget::syncSqlOptions );
  connect( mPkColumnsComboBox, &QgsCheckableComboBox::checkedItemsChanged, this, [ = ]( const QStringList & ) { syncSqlOptions(); } );
  connect( mGeometryColumnComboBox, &QComboBox::currentTextChanged, this, [ = ]( const QString & ) { syncSqlOptions(); } );

  mStatusLabel->hide();
  mSqlErrorText->hide();
  mLoadAsNewLayerGroupBox->setCollapsed( true );
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
  cancelRunningQuery();
}

void QgsQueryResultWidget::executeQuery()
{
  mQueryResultsTableView->hide();
  mSqlErrorText->hide();
  mFirstRowFetched = false;

  cancelRunningQuery();

  if ( mConnection )
  {
    const auto sql = mSqlEditor->text( );

    mWasCanceled = false;
    mFeedback = std::make_unique<QgsFeedback>();
    mStopButton->setEnabled( true );
    mStatusLabel->show();
    mStatusLabel->setText( tr( "Running⋯" ) );
    mSqlErrorMessage.clear();

    connect( mStopButton, &QPushButton::pressed, mFeedback.get(), [ = ]
    {
      mStatusLabel->setText( tr( "Stopped" ) );
      mFeedback->cancel();
      mWasCanceled = true;
    } );

    // Create model when result is ready
    connect( &mQueryResultWatcher, &QFutureWatcher<QgsAbstractDatabaseProviderConnection::QueryResult>::finished, this, &QgsQueryResultWidget::startFetching, Qt::ConnectionType::UniqueConnection );

    QFuture<QgsAbstractDatabaseProviderConnection::QueryResult> future = QtConcurrent::run( [ = ]() -> QgsAbstractDatabaseProviderConnection::QueryResult
    {
      try
      {
        return mConnection->execSql( sql, mFeedback.get() );
      }
      catch ( QgsProviderConnectionException &ex )
      {
        mSqlErrorMessage = ex.what();
        return QgsAbstractDatabaseProviderConnection::QueryResult();
      }
    } );
    mQueryResultWatcher.setFuture( future );
  }
  else
  {
    showError( tr( "Connection error" ), tr( "Cannot execute query: connection to the database not set." ) );
  }
}

void QgsQueryResultWidget::updateButtons()
{
  mFilterToolButton->setEnabled( false );
  mExecuteButton->setEnabled( ! mSqlEditor->text().isEmpty() );
  mLoadAsNewLayerGroupBox->setEnabled( mFirstRowFetched && ! mSqlEditor->text().isEmpty() && mConnection && mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::SqlLayers ) );
}

void QgsQueryResultWidget::updateSqlLayerColumns( )
{
  // Precondition
  Q_ASSERT( mModel );

  mFilterToolButton->setEnabled( true );
  mPkColumnsComboBox->clear();
  mGeometryColumnComboBox->clear();
  const bool hasPkInformation { ! mSqlVectorLayerOptions.primaryKeyColumns.isEmpty() };
  const bool hasGeomColInformation { ! mSqlVectorLayerOptions.geometryColumn.isEmpty() };
  static const QStringList geomColCandidates { QStringLiteral( "geom" ), QStringLiteral( "geometry" ),  QStringLiteral( "the_geom" ) };
  const auto constCols { mModel->columns() };
  for ( const auto &c : constCols )
  {
    const bool pkCheckedState = hasPkInformation ? mSqlVectorLayerOptions.primaryKeyColumns.contains( c ) : c.contains( QStringLiteral( "id" ), Qt::CaseSensitivity::CaseInsensitive );
    // Only check first match
    mPkColumnsComboBox->addItemWithCheckState( c, pkCheckedState && mPkColumnsComboBox->checkedItems().isEmpty() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    mGeometryColumnComboBox->addItem( c );
    if ( ! hasGeomColInformation && geomColCandidates.contains( c, Qt::CaseSensitivity::CaseInsensitive ) )
    {
      mGeometryColumnComboBox->setCurrentText( c );
    }
  }
  mPkColumnsCheckBox->setChecked( hasPkInformation );
  mGeometryColumnCheckBox->setChecked( hasGeomColInformation );
  if ( hasGeomColInformation )
  {
    mGeometryColumnComboBox->setCurrentText( mSqlVectorLayerOptions.geometryColumn );
  }
}

void QgsQueryResultWidget::cancelRunningQuery()
{
  // Cancel other threads
  if ( mFeedback )
  {
    mFeedback->cancel();
  }

  // ... and wait
  if ( mQueryResultWatcher.isRunning() )
  {
    mQueryResultWatcher.waitForFinished();
  }
}

void QgsQueryResultWidget::startFetching()
{
  if ( ! mWasCanceled )
  {
    if ( ! mSqlErrorMessage.isEmpty() )
    {
      showError( tr( "SQL error" ), mSqlErrorMessage, true );
    }
    else
    {
      QgsAbstractDatabaseProviderConnection::QueryResult result { mQueryResultWatcher.result() };
      mModel = std::make_unique<QgsQueryResultModel>( result );
      connect( mFeedback.get(), &QgsFeedback::canceled, mModel.get(), [ = ]
      {
        mModel->cancel();
        mWasCanceled = true;
      } );

      connect( mModel.get(), &QgsQueryResultModel::rowsInserted, this, [ = ]( const QModelIndex &, int, int )
      {
        if ( ! mFirstRowFetched )
        {
          emit firstResultBatchFetched();
          mFirstRowFetched = true;
          mQueryResultsTableView->show();
          updateButtons();
          updateSqlLayerColumns( );
        }
        mStatusLabel->setText( tr( "Fetched rows: %1 %2" )
                               .arg( mModel->rowCount( mModel->index( -1, -1 ) ) )
                               .arg( mWasCanceled ? tr( "(stopped)" ) : QString() ) );
      } );

      mQueryResultsTableView->setModel( mModel.get() );
      mQueryResultsTableView->show();

      connect( mModel.get(), &QgsQueryResultModel::fetchingComplete, mStopButton, [ = ]
      {
        mStopButton->setEnabled( false );
        if ( ! mWasCanceled )
        {
          mStatusLabel->setText( "Query executed successfully." );
        }
      } );
    }
  }
  else
  {
    mStatusLabel->setText( tr( "SQL command aborted" ) );
  }
}

void QgsQueryResultWidget::showError( const QString &title, const QString &message, bool isSqlError )
{
  mStatusLabel->show();
  mStatusLabel->setText( tr( "There was an error executing the query." ) );
  mQueryResultsTableView->hide();
  if ( isSqlError )
  {
    mSqlErrorText->show();
    mSqlErrorText->setText( message );
  }
  else
  {
    mMessageBar->pushCritical( title, message );
  }
}

void QgsQueryResultWidget::tokensReady( const QStringList &tokens )
{
  mSqlEditor->setFieldNames( mSqlEditor->fieldNames() + tokens );
  mSqlErrorText->setFieldNames( mSqlErrorText->fieldNames() + tokens );
}

void QgsQueryResultWidget::syncSqlOptions()
{
  mSqlVectorLayerOptions.sql = mSqlEditor->text();
  mSqlVectorLayerOptions.filter = mFilterLineEdit->text();
  mSqlVectorLayerOptions.primaryKeyColumns = mPkColumnsComboBox->checkedItems();
  mSqlVectorLayerOptions.geometryColumn = mGeometryColumnComboBox->currentText();
  mSqlVectorLayerOptions.layerName = mLayerNameLineEdit->text();
}

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsQueryResultWidget::sqlVectorLayerOptions() const
{
  QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions options { mSqlVectorLayerOptions };
  // Override if not used
  if ( ! mPkColumnsCheckBox->isChecked() )
  {
    options.primaryKeyColumns.clear();
  }
  if ( ! mGeometryColumnCheckBox->isChecked() )
  {
    options.geometryColumn.clear();
  }
  return options;
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
    mSqlEditor->setFieldNames( QStringList( ) );
    mSqlErrorText->setFieldNames( QStringList( ) );
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

  updateButtons();

}

void QgsQueryResultWidget::setQuery( const QString &sql )
{
  mSqlEditor->setText( sql );
}

void QgsQueryResultWidget::setSqlVectorLayerOptions( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options )
{
  mSqlVectorLayerOptions = options;
// TODO: check items
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

    for ( const auto &schema : std::as_const( schemas ) )
    {
      if ( mStopFetching ) { return; }
      QStringList tableNames;
      try
      {
        const auto tables = mConnection->tables( schema );
        for ( const auto &table : std::as_const( tables ) )
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
      for ( const auto &table : std::as_const( tableNames ) )
      {
        if ( mStopFetching ) { return; }
        QStringList fieldNames;
        try
        {
          const auto fields( mConnection->fields( schema, table ) );
          if ( mStopFetching ) { return; }
          for ( const auto &field : std::as_const( fields ) )
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
