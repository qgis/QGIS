/***************************************************************************
  qgsqueryresultwidget.cpp - QgsQueryResultWidget

 ---------------------
 begin                : 14.1.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
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
#include "qgsexpressionutils.h"
#include "qgscodeeditorsql.h"
#include "qgsmessagelog.h"
#include "qgsquerybuilder.h"
#include "qgsvectorlayer.h"

QgsQueryResultWidget::QgsQueryResultWidget( QWidget *parent, QgsAbstractDatabaseProviderConnection *connection )
  : QWidget( parent )
{
  setupUi( this );

  // Unsure :/
  // mSqlEditor->setLineNumbersVisible( true );

  mQueryResultsTableView->hide();
  mQueryResultsTableView->setItemDelegate( new QgsQueryResultItemDelegate( mQueryResultsTableView ) );
  mProgressBar->hide();

  connect( mExecuteButton, &QPushButton::pressed, this, &QgsQueryResultWidget::executeQuery );
  connect( mClearButton, &QPushButton::pressed, this, [ = ]
  {
    mSqlEditor->setText( QString() );
  } );
  connect( mLoadLayerPushButton, &QPushButton::pressed, this, [ = ]
  {
    if ( mConnection )
    {
      emit createSqlVectorLayer( mConnection->providerKey(), mConnection->uri(), sqlVectorLayerOptions() );
    }
  }
         );
  connect( mSqlEditor, &QgsCodeEditorSQL::textChanged, this, &QgsQueryResultWidget::updateButtons );
  connect( mFilterToolButton, &QToolButton::pressed, this, [ = ]
  {
    if ( mConnection )
    {
      try
      {
        std::unique_ptr<QgsVectorLayer> vlayer { mConnection->createSqlVectorLayer( sqlVectorLayerOptions() ) };
        QgsQueryBuilder builder{ vlayer.get() };
        if ( builder.exec() == QDialog::Accepted )
        {
          mFilterLineEdit->setText( builder.sql() );
        }
      }
      catch ( const QgsProviderConnectionException &ex )
      {
        mMessageBar->pushCritical( tr( "Error opening filter dialog" ), tr( "There was an error while preparing SQL filter dialog: %1." ).arg( ex.what() ) );
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

  connect( mLoadAsNewLayerGroupBox, &QgsCollapsibleGroupBox::collapsedStateChanged, this, [ = ]( bool collapsed )
  {
    if ( ! collapsed )
    {
      // Configure the load layer interface
      const bool showPkConfig { connection &&connection->sqlLayerDefinitionCapabilities().testFlag( Qgis::SqlLayerDefinitionCapability::PrimaryKeys )};
      mPkColumnsCheckBox->setVisible( showPkConfig );
      mPkColumnsComboBox->setVisible( showPkConfig );

      const bool showGeometryColumnConfig {connection &&connection->sqlLayerDefinitionCapabilities().testFlag( Qgis::SqlLayerDefinitionCapability::GeometryColumn )};
      mGeometryColumnCheckBox->setVisible( showGeometryColumnConfig );
      mGeometryColumnComboBox->setVisible( showGeometryColumnConfig );

      const bool showFilterConfig { connection &&connection->sqlLayerDefinitionCapabilities().testFlag( Qgis::SqlLayerDefinitionCapability::SubsetStringFilter ) };
      mFilterLabel->setVisible( showFilterConfig );
      mFilterToolButton->setVisible( showFilterConfig );
      mFilterLineEdit->setVisible( showFilterConfig );

      const bool showDisableSelectAtId{ connection &&connection->sqlLayerDefinitionCapabilities().testFlag( Qgis::SqlLayerDefinitionCapability::UnstableFeatureIds ) };
      mAvoidSelectingAsFeatureIdCheckBox->setVisible( showDisableSelectAtId );

    }
  } );

  setConnection( connection );
}

QgsQueryResultWidget::~QgsQueryResultWidget()
{
  cancelApiFetcher();
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
    const QString sql { mSqlEditor->text( ) };

    mWasCanceled = false;
    mFeedback = std::make_unique<QgsFeedback>();
    mStopButton->setEnabled( true );
    mStatusLabel->show();
    mStatusLabel->setText( tr( "Executing query…" ) );
    mProgressBar->show();
    mProgressBar->setRange( 0, 0 );
    mSqlErrorMessage.clear();

    connect( mStopButton, &QPushButton::pressed, mFeedback.get(), [ = ]
    {
      mStatusLabel->setText( tr( "Stopped" ) );
      mFeedback->cancel();
      mProgressBar->hide();
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
    showError( tr( "Connection error" ), tr( "Cannot execute query: connection to the database is not available." ) );
  }
}

void QgsQueryResultWidget::updateButtons()
{
  mFilterLineEdit->setEnabled( mFirstRowFetched );
  mFilterToolButton->setEnabled( mFirstRowFetched );
  mExecuteButton->setEnabled( ! mSqlEditor->text().isEmpty() );
  mLoadAsNewLayerGroupBox->setVisible( mConnection && mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::SqlLayers ) );
  mLoadAsNewLayerGroupBox->setEnabled(
    mSqlErrorMessage.isEmpty() &&
    mFirstRowFetched
  );
}

void QgsQueryResultWidget::updateSqlLayerColumns( )
{
  // Precondition
  Q_ASSERT( mModel );

  mFilterToolButton->setEnabled( true );
  mFilterLineEdit->setEnabled( true );
  mPkColumnsComboBox->clear();
  mGeometryColumnComboBox->clear();
  const bool hasPkInformation { ! mSqlVectorLayerOptions.primaryKeyColumns.isEmpty() };
  const bool hasGeomColInformation { ! mSqlVectorLayerOptions.geometryColumn.isEmpty() };
  static const QStringList geomColCandidates { QStringLiteral( "geom" ), QStringLiteral( "geometry" ),  QStringLiteral( "the_geom" ) };
  const QStringList constCols { mModel->columns() };
  for ( const QString &c : constCols )
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

void QgsQueryResultWidget::cancelApiFetcher()
{
  if ( mApiFetcher )
  {
    mApiFetcher->stopFetching();
    mApiFetcherWorkerThread.quit();
    mApiFetcherWorkerThread.wait();
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
      if ( mQueryResultWatcher.result().rowCount() != static_cast<long long>( Qgis::FeatureCountState::UnknownCount ) )
      {
        mStatusLabel->setText( QStringLiteral( "Query executed successfully (%1 rows, %2 ms)" )
                               .arg( QLocale().toString( mQueryResultWatcher.result().rowCount() ),
                                     QLocale().toString( mQueryResultWatcher.result().queryExecutionTime() ) ) );
      }
      else
      {
        mStatusLabel->setText( QStringLiteral( "Query executed successfully (%1 s)" ).arg( QLocale().toString( mQueryResultWatcher.result().queryExecutionTime() ) ) );
      }
      mProgressBar->hide();
      mModel = std::make_unique<QgsQueryResultModel>( mQueryResultWatcher.result() );
      connect( mFeedback.get(), &QgsFeedback::canceled, mModel.get(), [ = ]
      {
        mModel->cancel();
        mWasCanceled = true;
      } );

      connect( mModel.get(), &QgsQueryResultModel::fetchMoreRows, this, [ = ]( long long maxRows )
      {
        mFetchedRowsBatchCount = 0;
        mProgressBar->setRange( 0, maxRows );
        mProgressBar->show();
      } );

      connect( mModel.get(), &QgsQueryResultModel::rowsInserted, this, [ = ]( const QModelIndex &, int first, int last )
      {
        if ( ! mFirstRowFetched )
        {
          emit firstResultBatchFetched();
          mFirstRowFetched = true;
          mQueryResultsTableView->show();
          updateButtons();
          updateSqlLayerColumns( );
          mActualRowCount = mModel->queryResult().rowCount();
        }
        mStatusLabel->setText( tr( "Fetched rows: %1/%2 %3 %4 ms" )
                               .arg( QLocale().toString( mModel->rowCount( mModel->index( -1, -1 ) ) ),
                                     mActualRowCount != -1 ? QLocale().toString( mActualRowCount ) : tr( "unknown" ),
                                     mWasCanceled ? tr( "(stopped)" ) : QString(),
                                     QLocale().toString( mQueryResultWatcher.result().queryExecutionTime() ) ) );
        mFetchedRowsBatchCount += last - first + 1;
        mProgressBar->setValue( mFetchedRowsBatchCount );
      } );

      mQueryResultsTableView->setModel( mModel.get() );
      mQueryResultsTableView->show();

      connect( mModel.get(), &QgsQueryResultModel::fetchingComplete, mStopButton, [ = ]
      {
        mProgressBar->hide();
        mStopButton->setEnabled( false );
      } );
    }
  }
  else
  {
    mStatusLabel->setText( tr( "SQL command aborted" ) );
    mProgressBar->hide();
  }
}

void QgsQueryResultWidget::showError( const QString &title, const QString &message, bool isSqlError )
{
  mStatusLabel->show();
  mStatusLabel->setText( tr( "An error occurred while executing the query" ) );
  mProgressBar->hide();
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
  mSqlEditor->setExtraKeywords( mSqlEditor->extraKeywords() + tokens );
  mSqlErrorText->setExtraKeywords( mSqlErrorText->extraKeywords() + tokens );
}

void QgsQueryResultWidget::syncSqlOptions()
{
  mSqlVectorLayerOptions.sql = mSqlEditor->text();
  mSqlVectorLayerOptions.filter = mFilterLineEdit->text();
  mSqlVectorLayerOptions.primaryKeyColumns = mPkColumnsComboBox->checkedItems();
  mSqlVectorLayerOptions.geometryColumn = mGeometryColumnComboBox->currentText();
  mSqlVectorLayerOptions.layerName = mLayerNameLineEdit->text();
  mSqlVectorLayerOptions.disableSelectAtId = mAvoidSelectingAsFeatureIdCheckBox->isChecked();
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

  cancelApiFetcher();

  if ( connection )
  {

    // Add provider specific APIs
    const QMap<Qgis::SqlKeywordCategory, QStringList> keywordsDict { connection->sqlDictionary() };
    QStringList keywords;
    for ( auto it = keywordsDict.constBegin(); it != keywordsDict.constEnd(); it++ )
    {
      keywords.append( it.value() );
    }

    // Add static keywords from provider
    mSqlEditor->setExtraKeywords( keywords );
    mSqlErrorText->setExtraKeywords( keywords );

    // Add dynamic keywords in a separate thread
    mApiFetcher = std::make_unique<QgsConnectionsApiFetcher>( connection );
    mApiFetcher->moveToThread( &mApiFetcherWorkerThread );
    connect( &mApiFetcherWorkerThread, &QThread::started, mApiFetcher.get(), &QgsConnectionsApiFetcher::fetchTokens );
    connect( mApiFetcher.get(), &QgsConnectionsApiFetcher::tokensReady, this, &QgsQueryResultWidget::tokensReady );
    connect( mApiFetcher.get(), &QgsConnectionsApiFetcher::fetchingFinished, &mApiFetcherWorkerThread, [ = ]
    {
      mApiFetcherWorkerThread.quit();
      mApiFetcherWorkerThread.wait();
    } );
    mApiFetcherWorkerThread.start();
  }

  updateButtons();

}

void QgsQueryResultWidget::setQuery( const QString &sql )
{
  mSqlEditor->setText( sql );
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
        QgsMessageLog::logMessage( tr( "Error retrieving schemas: %1" ).arg( ex.what() ), QStringLiteral( "QGIS" ), Qgis::MessageLevel::Warning );
      }
    }
    else
    {
      schemas.push_back( QString() );  // Fake empty schema for DBs not supporting it
    }

    for ( const auto &schema : std::as_const( schemas ) )
    {

      if ( mStopFetching )
      {
        return;
      }

      QStringList tableNames;
      try
      {
        const QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables = mConnection->tables( schema );
        for ( const QgsAbstractDatabaseProviderConnection::TableProperty &table : std::as_const( tables ) )
        {
          if ( mStopFetching ) { return; }
          tableNames.push_back( table.tableName() );
        }
        emit tokensReady( tableNames );
      }
      catch ( QgsProviderConnectionException &ex )
      {
        QgsMessageLog::logMessage( tr( "Error retrieving tables: %1" ).arg( ex.what() ), QStringLiteral( "QGIS" ), Qgis::MessageLevel::Warning );
      }

      // Get fields
      for ( const auto &table : std::as_const( tableNames ) )
      {

        if ( mStopFetching )
        {
          return;
        }

        QStringList fieldNames;
        try
        {
          const QgsFields fields( mConnection->fields( schema, table ) );
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
          QgsMessageLog::logMessage( tr( "Error retrieving fields for table %1: %2" ).arg( table, ex.what() ), QStringLiteral( "QGIS" ), Qgis::MessageLevel::Warning );
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


QgsQueryResultItemDelegate::QgsQueryResultItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

QString QgsQueryResultItemDelegate::displayText( const QVariant &value, const QLocale &locale ) const
{
  Q_UNUSED( locale )
  QString result { QgsExpressionUtils::toLocalizedString( value ) };
  // Show no more than 255 characters
  if ( result.length() > 255 )
  {
    result.truncate( 255 );
    result.append( QStringLiteral( "…" ) );
  }
  return result;
}

///@endcond private
