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

/* $Id: qgsstylev2exportimportdialog.cpp 13187 2010-03-28 22:14:44Z jef $ */

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>

#include "qgsstylev2exportimportdialog.h"

#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"

QgsStyleV2ExportImportDialog::QgsStyleV2ExportImportDialog( QgsStyleV2* style,
    QWidget *parent, Mode mode, QString fileName )
    : QDialog( parent ), mDialogMode( mode ), mQgisStyle( style ), mFileName( fileName )
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

  if ( mDialogMode == Import )
  {
    label->setText( tr( "Select symbols to import" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Import" ) );
    if ( !populateStyles( mTempStyle ) )
    {
      QApplication::postEvent( this, new QCloseEvent() );
    }
  }
  else
  {
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
    if ( !mTempStyle->save( mFileName ) )
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
    mQgisStyle->save();

    // clear model
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>( listItems->model() );
    model->clear();
    accept();
  }

  mFileName = "";
  mTempStyle->clear();

  return;
}

bool QgsStyleV2ExportImportDialog::populateStyles( QgsStyleV2* style )
{
  // load symbols and color ramps from file
  if ( mDialogMode == Import )
  {
    if ( !mTempStyle->load( mFileName ) )
    {
      QMessageBox::warning( this, tr( "Import error" ),
                            tr( "An error was occured during import:\n%1" ).arg( mTempStyle->errorString() ) );
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
  QgsVectorColorRampV2* ramp;
  QModelIndex index;
  bool isSymbol = true;
  bool prompt = true;
  bool overwrite = true;

  for( int i = 0; i < selection->size(); ++i )
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
      if ( dst->symbolNames().contains(  symbolName ) && prompt )
      {
        int res = QMessageBox::warning( this, tr( "Duplicate names" ),
                                        tr( "Symbol with name '%1' already exists.\nOverwrite?" )
                                        .arg( symbolName ),
                                        QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
        switch ( res )
        {
          case QMessageBox::Cancel:   return;
          case QMessageBox::No:       continue;
          case QMessageBox::Yes:      dst->addSymbol( symbolName, symbol );
                                      continue;
          case QMessageBox::YesToAll: prompt = false;
                                      overwrite = true;
                                      break;
          case QMessageBox::NoToAll:  prompt = false;
                                      overwrite = false;
                                      break;
        }
      }

      if ( dst->symbolNames().contains(  symbolName ) && overwrite )
      {
        dst->addSymbol( symbolName, symbol );
      }
      else if ( dst->symbolNames().contains(  symbolName ) && !overwrite )
      {
        continue;
      }
      else
      {
        dst->addSymbol( symbolName, symbol );
      }
    }
    else
    {
      if ( dst->colorRampNames().contains(  symbolName ) && prompt )
      {
        int res = QMessageBox::warning( this, tr( "Duplicate names" ),
                                        tr( "Color ramp with name '%1' already exists.\nOverwrite?" )
                                        .arg( symbolName ),
                                        QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
        switch ( res )
        {
          case QMessageBox::Cancel:   return;
          case QMessageBox::No:       continue;
          case QMessageBox::Yes:      dst->addColorRamp( symbolName, ramp );
                                      continue;
          case QMessageBox::YesToAll: prompt = false;
                                      overwrite = true;
                                      break;
          case QMessageBox::NoToAll:  prompt = false;
                                      overwrite = false;
                                      break;
        }
      }

      if ( dst->colorRampNames().contains(  symbolName ) && overwrite )
      {
        dst->addColorRamp( symbolName, ramp );
      }
      else if ( dst->colorRampNames().contains(  symbolName ) && !overwrite )
      {
        continue;
      }
      else
      {
        dst->addColorRamp( symbolName, ramp );
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
