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

#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgslogger.h"

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

  mTempStyle = new QgsStyleV2();
  // TODO validate
  mFileName = "";
  mProgressDlg = NULL;
  mTempFile = NULL;
  mNetManager = new QNetworkAccessManager( this );
  mNetReply = NULL;

  if ( mDialogMode == Import )
  {
    // populate the import types
    importTypeCombo->addItem( "file specified below", QVariant( "file" ) );
    importTypeCombo->addItem( "official QGIS repo online", QVariant( "official" ) );
    importTypeCombo->addItem( "URL specified below", QVariant( "url" ) );
    connect( importTypeCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( importTypeChanged( int ) ) );

    btnBrowse->setText( "Browse" );
    connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( browse() ) );

    label->setText( tr( "Select symbols to import" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Import" ) );
    
  }
  else
  {
    // hide import specific controls when exporting
    btnBrowse->setHidden( true );
    fromLabel->setHidden( true );
    importTypeCombo->setHidden( true );
    locationLabel->setHidden( true );
    locationLineEdit->setHidden( true );

    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Export" ) );
    if ( !populateStyles( mQgisStyle ) )
    {
      QApplication::postEvent( this, new QCloseEvent() );
    }
  }

  // use Ok button for starting import and export operations
  disconnect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( doExportImport() ) );
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
    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save styles" ), ".",
                       tr( "XML files (*.xml *.XML)" ) );
    if ( fileName.isEmpty() )
    {
      return;
    }

    // ensure the user never ommited the extension from the file name
    if ( !fileName.toLower().endsWith( ".xml" ) )
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
                            tr( "An error occured during import:\n%1" ).arg( style->errorString() ) );
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
  QgsVectorColorRampV2 *ramp = 0;
  QModelIndex index;
  bool isSymbol = true;
  bool prompt = true;
  bool overwrite = true;

  for ( int i = 0; i < selection->size(); ++i )
  {
    index = selection->at( i );
    symbolName = index.model()->data( index, 0 ).toString();
    symbol = src->symbol( symbolName );
    if ( symbol == NULL )
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
              dst->saveSymbol( symbolName, symbol, 0, QStringList() );
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
          dst->saveSymbol( symbolName, symbol, 0, QStringList() );
      }
      else if ( dst->symbolNames().contains( symbolName ) && !overwrite )
      {
        continue;
      }
      else
      {
        dst->addSymbol( symbolName, symbol );
        if ( mDialogMode == Import )
          dst->saveSymbol( symbolName, symbol, 0, QStringList() );
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
              dst->saveColorRamp( symbolName, ramp, 0, QStringList() );
            continue;
          case QMessageBox::YesToAll:
            prompt = false;
            overwrite = true;
            break;
          case QMessageBox::NoToAll:  prompt = false;
            overwrite = false;
            break;
        }
      }

      if ( dst->colorRampNames().contains( symbolName ) && overwrite )
      {
        dst->addColorRamp( symbolName, ramp );
        if ( mDialogMode == Import )
          dst->saveColorRamp( symbolName, ramp, 0, QStringList() );
      }
      else if ( dst->colorRampNames().contains( symbolName ) && !overwrite )
      {
        continue;
      }
      else
      {
        dst->addColorRamp( symbolName, ramp );
        if ( mDialogMode == Import )
          dst->saveColorRamp( symbolName, ramp, 0, QStringList() );
      }
    }
  }
}

QgsStyleV2ExportImportDialog::~QgsStyleV2ExportImportDialog()
{
  delete mTempStyle;
}

void QgsStyleV2ExportImportDialog::selectAll()
{
  listItems->selectAll();
}

void QgsStyleV2ExportImportDialog::clearSelection()
{
  listItems->clearSelection();
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
    mFileName = QFileDialog::getOpenFileName( this, tr( "Load styles" ), ".",
                     tr( "XML files (*.xml *XML)" ) );
    if ( mFileName.isEmpty() )
    {
      return;
    }
    locationLineEdit->setText( mFileName );
    populateStyles( mTempStyle );
  }
  else if ( type == "official" )
  {
    // TODO set URL
    // downloadStyleXML( QUrl( "http://...." ) );
    QMessageBox::warning( this, tr( "Invalid Selection" ),
        tr( "Sorry! The official QGIS repository has not been implemented. You cannot use this feature now." ) );
  }
  else
  {
    downloadStyleXML( QUrl( locationLineEdit->text() ) );
  }
}

void QgsStyleV2ExportImportDialog::downloadStyleXML( QUrl url )
{
    // TODO Try to move this code to some core Network interface,
    // HTTP downloading is a generic functionality that might be used elsewhere

  mTempFile = new QTemporaryFile();
  if ( mTempFile->open() )
  {
    mFileName = mTempFile->fileName();

    if ( mProgressDlg )
    {
      QProgressDialog *dummy = mProgressDlg;
      mProgressDlg = NULL;
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
      mNetReply = NULL;
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
    delete mTempFile;
    mTempFile = NULL;
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
