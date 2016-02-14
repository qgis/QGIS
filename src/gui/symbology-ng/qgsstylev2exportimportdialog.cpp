/***************************************************************************
    qgsstylev2exportimportdialog.cpp
    ---------------------
    begin                : Jan 2011
    copyright            : (C) 2011 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstylev2exportimportdialog.h"
#include "ui_qgsstylev2exportimportdialogbase.h"

#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgslogger.h"

#include <QInputDialog>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>


QgsStyleV2ExportImportDialog::QgsStyleV2ExportImportDialog( QgsStyleV2* style, QWidget *parent, Mode mode )
    : QDialog( parent )
    , mDialogMode( mode )
    , mQgisStyle( style )
{
  setupUi( this );

  // additional buttons
  QPushButton *pb;
  pb = new QPushButton( tr( "Select all" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( selectAll() ) );

  pb = new QPushButton( tr( "Clear selection" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( clearSelection() ) );

  QStandardItemModel* model = new QStandardItemModel( listItems );

  listItems->setModel( model );
  connect( listItems->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged( const QItemSelection&, const QItemSelection& ) ) );

  mTempStyle = new QgsStyleV2();
  // TODO validate
  mFileName = "";
  mProgressDlg = nullptr;
  mGroupSelectionDlg = nullptr;
  mTempFile = nullptr;
  mNetManager = new QNetworkAccessManager( this );
  mNetReply = nullptr;

  if ( mDialogMode == Import )
  {
    setWindowTitle( tr( "Import symbol(s)" ) );
    // populate the import types
    importTypeCombo->addItem( tr( "file specified below" ), QVariant( "file" ) );
    // importTypeCombo->addItem( "official QGIS repo online", QVariant( "official" ) );
    importTypeCombo->addItem( tr( "URL specified below" ), QVariant( "url" ) );
    connect( importTypeCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( importTypeChanged( int ) ) );

    QStringList groups = mQgisStyle->groupNames();
    groupCombo->addItem( "imported", QVariant( "new" ) );
    Q_FOREACH ( const QString& gName, groups )
    {
      groupCombo->addItem( gName );
    }

    btnBrowse->setText( "Browse" );
    connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( browse() ) );

    label->setText( tr( "Select symbols to import" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Import" ) );
  }
  else
  {
    setWindowTitle( tr( "Export symbol(s)" ) );
    // hide import specific controls when exporting
    btnBrowse->setHidden( true );
    fromLabel->setHidden( true );
    importTypeCombo->setHidden( true );
    locationLabel->setHidden( true );
    locationLineEdit->setHidden( true );

    pb = new QPushButton( tr( "Select by group" ) );
    buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
    connect( pb, SIGNAL( clicked() ), this, SLOT( selectByGroup() ) );
    groupLabel->setHidden( true );
    groupCombo->setHidden( true );

    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Export" ) );
    if ( !populateStyles( mQgisStyle ) )
    {
      QApplication::postEvent( this, new QCloseEvent() );
    }

  }
  // use Ok button for starting import and export operations
  disconnect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( doExportImport() ) );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

void QgsStyleV2ExportImportDialog::doExportImport()
{
  QModelIndexList selection = listItems->selectionModel()->selectedIndexes();
  if ( selection.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Export/import error" ),
                          tr( "You should select at least one symbol/color ramp." ) );
    return;
  }

  if ( mDialogMode == Export )
  {
    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save styles" ), QDir::homePath(),
                       tr( "XML files (*.xml *.XML)" ) );
    if ( fileName.isEmpty() )
    {
      return;
    }

    // ensure the user never ommited the extension from the file name
    if ( !fileName.endsWith( ".xml", Qt::CaseInsensitive ) )
    {
      fileName += ".xml";
    }

    mFileName = fileName;

    moveStyles( &selection, mQgisStyle, mTempStyle );
    if ( !mTempStyle->exportXML( mFileName ) )
    {
      QMessageBox::warning( this, tr( "Export/import error" ),
                            tr( "Error when saving selected symbols to file:\n%1" )
                            .arg( mTempStyle->errorString() ) );
      return;
    }
  }
  else // import
  {
    moveStyles( &selection, mTempStyle, mQgisStyle );

    // clear model
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>( listItems->model() );
    model->clear();
    accept();
  }

  mFileName = "";
  mTempStyle->clear();
}

bool QgsStyleV2ExportImportDialog::populateStyles( QgsStyleV2* style )
{
  // load symbols and color ramps from file
  if ( mDialogMode == Import )
  {
    // NOTE mTempStyle is style here
    if ( !style->importXML( mFileName ) )
    {
      QMessageBox::warning( this, tr( "Import error" ),
                            tr( "An error occurred during import:\n%1" ).arg( style->errorString() ) );
      return false;
    }
  }

  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( listItems->model() );
  model->clear();

  // populate symbols
  QStringList styleNames = style->symbolNames();
  QString name;

  for ( int i = 0; i < styleNames.count(); ++i )
  {
    name = styleNames[i];
    QgsSymbolV2* symbol = style->symbol( name );
    QStandardItem* item = new QStandardItem( name );
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( symbol, listItems->iconSize() );
    item->setIcon( icon );
    model->appendRow( item );
    delete symbol;
  }

  // and color ramps
  styleNames = style->colorRampNames();

  for ( int i = 0; i < styleNames.count(); ++i )
  {
    name = styleNames[i];
    QgsVectorColorRampV2* ramp = style->colorRamp( name );

    QStandardItem* item = new QStandardItem( name );
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( ramp, listItems->iconSize() );
    item->setIcon( icon );
    model->appendRow( item );
    delete ramp;
  }
  return true;
}

void QgsStyleV2ExportImportDialog::moveStyles( QModelIndexList* selection, QgsStyleV2* src, QgsStyleV2* dst )
{
  QString symbolName;
  QgsSymbolV2* symbol;
  QgsVectorColorRampV2 *ramp = nullptr;
  QModelIndex index;
  bool isSymbol = true;
  bool prompt = true;
  bool overwrite = true;
  int groupid = 0;

  // get the groupid when going for import
  if ( mDialogMode == Import )
  {
    // get the name the user entered
    QString name = groupCombo->currentText();
    if ( name.isEmpty() )
    {
      // import to "ungrouped"
      groupid = 0;
    }
    else if ( dst->groupNames().contains( name ) )
    {
      groupid = dst->groupId( name );
    }
    else
    {
      groupid = dst->addGroup( name );
    }
  }

  for ( int i = 0; i < selection->size(); ++i )
  {
    index = selection->at( i );
    symbolName = index.model()->data( index, 0 ).toString();
    symbol = src->symbol( symbolName );
    if ( !symbol )
    {
      isSymbol = false;
      ramp = src->colorRamp( symbolName );
    }

    if ( isSymbol )
    {
      if ( dst->symbolNames().contains( symbolName ) && prompt )
      {
        int res = QMessageBox::warning( this, tr( "Duplicate names" ),
                                        tr( "Symbol with name '%1' already exists.\nOverwrite?" )
                                        .arg( symbolName ),
                                        QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
        switch ( res )
        {
          case QMessageBox::Cancel:
            return;
          case QMessageBox::No:
            continue;
          case QMessageBox::Yes:
            dst->addSymbol( symbolName, symbol );
            if ( mDialogMode == Import )
              dst->saveSymbol( symbolName, symbol, groupid, QStringList() );
            continue;
          case QMessageBox::YesToAll:
            prompt = false;
            overwrite = true;
            break;
          case QMessageBox::NoToAll:
            prompt = false;
            overwrite = false;
            break;
        }
      }

      if ( dst->symbolNames().contains( symbolName ) && overwrite )
      {
        dst->addSymbol( symbolName, symbol );
        if ( mDialogMode == Import )
          dst->saveSymbol( symbolName, symbol, groupid, QStringList() );
      }
      else if ( dst->symbolNames().contains( symbolName ) && !overwrite )
      {
        continue;
      }
      else
      {
        dst->addSymbol( symbolName, symbol );
        if ( mDialogMode == Import )
          dst->saveSymbol( symbolName, symbol, groupid, QStringList() );
      }
    }
    else
    {
      if ( dst->colorRampNames().contains( symbolName ) && prompt )
      {
        int res = QMessageBox::warning( this, tr( "Duplicate names" ),
                                        tr( "Color ramp with name '%1' already exists.\nOverwrite?" )
                                        .arg( symbolName ),
                                        QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
        switch ( res )
        {
          case QMessageBox::Cancel:
            return;
          case QMessageBox::No:
            continue;
          case QMessageBox::Yes:
            dst->addColorRamp( symbolName, ramp );
            if ( mDialogMode == Import )
              dst->saveColorRamp( symbolName, ramp, groupid, QStringList() );
            continue;
          case QMessageBox::YesToAll:
            prompt = false;
            overwrite = true;
            break;
          case QMessageBox::NoToAll:
            prompt = false;
            overwrite = false;
            break;
        }
      }

      if ( dst->colorRampNames().contains( symbolName ) && overwrite )
      {
        dst->addColorRamp( symbolName, ramp );
        if ( mDialogMode == Import )
          dst->saveColorRamp( symbolName, ramp, groupid, QStringList() );
      }
      else if ( dst->colorRampNames().contains( symbolName ) && !overwrite )
      {
        continue;
      }
      else
      {
        dst->addColorRamp( symbolName, ramp );
        if ( mDialogMode == Import )
          dst->saveColorRamp( symbolName, ramp, groupid, QStringList() );
      }
    }
  }
}

QgsStyleV2ExportImportDialog::~QgsStyleV2ExportImportDialog()
{
  delete mTempFile;
  delete mTempStyle;
  delete mGroupSelectionDlg;
}

void QgsStyleV2ExportImportDialog::selectAll()
{
  listItems->selectAll();
}

void QgsStyleV2ExportImportDialog::clearSelection()
{
  listItems->clearSelection();
}

void QgsStyleV2ExportImportDialog::selectSymbols( const QStringList& symbolNames )
{
  Q_FOREACH ( const QString &symbolName, symbolNames )
  {
    QModelIndexList indexes = listItems->model()->match( listItems->model()->index( 0, 0 ), Qt::DisplayRole, symbolName , 1, Qt::MatchFixedString | Qt::MatchCaseSensitive );
    Q_FOREACH ( const QModelIndex &index, indexes )
    {
      listItems->selectionModel()->select( index, QItemSelectionModel::Select );
    }
  }
}

void QgsStyleV2ExportImportDialog::deselectSymbols( const QStringList& symbolNames )
{
  Q_FOREACH ( const QString &symbolName, symbolNames )
  {
    QModelIndexList indexes = listItems->model()->match( listItems->model()->index( 0, 0 ), Qt::DisplayRole, symbolName , 1, Qt::MatchFixedString | Qt::MatchCaseSensitive );
    Q_FOREACH ( const QModelIndex &index, indexes )
    {
      QItemSelection deselection( index, index );
      listItems->selectionModel()->select( deselection, QItemSelectionModel::Deselect );
    }
  }
}

void QgsStyleV2ExportImportDialog::selectGroup( const QString& groupName )
{
  QStringList symbolNames = mQgisStyle->symbolsOfGroup( QgsStyleV2::SymbolEntity, mQgisStyle->groupId( groupName ) );
  selectSymbols( symbolNames );
  symbolNames = mQgisStyle->symbolsOfGroup( QgsStyleV2::ColorrampEntity, mQgisStyle->groupId( groupName ) );
  selectSymbols( symbolNames );
}


void QgsStyleV2ExportImportDialog::deselectGroup( const QString& groupName )
{
  QStringList symbolNames = mQgisStyle->symbolsOfGroup( QgsStyleV2::SymbolEntity, mQgisStyle->groupId( groupName ) );
  deselectSymbols( symbolNames );
  symbolNames = mQgisStyle->symbolsOfGroup( QgsStyleV2::ColorrampEntity, mQgisStyle->groupId( groupName ) );
  deselectSymbols( symbolNames );
}

void QgsStyleV2ExportImportDialog::selectSmartgroup( const QString& groupName )
{
  QStringList symbolNames = mQgisStyle->symbolsOfSmartgroup( QgsStyleV2::SymbolEntity, mQgisStyle->smartgroupId( groupName ) );
  selectSymbols( symbolNames );
  symbolNames = mQgisStyle->symbolsOfSmartgroup( QgsStyleV2::ColorrampEntity, mQgisStyle->smartgroupId( groupName ) );
  selectSymbols( symbolNames );
}

void QgsStyleV2ExportImportDialog::deselectSmartgroup( const QString& groupName )
{
  QStringList symbolNames = mQgisStyle->symbolsOfSmartgroup( QgsStyleV2::SymbolEntity, mQgisStyle->smartgroupId( groupName ) );
  deselectSymbols( symbolNames );
  symbolNames = mQgisStyle->symbolsOfSmartgroup( QgsStyleV2::ColorrampEntity, mQgisStyle->smartgroupId( groupName ) );
  deselectSymbols( symbolNames );
}

void QgsStyleV2ExportImportDialog::selectByGroup()
{
  if ( ! mGroupSelectionDlg )
  {
    mGroupSelectionDlg = new QgsStyleV2GroupSelectionDialog( mQgisStyle, this );
    mGroupSelectionDlg->setWindowTitle( tr( "Select symbols by group" ) );
    connect( mGroupSelectionDlg, SIGNAL( groupSelected( const QString ) ), this, SLOT( selectGroup( const QString ) ) );
    connect( mGroupSelectionDlg, SIGNAL( groupDeselected( const QString ) ), this, SLOT( deselectGroup( const QString ) ) );
    connect( mGroupSelectionDlg, SIGNAL( allSelected() ), this, SLOT( selectAll() ) );
    connect( mGroupSelectionDlg, SIGNAL( allDeselected() ), this, SLOT( clearSelection() ) );
    connect( mGroupSelectionDlg, SIGNAL( smartgroupSelected( const QString ) ), this, SLOT( selectSmartgroup( const QString ) ) );
    connect( mGroupSelectionDlg, SIGNAL( smartgroupDeselected( const QString ) ), this, SLOT( deselectSmartgroup( const QString ) ) );
  }
  mGroupSelectionDlg->show();
  mGroupSelectionDlg->raise();
  mGroupSelectionDlg->activateWindow();
}

void QgsStyleV2ExportImportDialog::importTypeChanged( int index )
{
  QString type = importTypeCombo->itemData( index ).toString();

  locationLineEdit->setText( "" );

  if ( type == "file" )
  {
    locationLineEdit->setEnabled( true );
    btnBrowse->setText( "Browse" );
  }
  else if ( type == "official" )
  {
    btnBrowse->setText( "Fetch Symbols" );
    locationLineEdit->setEnabled( false );
  }
  else
  {
    btnBrowse->setText( "Fetch Symbols" );
    locationLineEdit->setEnabled( true );
  }
}

void QgsStyleV2ExportImportDialog::browse()
{
  QString type = importTypeCombo->itemData( importTypeCombo->currentIndex() ).toString();

  if ( type == "file" )
  {
    mFileName = QFileDialog::getOpenFileName( this, tr( "Load styles" ), QDir::homePath(),
                tr( "XML files (*.xml *XML)" ) );
    if ( mFileName.isEmpty() )
    {
      return;
    }
    QFileInfo pathInfo( mFileName );
    QString groupName = pathInfo.fileName().remove( ".xml" );
    groupCombo->setItemText( 0, groupName );
    locationLineEdit->setText( mFileName );
    populateStyles( mTempStyle );
  }
  else if ( type == "official" )
  {
    // TODO set URL
    // downloadStyleXML( QUrl( "http://...." ) );
  }
  else
  {
    downloadStyleXML( QUrl( locationLineEdit->text() ) );
  }
}

void QgsStyleV2ExportImportDialog::downloadStyleXML( const QUrl& url )
{
  // XXX Try to move this code to some core Network interface,
  // HTTP downloading is a generic functionality that might be used elsewhere

  mTempFile = new QTemporaryFile();
  if ( mTempFile->open() )
  {
    mFileName = mTempFile->fileName();

    if ( mProgressDlg )
    {
      QProgressDialog *dummy = mProgressDlg;
      mProgressDlg = nullptr;
      delete dummy;
    }
    mProgressDlg = new QProgressDialog();
    mProgressDlg->setLabelText( tr( "Downloading style ... " ) );
    mProgressDlg->setAutoClose( true );

    connect( mProgressDlg, SIGNAL( canceled() ), this, SLOT( downloadCanceled() ) );

    // open the network connection and connect the respective slots
    if ( mNetReply )
    {
      QNetworkReply *dummyReply = mNetReply;
      mNetReply = nullptr;
      delete dummyReply;
    }
    mNetReply = mNetManager->get( QNetworkRequest( url ) );

    connect( mNetReply, SIGNAL( finished() ), this, SLOT( httpFinished() ) );
    connect( mNetReply, SIGNAL( readyRead() ), this, SLOT( fileReadyRead() ) );
    connect( mNetReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( updateProgress( qint64, qint64 ) ) );
  }
}

void QgsStyleV2ExportImportDialog::httpFinished()
{
  if ( mNetReply->error() )
  {
    mTempFile->remove();
    mFileName = "";
    mProgressDlg->hide();
    QMessageBox::information( this, tr( "HTTP Error!" ),
                              tr( "Download failed: %1." ).arg( mNetReply->errorString() ) );
    return;
  }
  else
  {
    mTempFile->flush();
    mTempFile->close();
    populateStyles( mTempStyle );
  }
}

void QgsStyleV2ExportImportDialog::fileReadyRead()
{
  mTempFile->write( mNetReply->readAll() );
}

void QgsStyleV2ExportImportDialog::updateProgress( qint64 bytesRead, qint64 bytesTotal )
{
  mProgressDlg->setMaximum( bytesTotal );
  mProgressDlg->setValue( bytesRead );
}

void QgsStyleV2ExportImportDialog::downloadCanceled()
{
  mNetReply->abort();
  mTempFile->remove();
  mFileName = "";
}

void QgsStyleV2ExportImportDialog::selectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  bool nothingSelected = listItems->selectionModel()->selectedIndexes().empty();
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( nothingSelected );
}
