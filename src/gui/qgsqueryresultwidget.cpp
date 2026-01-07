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
#include "qgsapplication.h"
#include "qgscodeeditorsql.h"
#include "qgscodeeditorwidget.h"
#include "qgsdbqueryhistoryprovider.h"
#include "qgsexpressionutils.h"
#include "qgsfileutils.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgshistoryentry.h"
#include "qgshistoryproviderregistry.h"
#include "qgshistorywidget.h"
#include "qgsmessagelog.h"
#include "qgsnewnamedialog.h"
#include "qgsproject.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsquerybuilder.h"
#include "qgsstoredquerymanager.h"
#include "qgsvectorlayer.h"

#include <QClipboard>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>

#include "moc_qgsqueryresultwidget.cpp"

///@cond PRIVATE
const QgsSettingsEntryString *QgsQueryResultWidget::settingLastSourceFolder = new QgsSettingsEntryString( u"last-source-folder"_s, sTreeSqlQueries, QString(), u"Last used folder for SQL source files"_s );
///@endcond PRIVATE
///


//
// QgsQueryResultPanelWidget
//

QgsQueryResultPanelWidget::QgsQueryResultPanelWidget( QWidget *parent, QgsAbstractDatabaseProviderConnection *connection )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  // Unsure :/
  // mSqlEditor->setLineNumbersVisible( true );

  splitter->setCollapsible( 0, false );
  splitter->setCollapsible( 1, false );
  QgsSettings settings;
  splitter->restoreState( settings.value( u"Windows/QueryResult/SplitState"_s ).toByteArray() );

  connect( splitter, &QSplitter::splitterMoved, this, [this] {
    QgsSettings settings;
    settings.setValue( u"Windows/QueryResult/SplitState"_s, splitter->saveState() );
  } );

  // explicitly needed for some reason (Qt 5.15)
  mainLayout->setSpacing( 6 );
  progressLayout->setSpacing( 6 );

  mResultsContainer->hide();
  mQueryResultsTableView->hide();
  mQueryResultsTableView->setItemDelegate( new QgsQueryResultItemDelegate( mQueryResultsTableView ) );
  mQueryResultsTableView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mQueryResultsTableView, &QTableView::customContextMenuRequested, this, &QgsQueryResultPanelWidget::showCellContextMenu );

  mProgressBar->hide();

  mSqlEditor = new QgsCodeEditorSQL();
  mCodeEditorWidget = new QgsCodeEditorWidget( mSqlEditor, mMessageBar );
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->addWidget( mCodeEditorWidget );
  mSqlEditorContainer->setLayout( vl );

  connect( mExecuteButton, &QPushButton::pressed, this, &QgsQueryResultPanelWidget::executeQuery );

  connect( mLoadLayerPushButton, &QPushButton::pressed, this, [this] {
    if ( mConnection )
    {
      const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions options = sqlVectorLayerOptions();

      try
      {
        QString message;
        const bool res = mConnection->validateSqlVectorLayer( options, message );
        if ( !res )
        {
          mMessageBar->pushCritical( QString(), message );
        }
        else
        {
          emit createSqlVectorLayer( mConnection->providerKey(), mConnection->uri(), options );
        }
      }
      catch ( QgsProviderConnectionException &e )
      {
        mMessageBar->pushCritical( tr( "Error validating query" ), e.what() );
      }
    }
  } );
  connect( mSqlEditor, &QgsCodeEditorSQL::textChanged, this, &QgsQueryResultPanelWidget::updateButtons );

  connect( mSqlEditor, &QgsCodeEditorSQL::selectionChanged, this, [this] {
    mExecuteButton->setText( mSqlEditor->selectedText().isEmpty() ? tr( "Execute" ) : tr( "Execute Selection" ) );
  } );
  connect( mFilterToolButton, &QToolButton::pressed, this, [this] {
    if ( mConnection )
    {
      try
      {
        std::unique_ptr<QgsVectorLayer> vlayer { mConnection->createSqlVectorLayer( sqlVectorLayerOptions() ) };
        QgsQueryBuilder builder { vlayer.get() };
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


  mStatusLabel->hide();
  mSqlErrorText->hide();

  mLoadAsNewLayerGroupBox->setCollapsed( true );

  connect( mLoadAsNewLayerGroupBox, &QgsCollapsibleGroupBox::collapsedStateChanged, this, [this, connection]( bool collapsed ) {
    if ( !collapsed )
    {
      // Configure the load layer interface
      const bool showPkConfig { connection && connection->sqlLayerDefinitionCapabilities().testFlag( Qgis::SqlLayerDefinitionCapability::PrimaryKeys ) };
      mPkColumnsCheckBox->setVisible( showPkConfig );
      mPkColumnsComboBox->setVisible( showPkConfig );

      const bool showGeometryColumnConfig { connection && connection->sqlLayerDefinitionCapabilities().testFlag( Qgis::SqlLayerDefinitionCapability::GeometryColumn ) };
      mGeometryColumnCheckBox->setVisible( showGeometryColumnConfig );
      mGeometryColumnComboBox->setVisible( showGeometryColumnConfig );

      const bool showFilterConfig { connection && connection->sqlLayerDefinitionCapabilities().testFlag( Qgis::SqlLayerDefinitionCapability::SubsetStringFilter ) };
      mFilterLabel->setVisible( showFilterConfig );
      mFilterToolButton->setVisible( showFilterConfig );
      mFilterLineEdit->setVisible( showFilterConfig );

      const bool showDisableSelectAtId { connection && connection->sqlLayerDefinitionCapabilities().testFlag( Qgis::SqlLayerDefinitionCapability::UnstableFeatureIds ) };
      mAvoidSelectingAsFeatureIdCheckBox->setVisible( showDisableSelectAtId );
    }
  } );

  QShortcut *copySelection = new QShortcut( QKeySequence::Copy, mQueryResultsTableView );
  connect( copySelection, &QShortcut::activated, this, &QgsQueryResultPanelWidget::copySelection );

  setConnection( connection );
}

QgsQueryResultPanelWidget::~QgsQueryResultPanelWidget()
{
  cancelApiFetcher();
  cancelRunningQuery();
}

QgsCodeEditorSQL *QgsQueryResultPanelWidget::sqlEditor()
{
  return mSqlEditor;
}

QgsCodeEditorWidget *QgsQueryResultPanelWidget::codeEditorWidget()
{
  return mCodeEditorWidget;
}

void QgsQueryResultPanelWidget::setSqlVectorLayerOptions( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options )
{
  mSqlVectorLayerOptions = options;
  if ( !options.sql.isEmpty() )
  {
    setQuery( options.sql );
  }
  mAvoidSelectingAsFeatureIdCheckBox->setChecked( options.disableSelectAtId );
  mPkColumnsCheckBox->setChecked( !options.primaryKeyColumns.isEmpty() );
  mPkColumnsComboBox->setCheckedItems( {} );
  if ( !options.primaryKeyColumns.isEmpty() )
  {
    mPkColumnsComboBox->setCheckedItems( options.primaryKeyColumns );
  }
  mGeometryColumnCheckBox->setChecked( !options.geometryColumn.isEmpty() );
  mGeometryColumnComboBox->clear();
  if ( !options.geometryColumn.isEmpty() )
  {
    mGeometryColumnComboBox->setCurrentText( options.geometryColumn );
  }
  mFilterLineEdit->setText( options.filter );
  mLayerNameLineEdit->setText( options.layerName );
}

void QgsQueryResultPanelWidget::setWidgetMode( QgsQueryResultWidget::QueryWidgetMode widgetMode )
{
  mQueryWidgetMode = widgetMode;
  switch ( widgetMode )
  {
    case QgsQueryResultWidget::QueryWidgetMode::SqlQueryMode:
      mLoadAsNewLayerGroupBox->setTitle( tr( "Load as New Layer" ) );
      mLoadLayerPushButton->setText( tr( "Load Layer" ) );
      mLoadAsNewLayerGroupBox->setCollapsed( true );
      break;
    case QgsQueryResultWidget::QueryWidgetMode::QueryLayerUpdateMode:
      mLoadAsNewLayerGroupBox->setTitle( tr( "Update Query Layer" ) );
      mLoadLayerPushButton->setText( tr( "Update Layer" ) );
      mLoadAsNewLayerGroupBox->setCollapsed( false );
      break;
  }
}

void QgsQueryResultPanelWidget::executeQuery()
{
  mQueryResultsTableView->hide();
  mSqlErrorText->hide();
  mResultsContainer->hide();
  mFirstRowFetched = false;

  cancelRunningQuery();
  if ( mConnection )
  {
    const QString sql { mSqlEditor->selectedText().isEmpty() ? mSqlEditor->text() : mSqlEditor->selectedText() };

    bool ok = false;
    mCurrentHistoryEntryId = QgsGui::historyProviderRegistry()->addEntry( u"dbquery"_s, QVariantMap {
                                                                                          { u"query"_s, sql },
                                                                                          { u"provider"_s, mConnection->providerKey() },
                                                                                          { u"connection"_s, mConnection->uri() },
                                                                                        },
                                                                          ok );

    mWasCanceled = false;
    mFeedback = std::make_unique<QgsFeedback>();
    mStopButton->setEnabled( true );
    mStatusLabel->show();
    mStatusLabel->setText( tr( "Executing query…" ) );
    mProgressBar->show();
    mProgressBar->setRange( 0, 0 );
    mSqlErrorMessage.clear();

    connect( mStopButton, &QPushButton::pressed, mFeedback.get(), [this] {
      mStatusLabel->setText( tr( "Stopped" ) );
      mFeedback->cancel();
      mProgressBar->hide();
      mWasCanceled = true;
    } );

    // Create model when result is ready
    connect( &mQueryResultWatcher, &QFutureWatcher<QgsAbstractDatabaseProviderConnection::QueryResult>::finished, this, &QgsQueryResultPanelWidget::startFetching, Qt::ConnectionType::UniqueConnection );

    QFuture<QgsAbstractDatabaseProviderConnection::QueryResult> future = QtConcurrent::run( [this, sql]() -> QgsAbstractDatabaseProviderConnection::QueryResult {
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

void QgsQueryResultPanelWidget::updateButtons()
{
  mFilterLineEdit->setEnabled( mFirstRowFetched );
  mFilterToolButton->setEnabled( mFirstRowFetched );
  const bool isEmpty = mSqlEditor->text().isEmpty();
  mExecuteButton->setEnabled( !isEmpty );
  mLoadAsNewLayerGroupBox->setVisible( mConnection && mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::SqlLayers ) );
  mLoadAsNewLayerGroupBox->setEnabled(
    mSqlErrorMessage.isEmpty() && mFirstRowFetched
  );
}

void QgsQueryResultPanelWidget::showCellContextMenu( QPoint point )
{
  const QModelIndex modelIndex = mQueryResultsTableView->indexAt( point );
  if ( modelIndex.isValid() )
  {
    QMenu *menu = new QMenu();
    menu->setAttribute( Qt::WA_DeleteOnClose );

    menu->addAction( QgsApplication::getThemeIcon( "mActionEditCopy.svg" ), tr( "Copy" ), this, [this] { copySelection(); }, QKeySequence::Copy );

    menu->exec( mQueryResultsTableView->viewport()->mapToGlobal( point ) );
  }
}

void QgsQueryResultPanelWidget::copySelection()
{
  const QModelIndexList selection = mQueryResultsTableView->selectionModel()->selectedIndexes();
  if ( selection.empty() )
    return;

  int minRow = -1;
  int maxRow = -1;
  int minCol = -1;
  int maxCol = -1;
  for ( const QModelIndex &index : selection )
  {
    if ( minRow == -1 || index.row() < minRow )
      minRow = index.row();
    if ( maxRow == -1 || index.row() > maxRow )
      maxRow = index.row();
    if ( minCol == -1 || index.column() < minCol )
      minCol = index.column();
    if ( maxCol == -1 || index.column() > maxCol )
      maxCol = index.column();
  }

  if ( minRow == maxRow && minCol == maxCol )
  {
    // copy only one cell
    const QString text = mModel->data( selection.at( 0 ), Qt::DisplayRole ).toString();
    QApplication::clipboard()->setText( text );
  }
  else
  {
    copyResults( minRow, maxRow, minCol, maxCol );
  }
}

void QgsQueryResultPanelWidget::updateSqlLayerColumns()
{
  // Precondition
  Q_ASSERT( mModel );

  mFilterToolButton->setEnabled( true );
  mFilterLineEdit->setEnabled( true );
  mPkColumnsComboBox->clear();
  mGeometryColumnComboBox->clear();
  const bool hasPkInformation { !mSqlVectorLayerOptions.primaryKeyColumns.isEmpty() };
  const bool hasGeomColInformation { !mSqlVectorLayerOptions.geometryColumn.isEmpty() };
  static const QStringList geomColCandidates { u"geom"_s, u"geometry"_s, u"the_geom"_s };
  const QStringList constCols { mModel->columns() };
  for ( const QString &c : constCols )
  {
    const bool pkCheckedState = hasPkInformation ? mSqlVectorLayerOptions.primaryKeyColumns.contains( c ) : c.contains( u"id"_s, Qt::CaseSensitivity::CaseInsensitive );
    // Only check first match
    mPkColumnsComboBox->addItemWithCheckState( c, pkCheckedState && mPkColumnsComboBox->checkedItems().isEmpty() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    mGeometryColumnComboBox->addItem( c );
    if ( !hasGeomColInformation && geomColCandidates.contains( c, Qt::CaseSensitivity::CaseInsensitive ) )
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

void QgsQueryResultPanelWidget::cancelRunningQuery()
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

void QgsQueryResultPanelWidget::cancelApiFetcher()
{
  if ( mApiFetcher )
  {
    mApiFetcher->stopFetching();
    // apiFetcher and apiFetcherWorkerThread will be deleted when the thread fetchingFinished signal is emitted
  }
}

void QgsQueryResultPanelWidget::startFetching()
{
  if ( !mWasCanceled )
  {
    if ( !mSqlErrorMessage.isEmpty() )
    {
      showError( tr( "SQL error" ), mSqlErrorMessage, true );
    }
    else
    {
      if ( mQueryResultWatcher.result().rowCount() != static_cast<long long>( Qgis::FeatureCountState::UnknownCount ) )
      {
        mStatusLabel->setText( u"Query executed successfully (%1 rows, %2 ms)"_s
                                 .arg( QLocale().toString( mQueryResultWatcher.result().rowCount() ), QLocale().toString( mQueryResultWatcher.result().queryExecutionTime() ) ) );
      }
      else
      {
        mStatusLabel->setText( u"Query executed successfully (%1 s)"_s.arg( QLocale().toString( mQueryResultWatcher.result().queryExecutionTime() ) ) );
      }
      mProgressBar->hide();
      mModel = std::make_unique<QgsQueryResultModel>( mQueryResultWatcher.result() );
      connect( mFeedback.get(), &QgsFeedback::canceled, mModel.get(), [this] {
        mModel->cancel();
        mWasCanceled = true;
      } );

      connect( mModel.get(), &QgsQueryResultModel::fetchMoreRows, this, [this]( long long maxRows ) {
        mFetchedRowsBatchCount = 0;
        mProgressBar->setRange( 0, maxRows );
        mProgressBar->show();
      } );

      connect( mModel.get(), &QgsQueryResultModel::rowsInserted, this, [this]( const QModelIndex &, int first, int last ) {
        if ( !mFirstRowFetched )
        {
          emit firstResultBatchFetched();
          mFirstRowFetched = true;
          mQueryResultsTableView->show();
          mResultsContainer->show();
          updateButtons();
          updateSqlLayerColumns();
          mActualRowCount = mModel->queryResult().rowCount();
        }
        mStatusLabel->setText( tr( "Fetched rows: %1/%2 %3 %4 ms" )
                                 .arg( QLocale().toString( mModel->rowCount( mModel->index( -1, -1 ) ) ), mActualRowCount != -1 ? QLocale().toString( mActualRowCount ) : tr( "unknown" ), mWasCanceled ? tr( "(stopped)" ) : QString(), QLocale().toString( mQueryResultWatcher.result().queryExecutionTime() ) ) );
        mFetchedRowsBatchCount += last - first + 1;
        mProgressBar->setValue( mFetchedRowsBatchCount );
      } );

      mQueryResultsTableView->setModel( mModel.get() );
      mQueryResultsTableView->show();
      mResultsContainer->show();

      connect( mModel.get(), &QgsQueryResultModel::fetchingComplete, mStopButton, [this] {
        bool ok = false;
        const QgsHistoryEntry currentHistoryEntry = QgsGui::historyProviderRegistry()->entry( mCurrentHistoryEntryId, ok );
        QVariantMap entryDetails = currentHistoryEntry.entry;
        entryDetails.insert( u"rows"_s, mActualRowCount );
        entryDetails.insert( u"time"_s, mQueryResultWatcher.result().queryExecutionTime() );

        QgsGui::historyProviderRegistry()->updateEntry( mCurrentHistoryEntryId, entryDetails );
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

void QgsQueryResultPanelWidget::showError( const QString &title, const QString &message, bool isSqlError )
{
  mStatusLabel->show();
  mStatusLabel->setText( tr( "An error occurred while executing the query" ) );
  mProgressBar->hide();
  mQueryResultsTableView->hide();
  if ( isSqlError )
  {
    mSqlErrorText->show();
    mSqlErrorText->setText( message );
    mResultsContainer->show();
  }
  else
  {
    mMessageBar->pushCritical( title, message );
    mResultsContainer->hide();
  }
}

void QgsQueryResultPanelWidget::tokensReady( const QStringList &tokens )
{
  mSqlEditor->setExtraKeywords( mSqlEditor->extraKeywords() + tokens );
  mSqlErrorText->setExtraKeywords( mSqlErrorText->extraKeywords() + tokens );
}

void QgsQueryResultPanelWidget::copyResults()
{
  const int rowCount = mModel->rowCount( QModelIndex() );
  const int columnCount = mModel->columnCount( QModelIndex() );
  copyResults( 0, rowCount - 1, 0, columnCount - 1 );
}

void QgsQueryResultPanelWidget::copyResults( int fromRow, int toRow, int fromColumn, int toColumn )
{
  QStringList rowStrings;
  QStringList columnStrings;

  const int rowCount = mModel->rowCount( QModelIndex() );
  const int columnCount = mModel->columnCount( QModelIndex() );

  toRow = std::min( toRow, rowCount - 1 );
  toColumn = std::min( toColumn, columnCount - 1 );

  rowStrings.reserve( toRow - fromRow );

  // add titles first
  for ( int col = fromColumn; col <= toColumn; col++ )
  {
    columnStrings += mModel->headerData( col, Qt::Horizontal, Qt::DisplayRole ).toString();
  }
  rowStrings += columnStrings.join( QLatin1Char( '\t' ) );
  columnStrings.clear();

  for ( int row = fromRow; row <= toRow; row++ )
  {
    for ( int col = fromColumn; col <= toColumn; col++ )
    {
      columnStrings += mModel->data( mModel->index( row, col ), Qt::DisplayRole ).toString();
    }
    rowStrings += columnStrings.join( QLatin1Char( '\t' ) );
    columnStrings.clear();
  }

  if ( !rowStrings.isEmpty() )
  {
    const QString text = rowStrings.join( QLatin1Char( '\n' ) );
    QString html = u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>%1</td></tr></table></body></html>"_s.arg( text );
    html.replace( "\t"_L1, "</td><td>"_L1 ).replace( "\n"_L1, "</td></tr><tr><td>"_L1 );

    QMimeData *mdata = new QMimeData();
    mdata->setData( u"text/html"_s, html.toUtf8() );
    if ( !text.isEmpty() )
    {
      mdata->setText( text );
    }
    // Transfers ownership to the clipboard object
#ifdef Q_OS_LINUX
    QApplication::clipboard()->setMimeData( mdata, QClipboard::Selection );
#endif
    QApplication::clipboard()->setMimeData( mdata, QClipboard::Clipboard );
  }
}

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsQueryResultPanelWidget::sqlVectorLayerOptions() const
{
  const thread_local QRegularExpression rx( u";\\s*$"_s );
  mSqlVectorLayerOptions.sql = mSqlEditor->text().replace( rx, QString() );
  mSqlVectorLayerOptions.filter = mFilterLineEdit->text();
  mSqlVectorLayerOptions.primaryKeyColumns = mPkColumnsComboBox->checkedItems();
  mSqlVectorLayerOptions.geometryColumn = mGeometryColumnComboBox->currentText();
  mSqlVectorLayerOptions.layerName = mLayerNameLineEdit->text();
  mSqlVectorLayerOptions.disableSelectAtId = mAvoidSelectingAsFeatureIdCheckBox->isChecked();
  QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions options { mSqlVectorLayerOptions };
  // Override if not used
  if ( !mPkColumnsCheckBox->isChecked() )
  {
    options.primaryKeyColumns.clear();
  }
  if ( !mGeometryColumnCheckBox->isChecked() )
  {
    options.geometryColumn.clear();
  }
  return options;
}

void QgsQueryResultPanelWidget::setConnection( QgsAbstractDatabaseProviderConnection *connection )
{
  mConnection.reset( connection );

  cancelApiFetcher();

  if ( connection )
  {
    // Add provider specific APIs
    const QMultiMap<Qgis::SqlKeywordCategory, QStringList> keywordsDict { connection->sqlDictionary() };
    QStringList keywords;
    for ( auto it = keywordsDict.constBegin(); it != keywordsDict.constEnd(); it++ )
    {
      keywords.append( it.value() );
    }

    // Add static keywords from provider
    mSqlEditor->setExtraKeywords( keywords );
    mSqlErrorText->setExtraKeywords( keywords );

    // Add dynamic keywords in a separate thread
    QThread *apiFetcherWorkerThread = new QThread();
    QgsConnectionsApiFetcher *apiFetcher = new QgsConnectionsApiFetcher( mConnection->uri(), mConnection->providerKey() );
    apiFetcher->moveToThread( apiFetcherWorkerThread );
    connect( apiFetcherWorkerThread, &QThread::started, apiFetcher, &QgsConnectionsApiFetcher::fetchTokens );
    connect( apiFetcher, &QgsConnectionsApiFetcher::tokensReady, this, &QgsQueryResultPanelWidget::tokensReady );
    connect( apiFetcher, &QgsConnectionsApiFetcher::fetchingFinished, apiFetcherWorkerThread, [apiFetcher, apiFetcherWorkerThread] {
      apiFetcherWorkerThread->quit();
      apiFetcherWorkerThread->wait();
      apiFetcherWorkerThread->deleteLater();
      apiFetcher->deleteLater();
    } );

    mApiFetcher = apiFetcher;
    apiFetcherWorkerThread->start();
  }

  updateButtons();
}

void QgsQueryResultPanelWidget::setQuery( const QString &sql )
{
  mSqlEditor->setText( sql );
}

void QgsQueryResultPanelWidget::notify( const QString &title, const QString &text, Qgis::MessageLevel level )
{
  mMessageBar->pushMessage( title, text, level );
}

void QgsQueryResultPanelWidget::storeCurrentQuery( Qgis::QueryStorageBackend backend )
{
  const QStringList existingQueryNames = QgsGui::storedQueryManager()->allQueryNames( backend );
  QgsNewNameDialog dlg(
    QString(),
    QString(),
    QStringList(),
    existingQueryNames
  );
  dlg.setWindowTitle( tr( "Store Query" ) );
  dlg.setHintString( tr( "Name for the stored query" ) );
  dlg.setOverwriteEnabled( true );
  dlg.setConflictingNameWarning( tr( "A stored query with this name already exists, it will be overwritten." ) );
  dlg.setShowExistingNamesCompleter( true );
  if ( dlg.exec() != QDialog::Accepted )
    return;

  const QString name = dlg.name();
  if ( name.isEmpty() )
    return;

  QgsGui::storedQueryManager()->storeQuery( name, mSqlEditor->text(), backend );
  if ( backend == Qgis::QueryStorageBackend::CurrentProject )
  {
    QgsProject::instance()->setDirty();
  }
}

//
// QgsQueryResultWidget
//

QgsQueryResultWidget::QgsQueryResultWidget( QWidget *parent, QgsAbstractDatabaseProviderConnection *connection )
  : QWidget( parent )
{
  setupUi( this );

  mToolBar->setIconSize( QgsGuiUtils::iconSize( false ) );

  mQueryWidget = new QgsQueryResultPanelWidget( nullptr, connection );
  mPanelStack->setMainPanel( mQueryWidget );

  mPresetQueryMenu = new QMenu( this );
  connect( mPresetQueryMenu, &QMenu::aboutToShow, this, &QgsQueryResultWidget::populatePresetQueryMenu );

  QToolButton *presetQueryButton = new QToolButton();
  presetQueryButton->setMenu( mPresetQueryMenu );
  presetQueryButton->setIcon( QgsApplication::getThemeIcon( u"mIconStoredQueries.svg"_s ) );
  presetQueryButton->setPopupMode( QToolButton::InstantPopup );
  mToolBar->addWidget( presetQueryButton );

  connect( mActionOpenQuery, &QAction::triggered, this, &QgsQueryResultWidget::openQuery );
  connect( mActionSaveQuery, &QAction::triggered, this, [this] { saveQuery( false ); } );
  connect( mActionSaveQueryAs, &QAction::triggered, this, [this] { saveQuery( true ); } );

  connect( mActionCut, &QAction::triggered, mQueryWidget->sqlEditor(), &QgsCodeEditor::cut );
  connect( mActionCopy, &QAction::triggered, mQueryWidget->sqlEditor(), &QgsCodeEditor::copy );
  connect( mActionPaste, &QAction::triggered, mQueryWidget->sqlEditor(), &QgsCodeEditor::paste );
  connect( mActionUndo, &QAction::triggered, mQueryWidget->sqlEditor(), &QgsCodeEditor::undo );
  connect( mActionRedo, &QAction::triggered, mQueryWidget->sqlEditor(), &QgsCodeEditor::redo );
  mActionUndo->setEnabled( false );
  mActionRedo->setEnabled( false );

  connect( mActionFindReplace, &QAction::toggled, mQueryWidget->codeEditorWidget(), &QgsCodeEditorWidget::setSearchBarVisible );
  connect( mQueryWidget->codeEditorWidget(), &QgsCodeEditorWidget::searchBarToggled, mActionFindReplace, &QAction::setChecked );
  connect( mQueryWidget->sqlEditor(), &QgsCodeEditor::modificationChanged, this, &QgsQueryResultWidget::setHasChanged );

  connect( mActionShowHistory, &QAction::toggled, this, &QgsQueryResultWidget::showHistoryPanel );

  connect( mActionClear, &QAction::triggered, this, [this] {
    // Cannot use setText() because it resets the undo/redo buffer.
    mQueryWidget->sqlEditor()->SendScintilla( QsciScintilla::SCI_SETTEXT, "" );
  } );

  connect( mQueryWidget->sqlEditor(), &QgsCodeEditorSQL::textChanged, this, &QgsQueryResultWidget::updateButtons );

  connect( mQueryWidget->sqlEditor(), &QgsCodeEditorSQL::copyAvailable, mActionCut, &QAction::setEnabled );
  connect( mQueryWidget->sqlEditor(), &QgsCodeEditorSQL::copyAvailable, mActionCopy, &QAction::setEnabled );

  connect( mQueryWidget, &QgsQueryResultPanelWidget::createSqlVectorLayer, this, &QgsQueryResultWidget::createSqlVectorLayer );
  connect( mQueryWidget, &QgsQueryResultPanelWidget::firstResultBatchFetched, this, &QgsQueryResultWidget::firstResultBatchFetched );

  updateButtons();
  setHasChanged( false );
}

QgsQueryResultWidget::~QgsQueryResultWidget()
{
  if ( mHistoryWidget )
  {
    mPanelStack->closePanel( mHistoryWidget );
    mHistoryWidget->deleteLater();
  }
}

void QgsQueryResultWidget::setSqlVectorLayerOptions( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options )
{
  if ( !options.sql.isEmpty() )
  {
    setQuery( options.sql );
  }
  mQueryWidget->setSqlVectorLayerOptions( options );
}

void QgsQueryResultWidget::setWidgetMode( QueryWidgetMode widgetMode )
{
  mQueryWidget->setWidgetMode( widgetMode );
}

void QgsQueryResultWidget::executeQuery()
{
  mQueryWidget->executeQuery();
}

void QgsQueryResultWidget::updateButtons()
{
  mQueryWidget->updateButtons();

  const bool isEmpty = mQueryWidget->sqlEditor()->text().isEmpty();
  mActionClear->setEnabled( !isEmpty );
  mActionUndo->setEnabled( mQueryWidget->sqlEditor()->isUndoAvailable() );
  mActionRedo->setEnabled( mQueryWidget->sqlEditor()->isRedoAvailable() );
}

void QgsQueryResultWidget::showError( const QString &title, const QString &message, bool isSqlError )
{
  mQueryWidget->showError( title, message, isSqlError );
}

void QgsQueryResultWidget::tokensReady( const QStringList & )
{
}

void QgsQueryResultWidget::copyResults()
{
  mQueryWidget->copyResults();
}

void QgsQueryResultWidget::copyResults( int fromRow, int toRow, int fromColumn, int toColumn )
{
  mQueryWidget->copyResults( fromRow, toRow, fromColumn, toColumn );
}

void QgsQueryResultWidget::openQuery()
{
  if ( !mQueryWidget->codeEditorWidget()->filePath().isEmpty() && mHasChangedFileContents )
  {
    if ( QMessageBox::warning( this, tr( "Unsaved Changes" ), tr( "There are unsaved changes in the query. Continue?" ), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
      return;
  }

  QString initialDir = settingLastSourceFolder->value();
  if ( initialDir.isEmpty() )
    initialDir = QDir::homePath();

  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Open Query" ), initialDir, tr( "SQL queries (*.sql *.SQL)" ) + u";;"_s + QObject::tr( "All files" ) + u" (*.*)"_s );

  if ( fileName.isEmpty() )
    return;

  QFileInfo fi( fileName );
  settingLastSourceFolder->setValue( fi.path() );

  QgsTemporaryCursorOverride cursor( Qt::CursorShape::WaitCursor );

  mQueryWidget->codeEditorWidget()->loadFile( fileName );
  setHasChanged( false );
}

void QgsQueryResultWidget::saveQuery( bool saveAs )
{
  if ( mQueryWidget->codeEditorWidget()->filePath().isEmpty() || saveAs )
  {
    QString selectedFilter;

    QString initialDir = settingLastSourceFolder->value();
    if ( initialDir.isEmpty() )
      initialDir = QDir::homePath();

    QString newPath = QFileDialog::getSaveFileName(
      this,
      tr( "Save Query" ),
      initialDir,
      tr( "SQL queries (*.sql *.SQL)" ) + u";;"_s + QObject::tr( "All files" ) + u" (*.*)"_s,
      &selectedFilter
    );

    if ( !newPath.isEmpty() )
    {
      QFileInfo fi( newPath );
      settingLastSourceFolder->setValue( fi.path() );

      if ( !selectedFilter.contains( u"*.*)"_s ) )
        newPath = QgsFileUtils::ensureFileNameHasExtension( newPath, { u"sql"_s } );
      mQueryWidget->codeEditorWidget()->save( newPath );
      setHasChanged( false );
    }
  }
  else if ( !mQueryWidget->codeEditorWidget()->filePath().isEmpty() )
  {
    mQueryWidget->codeEditorWidget()->save();
    setHasChanged( false );
  }
}

void QgsQueryResultWidget::setConnection( QgsAbstractDatabaseProviderConnection *connection )
{
  mQueryWidget->setConnection( connection );
  updateButtons();
}

void QgsQueryResultWidget::setQuery( const QString &sql )
{
  mQueryWidget->sqlEditor()->setText( sql );
  // from the QScintilla docs, calling setText clears undo history!
  mActionUndo->setEnabled( false );
  mActionRedo->setEnabled( false );
}

bool QgsQueryResultWidget::promptUnsavedChanges()
{
  if ( !mQueryWidget->codeEditorWidget()->filePath().isEmpty() && mHasChangedFileContents )
  {
    const QMessageBox::StandardButton ret = QMessageBox::question(
      this,
      tr( "Save Query?" ),
      tr(
        "There are unsaved changes in this query. Do you want to save those?"
      ),
      QMessageBox::StandardButton::Save
        | QMessageBox::StandardButton::Cancel
        | QMessageBox::StandardButton::Discard,
      QMessageBox::StandardButton::Cancel
    );

    if ( ret == QMessageBox::StandardButton::Save )
    {
      saveQuery( false );
      return true;
    }
    else if ( ret == QMessageBox::StandardButton::Discard )
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true;
  }
}


void QgsQueryResultWidget::notify( const QString &title, const QString &text, Qgis::MessageLevel level )
{
  mQueryWidget->notify( title, text, level );
}


void QgsQueryResultWidget::setHasChanged( bool hasChanged )
{
  mActionSaveQuery->setEnabled( hasChanged );
  mHasChangedFileContents = hasChanged;
  updateDialogTitle();
}

void QgsQueryResultWidget::updateDialogTitle()
{
  QString fileName;
  if ( !mQueryWidget->codeEditorWidget()->filePath().isEmpty() )
  {
    const QFileInfo fi( mQueryWidget->codeEditorWidget()->filePath() );
    fileName = fi.fileName();
    if ( mHasChangedFileContents )
    {
      fileName.prepend( '*' );
    }
  }

  emit requestDialogTitleUpdate( fileName );
}

void QgsQueryResultWidget::populatePresetQueryMenu()
{
  mPresetQueryMenu->clear();

  QMenu *storeQueryMenu = new QMenu( tr( "Store Current Query" ), mPresetQueryMenu );
  mPresetQueryMenu->addMenu( storeQueryMenu );
  QAction *storeInProfileAction = new QAction( tr( "In User Profile…" ), storeQueryMenu );
  storeQueryMenu->addAction( storeInProfileAction );
  storeInProfileAction->setEnabled( !mQueryWidget->sqlEditor()->text().isEmpty() );
  connect( storeInProfileAction, &QAction::triggered, this, [this] {
    storeCurrentQuery( Qgis::QueryStorageBackend::LocalProfile );
  } );
  QAction *storeInProjectAction = new QAction( tr( "In Current Project…" ), storeQueryMenu );
  storeQueryMenu->addAction( storeInProjectAction );
  storeInProjectAction->setEnabled( !mQueryWidget->sqlEditor()->text().isEmpty() );
  connect( storeInProjectAction, &QAction::triggered, this, [this] {
    storeCurrentQuery( Qgis::QueryStorageBackend::CurrentProject );
  } );


  const QList< QgsStoredQueryManager::QueryDetails > storedQueries = QgsGui::storedQueryManager()->allQueries();
  if ( !storedQueries.isEmpty() )
  {
    QList< QgsStoredQueryManager::QueryDetails > userProfileQueries;
    std::copy_if( storedQueries.begin(), storedQueries.end(), std::back_inserter( userProfileQueries ), []( const QgsStoredQueryManager::QueryDetails &details ) {
      return details.backend == Qgis::QueryStorageBackend::LocalProfile;
    } );

    QList< QgsStoredQueryManager::QueryDetails > projectQueries;
    std::copy_if( storedQueries.begin(), storedQueries.end(), std::back_inserter( projectQueries ), []( const QgsStoredQueryManager::QueryDetails &details ) {
      return details.backend == Qgis::QueryStorageBackend::CurrentProject;
    } );

    mPresetQueryMenu->addSection( QgsApplication::getThemeIcon( u"mIconStoredQueries.svg"_s ), tr( "User Profile" ) );
    for ( const QgsStoredQueryManager::QueryDetails &query : std::as_const( userProfileQueries ) )
    {
      QAction *action = new QAction( query.name, mPresetQueryMenu );
      mPresetQueryMenu->addAction( action );
      connect( action, &QAction::triggered, this, [this, query] {
        mQueryWidget->sqlEditor()->insertText( query.definition );
      } );
    }
    if ( userProfileQueries.empty() )
    {
      QAction *action = new QAction( tr( "No Stored Queries Available" ), mPresetQueryMenu );
      action->setEnabled( false );
      mPresetQueryMenu->addAction( action );
    }

    mPresetQueryMenu->addSection( QgsApplication::getThemeIcon( u"mIconStoredQueries.svg"_s ), tr( "Current Project" ) );
    for ( const QgsStoredQueryManager::QueryDetails &query : std::as_const( projectQueries ) )
    {
      QAction *action = new QAction( query.name, mPresetQueryMenu );
      mPresetQueryMenu->addAction( action );
      connect( action, &QAction::triggered, this, [this, query] {
        mQueryWidget->sqlEditor()->insertText( query.definition );
      } );
    }
    if ( projectQueries.empty() )
    {
      QAction *action = new QAction( tr( "No Stored Queries Available" ), mPresetQueryMenu );
      action->setEnabled( false );
      mPresetQueryMenu->addAction( action );
    }

    mPresetQueryMenu->addSeparator();

    QMenu *removeQueryMenu = new QMenu( tr( "Removed Stored Query" ), mPresetQueryMenu );
    mPresetQueryMenu->addMenu( removeQueryMenu );

    for ( const QgsStoredQueryManager::QueryDetails &query : storedQueries )
    {
      QAction *action = new QAction( tr( "%1…" ).arg( query.name ), mPresetQueryMenu );
      removeQueryMenu->addAction( action );
      connect( action, &QAction::triggered, this, [this, query] {
        const QMessageBox::StandardButton res = QMessageBox::question( this, tr( "Remove Stored Query" ), tr( "Are you sure you want to remove the stored query “%1”?" ).arg( query.name ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
        if ( res == QMessageBox::Yes )
        {
          QgsGui::storedQueryManager()->removeQuery( query.name, query.backend );
          if ( query.backend == Qgis::QueryStorageBackend::CurrentProject )
          {
            QgsProject::instance()->setDirty();
          }
        }
      } );
    }
  }
}

void QgsQueryResultWidget::storeCurrentQuery( Qgis::QueryStorageBackend backend )
{
  const QStringList existingQueryNames = QgsGui::storedQueryManager()->allQueryNames( backend );
  QgsNewNameDialog dlg(
    QString(),
    QString(),
    QStringList(),
    existingQueryNames
  );
  dlg.setWindowTitle( tr( "Store Query" ) );
  dlg.setHintString( tr( "Name for the stored query" ) );
  dlg.setOverwriteEnabled( true );
  dlg.setConflictingNameWarning( tr( "A stored query with this name already exists, it will be overwritten." ) );
  dlg.setShowExistingNamesCompleter( true );
  if ( dlg.exec() != QDialog::Accepted )
    return;

  const QString name = dlg.name();
  if ( name.isEmpty() )
    return;

  QgsGui::storedQueryManager()->storeQuery( name, mQueryWidget->sqlEditor()->text(), backend );
  if ( backend == Qgis::QueryStorageBackend::CurrentProject )
  {
    QgsProject::instance()->setDirty();
  }
}


void QgsQueryResultWidget::showHistoryPanel( bool show )
{
  // the below code block trips up the clang analyser!
  // NOLINTBEGIN(bugprone-branch-clone)
  if ( show )
  {
    mHistoryWidget = new QgsDatabaseQueryHistoryWidget();
    mHistoryWidget->setPanelTitle( tr( "SQL History" ) );
    mPanelStack->showPanel( mHistoryWidget );
    connect( mHistoryWidget, &QgsPanelWidget::panelAccepted, this, [this] { whileBlocking( mActionShowHistory )->setChecked( false ); } );
    connect( mHistoryWidget, &QgsDatabaseQueryHistoryWidget::sqlTriggered, this, [this]( const QString &connectionUri, const QString &provider, const QString &sql ) {
      Q_UNUSED( connectionUri );
      Q_UNUSED( provider );

      mQueryWidget->sqlEditor()->setText( sql );
      mActionUndo->setEnabled( false );
      mActionRedo->setEnabled( false );
      mHistoryWidget->acceptPanel();
    } );
  }
  else if ( mHistoryWidget )
  {
    mPanelStack->closePanel( mHistoryWidget );
    mHistoryWidget->deleteLater();
  }
  // NOLINTEND(bugprone-branch-clone)
}


///@cond private

void QgsConnectionsApiFetcher::fetchTokens()
{
  if ( mStopFetching )
  {
    emit fetchingFinished();
    return;
  }


  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( mProviderKey );
  if ( !md )
  {
    emit fetchingFinished();
    return;
  }
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> connection( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( mUri, {} ) ) );
  if ( !mStopFetching && connection )
  {
    mFeedback = std::make_unique<QgsFeedback>();
    QStringList schemas;
    if ( connection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Schemas ) )
    {
      try
      {
        schemas = connection->schemas();
        emit tokensReady( schemas );
      }
      catch ( QgsProviderConnectionException &ex )
      {
        QgsMessageLog::logMessage( tr( "Error retrieving schemas: %1" ).arg( ex.what() ), u"QGIS"_s, Qgis::MessageLevel::Warning );
      }
    }
    else
    {
      schemas.push_back( QString() ); // Fake empty schema for DBs not supporting it
    }

    for ( const auto &schema : std::as_const( schemas ) )
    {
      if ( mStopFetching )
      {
        connection.reset();
        emit fetchingFinished();
        return;
      }

      QStringList tableNames;
      try
      {
        const QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables = connection->tables( schema, QgsAbstractDatabaseProviderConnection::TableFlags(), mFeedback.get() );
        for ( const QgsAbstractDatabaseProviderConnection::TableProperty &table : std::as_const( tables ) )
        {
          if ( mStopFetching )
          {
            connection.reset();
            emit fetchingFinished();
            return;
          }
          tableNames.push_back( table.tableName() );
        }
        emit tokensReady( tableNames );
      }
      catch ( QgsProviderConnectionException &ex )
      {
        QgsMessageLog::logMessage( tr( "Error retrieving tables: %1" ).arg( ex.what() ), u"QGIS"_s, Qgis::MessageLevel::Warning );
      }

      // Get fields
      for ( const auto &table : std::as_const( tableNames ) )
      {
        if ( mStopFetching )
        {
          connection.reset();
          emit fetchingFinished();
          return;
        }

        QStringList fieldNames;
        try
        {
          const QgsFields fields( connection->fields( schema, table, mFeedback.get() ) );
          if ( mStopFetching )
          {
            connection.reset();
            emit fetchingFinished();
            return;
          }

          for ( const auto &field : std::as_const( fields ) )
          {
            fieldNames.push_back( field.name() );
            if ( mStopFetching )
            {
              connection.reset();
              emit fetchingFinished();
              return;
            }
          }
          emit tokensReady( fieldNames );
        }
        catch ( QgsProviderConnectionException &ex )
        {
          QgsMessageLog::logMessage( tr( "Error retrieving fields for table %1: %2" ).arg( table, ex.what() ), u"QGIS"_s, Qgis::MessageLevel::Warning );
        }
      }
    }
  }

  connection.reset();
  emit fetchingFinished();
}

void QgsConnectionsApiFetcher::stopFetching()
{
  mStopFetching = 1;
  if ( mFeedback )
    mFeedback->cancel();
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
    result.append( u"…"_s );
  }
  return result;
}

///@endcond private

//
// QgsQueryResultDialog
//

QgsQueryResultDialog::QgsQueryResultDialog( QgsAbstractDatabaseProviderConnection *connection, QWidget *parent )
  : QDialog( parent )
{
  setObjectName( u"QgsQueryResultDialog"_s );
  QgsGui::enableAutoGeometryRestore( this );

  mWidget = new QgsQueryResultWidget( this, connection );
  QVBoxLayout *l = new QVBoxLayout();
  l->setContentsMargins( 6, 6, 6, 6 );

  QDialogButtonBox *mButtonBox = new QDialogButtonBox( QDialogButtonBox::StandardButton::Close | QDialogButtonBox::StandardButton::Help );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::close );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [] {
    QgsHelp::openHelp( u"managing_data_source/create_layers.html#execute-sql"_s );
  } );
  l->addWidget( mWidget );
  l->addWidget( mButtonBox );

  setLayout( l );
}

void QgsQueryResultDialog::closeEvent( QCloseEvent *event )
{
  if ( !mWidget->promptUnsavedChanges() )
  {
    event->ignore();
  }
  else
  {
    event->accept();
  }
}

//
// QgsQueryResultMainWindow
//

QgsQueryResultMainWindow::QgsQueryResultMainWindow( QgsAbstractDatabaseProviderConnection *connection, const QString &identifierName )
  : mIdentifierName( identifierName )
{
  setObjectName( u"SQLCommandsDialog"_s );

  QgsGui::enableAutoGeometryRestore( this );

  mWidget = new QgsQueryResultWidget( nullptr, connection );
  setCentralWidget( mWidget );
  mWidget->layout()->setContentsMargins( 6, 6, 6, 6 );

  connect( mWidget, &QgsQueryResultWidget::requestDialogTitleUpdate, this, &QgsQueryResultMainWindow::updateWindowTitle );

  updateWindowTitle( QString() );
}

void QgsQueryResultMainWindow::closeEvent( QCloseEvent *event )
{
  if ( !mWidget->promptUnsavedChanges() )
  {
    event->ignore();
  }
  else
  {
    event->accept();
  }
}

void QgsQueryResultMainWindow::updateWindowTitle( const QString &fileName )
{
  if ( fileName.isEmpty() )
  {
    if ( !mIdentifierName.isEmpty() )
      setWindowTitle( tr( "%1 — Execute SQL" ).arg( mIdentifierName ) );
    else
      setWindowTitle( tr( "Execute SQL" ) );
  }
  else
  {
    if ( !mIdentifierName.isEmpty() )
      setWindowTitle( tr( "%1 — %2 — Execute SQL" ).arg( fileName, mIdentifierName ) );
    else
      setWindowTitle( tr( "%1 — Execute SQL" ).arg( fileName ) );
  }
}
