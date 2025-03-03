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
#include "moc_qgsqueryresultwidget.cpp"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsexpressionutils.h"
#include "qgscodeeditorsql.h"
#include "qgsmessagelog.h"
#include "qgsquerybuilder.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgshistoryproviderregistry.h"
#include "qgshistoryentry.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgscodeeditorwidget.h"
#include "qgsfileutils.h"
#include "qgsstoredquerymanager.h"
#include "qgsproject.h"

#include <QClipboard>
#include <QShortcut>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

///@cond PRIVATE
const QgsSettingsEntryString *QgsQueryResultWidget::settingLastSourceFolder = new QgsSettingsEntryString( QStringLiteral( "last-source-folder" ), sTreeSqlQueries, QString(), QStringLiteral( "Last used folder for SQL source files" ) );
///@endcond PRIVATE

QgsQueryResultWidget::QgsQueryResultWidget( QWidget *parent, QgsAbstractDatabaseProviderConnection *connection )
  : QWidget( parent )
{
  setupUi( this );

  // Unsure :/
  // mSqlEditor->setLineNumbersVisible( true );

  mToolBar->setIconSize( QgsGuiUtils::iconSize( false ) );

  mPresetQueryMenu = new QMenu( this );
  connect( mPresetQueryMenu, &QMenu::aboutToShow, this, &QgsQueryResultWidget::populatePresetQueryMenu );

  QToolButton *presetQueryButton = new QToolButton();
  presetQueryButton->setMenu( mPresetQueryMenu );
  presetQueryButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconStoredQueries.svg" ) ) );
  presetQueryButton->setPopupMode( QToolButton::InstantPopup );
  mToolBar->addWidget( presetQueryButton );

  // explicitly needed for some reason (Qt 5.15)
  mainLayout->setSpacing( 6 );
  progressLayout->setSpacing( 6 );

  mQueryResultsTableView->hide();
  mQueryResultsTableView->setItemDelegate( new QgsQueryResultItemDelegate( mQueryResultsTableView ) );
  mQueryResultsTableView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mQueryResultsTableView, &QTableView::customContextMenuRequested, this, &QgsQueryResultWidget::showCellContextMenu );

  mProgressBar->hide();

  mSqlEditor = new QgsCodeEditorSQL();
  mCodeEditorWidget = new QgsCodeEditorWidget( mSqlEditor, mMessageBar );
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->addWidget( mCodeEditorWidget );
  mSqlEditorContainer->setLayout( vl );

  connect( mActionOpenQuery, &QAction::triggered, this, &QgsQueryResultWidget::openQuery );
  connect( mActionSaveQuery, &QAction::triggered, this, [this] { saveQuery( false ); } );
  connect( mActionSaveQueryAs, &QAction::triggered, this, [this] { saveQuery( true ); } );

  connect( mActionCut, &QAction::triggered, mSqlEditor, &QgsCodeEditor::cut );
  connect( mActionCopy, &QAction::triggered, mSqlEditor, &QgsCodeEditor::copy );
  connect( mActionPaste, &QAction::triggered, mSqlEditor, &QgsCodeEditor::paste );
  connect( mActionUndo, &QAction::triggered, mSqlEditor, &QgsCodeEditor::undo );
  connect( mActionRedo, &QAction::triggered, mSqlEditor, &QgsCodeEditor::redo );
  mActionUndo->setEnabled( false );
  mActionRedo->setEnabled( false );

  connect( mActionFindReplace, &QAction::toggled, mCodeEditorWidget, &QgsCodeEditorWidget::setSearchBarVisible );
  connect( mCodeEditorWidget, &QgsCodeEditorWidget::searchBarToggled, mActionFindReplace, &QAction::setChecked );
  connect( mSqlEditor, &QgsCodeEditor::modificationChanged, this, &QgsQueryResultWidget::setHasChanged );

  connect( mExecuteButton, &QPushButton::pressed, this, &QgsQueryResultWidget::executeQuery );

  connect( mActionClear, &QAction::triggered, this, [=] {
    mSqlEditor->setText( QString() );
    mActionUndo->setEnabled( false );
    mActionRedo->setEnabled( false );
  } );
  connect( mLoadLayerPushButton, &QPushButton::pressed, this, [=] {
    if ( mConnection )
    {
      emit createSqlVectorLayer( mConnection->providerKey(), mConnection->uri(), sqlVectorLayerOptions() );
    }
  } );
  connect( mSqlEditor, &QgsCodeEditorSQL::textChanged, this, &QgsQueryResultWidget::updateButtons );

  connect( mSqlEditor, &QgsCodeEditorSQL::copyAvailable, mActionCut, &QAction::setEnabled );
  connect( mSqlEditor, &QgsCodeEditorSQL::copyAvailable, mActionCopy, &QAction::setEnabled );

  connect( mSqlEditor, &QgsCodeEditorSQL::selectionChanged, this, [=] {
    mExecuteButton->setText( mSqlEditor->selectedText().isEmpty() ? tr( "Execute" ) : tr( "Execute Selection" ) );
  } );
  connect( mFilterToolButton, &QToolButton::pressed, this, [=] {
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

  connect( mLoadAsNewLayerGroupBox, &QgsCollapsibleGroupBox::collapsedStateChanged, this, [=]( bool collapsed ) {
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
  connect( copySelection, &QShortcut::activated, this, &QgsQueryResultWidget::copySelection );

  setConnection( connection );
  setHasChanged( false );
}

QgsQueryResultWidget::~QgsQueryResultWidget()
{
  cancelApiFetcher();
  cancelRunningQuery();
}

void QgsQueryResultWidget::setSqlVectorLayerOptions( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options )
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

void QgsQueryResultWidget::setWidgetMode( QueryWidgetMode widgetMode )
{
  mQueryWidgetMode = widgetMode;
  switch ( widgetMode )
  {
    case QueryWidgetMode::SqlQueryMode:
      mLoadAsNewLayerGroupBox->setTitle( tr( "Load as New Layer" ) );
      mLoadLayerPushButton->setText( tr( "Load Layer" ) );
      mLoadAsNewLayerGroupBox->setCollapsed( true );
      break;
    case QueryWidgetMode::QueryLayerUpdateMode:
      mLoadAsNewLayerGroupBox->setTitle( tr( "Update Query Layer" ) );
      mLoadLayerPushButton->setText( tr( "Update Layer" ) );
      mLoadAsNewLayerGroupBox->setCollapsed( false );
      break;
  }
}

void QgsQueryResultWidget::executeQuery()
{
  mQueryResultsTableView->hide();
  mSqlErrorText->hide();
  mFirstRowFetched = false;

  cancelRunningQuery();
  if ( mConnection )
  {
    const QString sql { mSqlEditor->selectedText().isEmpty() ? mSqlEditor->text() : mSqlEditor->selectedText() };

    bool ok = false;
    mCurrentHistoryEntryId = QgsGui::historyProviderRegistry()->addEntry( QStringLiteral( "dbquery" ), QVariantMap {
                                                                                                         { QStringLiteral( "query" ), sql },
                                                                                                         { QStringLiteral( "provider" ), mConnection->providerKey() },
                                                                                                         { QStringLiteral( "connection" ), mConnection->uri() },
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

    connect( mStopButton, &QPushButton::pressed, mFeedback.get(), [=] {
      mStatusLabel->setText( tr( "Stopped" ) );
      mFeedback->cancel();
      mProgressBar->hide();
      mWasCanceled = true;
    } );

    // Create model when result is ready
    connect( &mQueryResultWatcher, &QFutureWatcher<QgsAbstractDatabaseProviderConnection::QueryResult>::finished, this, &QgsQueryResultWidget::startFetching, Qt::ConnectionType::UniqueConnection );

    QFuture<QgsAbstractDatabaseProviderConnection::QueryResult> future = QtConcurrent::run( [=]() -> QgsAbstractDatabaseProviderConnection::QueryResult {
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
  const bool isEmpty = mSqlEditor->text().isEmpty();
  mExecuteButton->setEnabled( !isEmpty );
  mActionClear->setEnabled( !isEmpty );
  mActionUndo->setEnabled( mSqlEditor->isUndoAvailable() );
  mActionRedo->setEnabled( mSqlEditor->isRedoAvailable() );
  mLoadAsNewLayerGroupBox->setVisible( mConnection && mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::SqlLayers ) );
  mLoadAsNewLayerGroupBox->setEnabled(
    mSqlErrorMessage.isEmpty() && mFirstRowFetched
  );
}

void QgsQueryResultWidget::showCellContextMenu( QPoint point )
{
  const QModelIndex modelIndex = mQueryResultsTableView->indexAt( point );
  if ( modelIndex.isValid() )
  {
    QMenu *menu = new QMenu();
    menu->setAttribute( Qt::WA_DeleteOnClose );

    menu->addAction( QgsApplication::getThemeIcon( "mActionEditCopy.svg" ), tr( "Copy" ), this, [=] { copySelection(); }, QKeySequence::Copy );

    menu->exec( mQueryResultsTableView->viewport()->mapToGlobal( point ) );
  }
}

void QgsQueryResultWidget::copySelection()
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

void QgsQueryResultWidget::updateSqlLayerColumns()
{
  // Precondition
  Q_ASSERT( mModel );

  mFilterToolButton->setEnabled( true );
  mFilterLineEdit->setEnabled( true );
  mPkColumnsComboBox->clear();
  mGeometryColumnComboBox->clear();
  const bool hasPkInformation { !mSqlVectorLayerOptions.primaryKeyColumns.isEmpty() };
  const bool hasGeomColInformation { !mSqlVectorLayerOptions.geometryColumn.isEmpty() };
  static const QStringList geomColCandidates { QStringLiteral( "geom" ), QStringLiteral( "geometry" ), QStringLiteral( "the_geom" ) };
  const QStringList constCols { mModel->columns() };
  for ( const QString &c : constCols )
  {
    const bool pkCheckedState = hasPkInformation ? mSqlVectorLayerOptions.primaryKeyColumns.contains( c ) : c.contains( QStringLiteral( "id" ), Qt::CaseSensitivity::CaseInsensitive );
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
    // apiFetcher and apiFetcherWorkerThread will be deleted when the thread fetchingFinished signal is emitted
  }
}

void QgsQueryResultWidget::startFetching()
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
        mStatusLabel->setText( QStringLiteral( "Query executed successfully (%1 rows, %2 ms)" )
                                 .arg( QLocale().toString( mQueryResultWatcher.result().rowCount() ), QLocale().toString( mQueryResultWatcher.result().queryExecutionTime() ) ) );
      }
      else
      {
        mStatusLabel->setText( QStringLiteral( "Query executed successfully (%1 s)" ).arg( QLocale().toString( mQueryResultWatcher.result().queryExecutionTime() ) ) );
      }
      mProgressBar->hide();
      mModel = std::make_unique<QgsQueryResultModel>( mQueryResultWatcher.result() );
      connect( mFeedback.get(), &QgsFeedback::canceled, mModel.get(), [=] {
        mModel->cancel();
        mWasCanceled = true;
      } );

      connect( mModel.get(), &QgsQueryResultModel::fetchMoreRows, this, [=]( long long maxRows ) {
        mFetchedRowsBatchCount = 0;
        mProgressBar->setRange( 0, maxRows );
        mProgressBar->show();
      } );

      connect( mModel.get(), &QgsQueryResultModel::rowsInserted, this, [=]( const QModelIndex &, int first, int last ) {
        if ( !mFirstRowFetched )
        {
          emit firstResultBatchFetched();
          mFirstRowFetched = true;
          mQueryResultsTableView->show();
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

      connect( mModel.get(), &QgsQueryResultModel::fetchingComplete, mStopButton, [=] {
        bool ok = false;
        const QgsHistoryEntry currentHistoryEntry = QgsGui::historyProviderRegistry()->entry( mCurrentHistoryEntryId, ok );
        QVariantMap entryDetails = currentHistoryEntry.entry;
        entryDetails.insert( QStringLiteral( "rows" ), mActualRowCount );
        entryDetails.insert( QStringLiteral( "time" ), mQueryResultWatcher.result().queryExecutionTime() );

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

void QgsQueryResultWidget::copyResults()
{
  const int rowCount = mModel->rowCount( QModelIndex() );
  const int columnCount = mModel->columnCount( QModelIndex() );
  copyResults( 0, rowCount - 1, 0, columnCount - 1 );
}

void QgsQueryResultWidget::copyResults( int fromRow, int toRow, int fromColumn, int toColumn )
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
    QString html = QStringLiteral( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>%1</td></tr></table></body></html>" ).arg( text );
    html.replace( QLatin1String( "\t" ), QLatin1String( "</td><td>" ) ).replace( QLatin1String( "\n" ), QLatin1String( "</td></tr><tr><td>" ) );

    QMimeData *mdata = new QMimeData();
    mdata->setData( QStringLiteral( "text/html" ), html.toUtf8() );
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

void QgsQueryResultWidget::openQuery()
{
  if ( !mCodeEditorWidget->filePath().isEmpty() && mHasChangedFileContents )
  {
    if ( QMessageBox::warning( this, tr( "Unsaved Changes" ), tr( "There are unsaved changes in the query. Continue?" ), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
      return;
  }

  QString initialDir = settingLastSourceFolder->value();
  if ( initialDir.isEmpty() )
    initialDir = QDir::homePath();

  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Open Query" ), initialDir, tr( "SQL queries (*.sql *.SQL)" ) + QStringLiteral( ";;" ) + QObject::tr( "All files" ) + QStringLiteral( " (*.*)" ) );

  if ( fileName.isEmpty() )
    return;

  QFileInfo fi( fileName );
  settingLastSourceFolder->setValue( fi.path() );

  QgsTemporaryCursorOverride cursor( Qt::CursorShape::WaitCursor );

  mCodeEditorWidget->loadFile( fileName );
  setHasChanged( false );
}

void QgsQueryResultWidget::saveQuery( bool saveAs )
{
  if ( mCodeEditorWidget->filePath().isEmpty() || saveAs )
  {
    QString selectedFilter;

    QString initialDir = settingLastSourceFolder->value();
    if ( initialDir.isEmpty() )
      initialDir = QDir::homePath();

    QString newPath = QFileDialog::getSaveFileName(
      this,
      tr( "Save Query" ),
      initialDir,
      tr( "SQL queries (*.sql *.SQL)" ) + QStringLiteral( ";;" ) + QObject::tr( "All files" ) + QStringLiteral( " (*.*)" ),
      &selectedFilter
    );

    if ( !newPath.isEmpty() )
    {
      QFileInfo fi( newPath );
      settingLastSourceFolder->setValue( fi.path() );

      if ( !selectedFilter.contains( QStringLiteral( "*.*)" ) ) )
        newPath = QgsFileUtils::ensureFileNameHasExtension( newPath, { QStringLiteral( "sql" ) } );
      mCodeEditorWidget->save( newPath );
      setHasChanged( false );
    }
  }
  else if ( !mCodeEditorWidget->filePath().isEmpty() )
  {
    mCodeEditorWidget->save();
    setHasChanged( false );
  }
}

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsQueryResultWidget::sqlVectorLayerOptions() const
{
  const thread_local QRegularExpression rx( QStringLiteral( ";\\s*$" ) );
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

void QgsQueryResultWidget::setConnection( QgsAbstractDatabaseProviderConnection *connection )
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
    connect( apiFetcher, &QgsConnectionsApiFetcher::tokensReady, this, &QgsQueryResultWidget::tokensReady );
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

void QgsQueryResultWidget::setQuery( const QString &sql )
{
  mSqlEditor->setText( sql );
  // from the QScintilla docs, calling setText clears undo history!
  mActionUndo->setEnabled( false );
  mActionRedo->setEnabled( false );
}


bool QgsQueryResultWidget::promptUnsavedChanges()
{
  if ( !mCodeEditorWidget->filePath().isEmpty() && mHasChangedFileContents )
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
  mMessageBar->pushMessage( title, text, level );
}


void QgsQueryResultWidget::setHasChanged( bool hasChanged )
{
  mHasChangedFileContents = hasChanged;
  mActionSaveQuery->setEnabled( hasChanged );
  updateDialogTitle();
}

void QgsQueryResultWidget::updateDialogTitle()
{
  QString fileName;
  if ( !mCodeEditorWidget->filePath().isEmpty() )
  {
    const QFileInfo fi( mCodeEditorWidget->filePath() );
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
  storeInProfileAction->setEnabled( !mSqlEditor->text().isEmpty() );
  connect( storeInProfileAction, &QAction::triggered, this, [this] {
    storeCurrentQuery( Qgis::QueryStorageBackend::LocalProfile );
  } );
  QAction *storeInProjectAction = new QAction( tr( "In Current Project…" ), storeQueryMenu );
  storeQueryMenu->addAction( storeInProjectAction );
  storeInProjectAction->setEnabled( !mSqlEditor->text().isEmpty() );
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

    mPresetQueryMenu->addSection( QgsApplication::getThemeIcon( QStringLiteral( "mIconStoredQueries.svg" ) ), tr( "User Profile" ) );
    for ( const QgsStoredQueryManager::QueryDetails &query : std::as_const( userProfileQueries ) )
    {
      QAction *action = new QAction( query.name, mPresetQueryMenu );
      mPresetQueryMenu->addAction( action );
      connect( action, &QAction::triggered, this, [this, query] {
        mSqlEditor->insertText( query.definition );
      } );
    }
    if ( userProfileQueries.empty() )
    {
      QAction *action = new QAction( tr( "No Stored Queries Available" ), mPresetQueryMenu );
      action->setEnabled( false );
      mPresetQueryMenu->addAction( action );
    }

    mPresetQueryMenu->addSection( QgsApplication::getThemeIcon( QStringLiteral( "mIconStoredQueries.svg" ) ), tr( "Current Project" ) );
    for ( const QgsStoredQueryManager::QueryDetails &query : std::as_const( projectQueries ) )
    {
      QAction *action = new QAction( query.name, mPresetQueryMenu );
      mPresetQueryMenu->addAction( action );
      connect( action, &QAction::triggered, this, [this, query] {
        mSqlEditor->insertText( query.definition );
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
  bool ok;
  const QString name = QInputDialog::getText( this, tr( "Query Name" ), tr( "Name for the stored query" ), QLineEdit::Normal, QString(), &ok );
  if ( !ok || name.isEmpty() )
    return;

  QgsGui::storedQueryManager()->storeQuery( name, mSqlEditor->text(), backend );
  if ( backend == Qgis::QueryStorageBackend::CurrentProject )
  {
    QgsProject::instance()->setDirty();
  }
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
        QgsMessageLog::logMessage( tr( "Error retrieving schemas: %1" ).arg( ex.what() ), QStringLiteral( "QGIS" ), Qgis::MessageLevel::Warning );
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
        QgsMessageLog::logMessage( tr( "Error retrieving tables: %1" ).arg( ex.what() ), QStringLiteral( "QGIS" ), Qgis::MessageLevel::Warning );
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
          QgsMessageLog::logMessage( tr( "Error retrieving fields for table %1: %2" ).arg( table, ex.what() ), QStringLiteral( "QGIS" ), Qgis::MessageLevel::Warning );
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
    result.append( QStringLiteral( "…" ) );
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
  setObjectName( QStringLiteral( "QgsQueryResultDialog" ) );
  QgsGui::enableAutoGeometryRestore( this );

  mWidget = new QgsQueryResultWidget( this, connection );
  QVBoxLayout *l = new QVBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  l->addWidget( mWidget );
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
  setObjectName( QStringLiteral( "SQLCommandsDialog" ) );

  QgsGui::enableAutoGeometryRestore( this );

  mWidget = new QgsQueryResultWidget( nullptr, connection );
  setCentralWidget( mWidget );

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
