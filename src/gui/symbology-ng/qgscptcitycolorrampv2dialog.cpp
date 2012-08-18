/***************************************************************************
    qgscptcitycolorrampv2dialog.cpp
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscptcitycolorrampv2dialog.h"

#include "qgscptcitybrowsermodel.h"
#include "qgsvectorcolorrampv2.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsdialog.h"

#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QErrorMessage>

/////////

/*
TODO

- try to keep file reads at a minimum -> when selecting an item in the browser copy its ramp over to mRamp
- collapse all button
- show information from dir when selected
- when browsing by selections, remove dir from scheme name
- show type when there are variants - based on the first file

- selections:
  - shorten child names
  - when collection has only 1 item (e.g. es_skywalker), bring up one level

 */
QgsCptCityColorRampV2Dialog::QgsCptCityColorRampV2Dialog( QgsCptCityColorRampV2* ramp, QWidget* parent )
    : QDialog( parent ), mRamp( ramp )
{
  setupUi( this );
  mAuthorsModel = mSelectionsModel = 0;
}


void QgsCptCityColorRampV2Dialog::populateVariants( QString newVariant )
{
  QStringList variantList;
  QgsCptCityColorRampItem *item =
    dynamic_cast< QgsCptCityColorRampItem* >( mModel->dataItem( mBrowserView->currentIndex() ) );
  if ( item )
    variantList = item->ramp().variantList();

  cboVariantName->blockSignals( true );
  cboVariantName->clear();

  if ( variantList.isEmpty() )
  {
    cboVariantName->setEnabled( false );
    cboVariantName->setVisible( false );
    on_cboVariantName_currentIndexChanged( -1 );
  }
  else
  {
    QString oldVariant = cboVariantName->currentText();
    QgsCptCityColorRampV2 ramp( mRamp->schemeName(), QString() );
    QPixmap blankPixmap( cboVariantName->iconSize() );
    blankPixmap.fill( Qt::white );
    QIcon blankIcon( blankPixmap );
    int index;

    foreach ( QString variant, variantList )
    {
      QString variantStr = variant;
      if ( variantStr.startsWith( "-" ) || variantStr.startsWith( "_" ) )
        variantStr.remove( 0, 1 );
      cboVariantName->addItem( " " + variantStr );
      index = cboVariantName->count() - 1;
      cboVariantName->setItemData( index, variant, Qt::UserRole );

      ramp.setVariantName( variant );
      if ( ramp.loadFile() )
        cboVariantName->setItemIcon( index,
                                     QgsSymbolLayerV2Utils::colorRampPreviewIcon( &ramp, cboVariantName->iconSize() ) );
      else
        cboVariantName->setItemIcon( index, blankIcon );
      cboVariantName->setItemData( index, Qt::AlignHCenter, Qt::TextAlignmentRole );
    }

    cboVariantName->blockSignals( false );
    cboVariantName->setEnabled( true );
    cboVariantName->setVisible( true );

    // try to set the original variant again (if exists)
    int idx;
    if ( newVariant != QString() )
    {
      if ( newVariant.startsWith( "-" ) || newVariant.startsWith( "_" ) )
        newVariant.remove( 0, 1 );
      newVariant = " " + newVariant;
      idx = cboVariantName->findText( newVariant );
    }
    else
      idx = cboVariantName->findText( oldVariant );

    // if not found use the item in the middle
    if ( idx == -1 )
    {
      idx = cboVariantName->count() / 2;
    }
    cboVariantName->setCurrentIndex( idx );
    // updatePreview();
  }

}

void QgsCptCityColorRampV2Dialog::on_mBrowserView_clicked( const QModelIndex &index )
{
  QgsCptCityDataItem *item = mModel->dataItem( index );
  if ( ! item )
    return;
  if ( item->type() == QgsCptCityDataItem::ColorRamp )
  {
    lblSchemeName->setText( item->name() );
    mRamp->setSchemeName( item->path() );
    populateVariants();
  }
  else if ( item->type() == QgsCptCityDataItem::Directory )
  {
    mRamp->setName( "", "" );
    // lblSchemeName->setText( "" );
    populateVariants();
    lblSchemePath->setText( item->path() );
    updateCopyingInfo( mCollection->copyingInfo( mCollection->copyingFileName( item->path() ) ) );
  }
}

void QgsCptCityColorRampV2Dialog::on_tabBar_currentChanged( int index )
{
  if ( index == 0 )
  {
    mCollectionGroup = "selections";
    mModel = mSelectionsModel;
  }
  else if ( index == 1 )
  {
    mModel = mAuthorsModel;
    mCollectionGroup = "authors";
  }
  else
  {
    QgsDebugMsg( QString( "invalid index %1" ).arg( index ) );
    mModel = mAuthorsModel;
    mCollectionGroup = "authors";
  }

  // set model
  mBrowserView->setModel( mModel );

  // try to apply selection to view
  // TODO fix ssearching when item has variants (e.g. ds/rgi/rgi_15)
  QModelIndex modelIndex = mModel->findPath( mRamp->schemeName() );
  if ( modelIndex != QModelIndex() )
  {
    mBrowserView->setCurrentIndex( modelIndex );
    mBrowserView->scrollTo( modelIndex, QAbstractItemView::PositionAtCenter );
    on_mBrowserView_clicked( modelIndex );
  }
}


void QgsCptCityColorRampV2Dialog::on_pbtnLicenseDetails_pressed()
{
  QString path, title, copyFile, descFile;

  // get basic information, depending on if is color ramp or directory
  QgsCptCityDataItem *item =
    dynamic_cast< QgsCptCityDataItem* >( mModel->dataItem( mBrowserView->currentIndex() ) );
  if ( item && item->type() == QgsCptCityDataItem::Directory )
  {
    path = item->path();
    title = tr( "%1 directory details" ).arg( item->name() );
    copyFile = mCollection->copyingFileName( path );
    descFile = mCollection->descFileName( path );
  }
  else if ( item )
  {
    path = mRamp->schemeName() + mRamp->variantName();
    title = tr( "%1 gradient details" ).arg( path );
    copyFile = mRamp->copyingFileName();
    descFile = mRamp->descFileName();
  }
  else
  {
    return;
  }

  // prepare dialog
  QgsDialog dlg( this, 0, QDialogButtonBox::Close );
  QVBoxLayout *layout = dlg.layout();
  dlg.setWindowTitle( title );
  QTextEdit *textEdit = new QTextEdit( &dlg );
  textEdit->setReadOnly( true );
  layout->addWidget( textEdit );

  // add contents of DESC.xml and COPYING.xml
  QString copyText;
  if ( ! copyFile.isNull() )
  {
    QFile file( copyFile );
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      copyText = QString( file.readAll() );
      file.close();
    }
  }
  QString descText;
  if ( ! descFile.isNull() )
  {
    QFile file( descFile );
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      descText = QString( file.readAll() );
      file.close();
    }
  }
  textEdit->insertPlainText( title + "\n\n" );
  textEdit->insertPlainText( "===================" );
  textEdit->insertPlainText( " copying " );
  textEdit->insertPlainText( "===================\n" );
  textEdit->insertPlainText( copyText );
  textEdit->insertPlainText( "\n" );
  textEdit->insertPlainText( "==================" );
  textEdit->insertPlainText( " description " );
  textEdit->insertPlainText( "==================\n" );
  textEdit->insertPlainText( descText );
  textEdit->moveCursor( QTextCursor::Start );

  dlg.resize( 600, 600 );
  dlg.exec();
}

void QgsCptCityColorRampV2Dialog::updatePreview()
{
  QSize size( 300, 50 );

  if ( mRamp->schemeName() == "" )
  {
    lblSchemeName->setText( "" );
    lblSchemePath->setText( "" );
    QPixmap blankPixmap( size );
    blankPixmap.fill( Qt::transparent );
    lblPreview->setPixmap( blankPixmap );
    return;
  }

  mRamp->loadFile();

  lblSchemePath->setText( mRamp->schemeName() + mRamp->variantName() );

  // update pixmap
  // TODO draw checker-board/transparent background
  // for transparent, add  [ pixmap.fill( Qt::transparent ); ] to QgsSymbolLayerV2Utils::colorRampPreviewPixmap
  QPixmap pixmap = QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size );
  lblPreview->setPixmap( pixmap );

  // add copyright information from COPYING.xml file
  updateCopyingInfo( mRamp->copyingInfo() );
}

void QgsCptCityColorRampV2Dialog::updateCopyingInfo( const QMap< QString, QString >& copyingMap )
{
  QString authorStr = copyingMap.value( "authors" );
  if ( authorStr.length() > 80 )
    authorStr.replace( authorStr.indexOf( " ", 80 ), 1, "\n" );
  lblAuthorName->setText( authorStr );
  QString licenseStr = copyingMap.value( "license/informal" );
  if ( copyingMap.contains( "license/year" ) )
    licenseStr += " (" + copyingMap.value( "license/year" ) + ")";
  if ( licenseStr.length() > 80 )
    licenseStr.replace( licenseStr.indexOf( " ", 80 ), 1, "\n" );
  if ( copyingMap.contains( "license/url" ) )
    licenseStr += "\n[ " + copyingMap.value( "license/url" ) + " ]";
  else
    licenseStr += "\n";
  lblLicenseName->setText( licenseStr );
  if ( copyingMap.contains( "src/link" ) )
    lblSrcLink->setText( copyingMap.value( "src/link" ) );
  else
    lblSrcLink->setText( "" );
}

void QgsCptCityColorRampV2Dialog::on_cboVariantName_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  mRamp->setVariantName( cboVariantName->itemData( cboVariantName->currentIndex(), Qt::UserRole ).toString() );
  updatePreview();
}


// this function if for a svg preview, available if the svg files have been processed with svgx
// e.g. for f in `ls */*/*/*/*.svg`; do echo $f ; svgx -p -t svg $f > tmp1.svg; mv tmp1.svg $f; done
// perhaps a future version of the cpt-city svg gradients will have them by default
bool QgsCptCityColorRampV2Dialog::eventFilter( QObject *obj, QEvent *event )
{
  QSize size( 300, 50 );
  if ( event->type() == QEvent::MouseButtonPress )
  {
    // create preview from svg file if supported - depends on file versions
    QPixmap pixmap( mRamp->fileName() );
    if ( ! pixmap.isNull() )
      lblPreview->setPixmap( pixmap.scaled( size ) );
    return true;
  }
  else if ( event->type() == QEvent::MouseButtonRelease )
  {
    // restore preview
    QPixmap pixmap = QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size );
    lblPreview->setPixmap( pixmap );
    return true;
  }
  else
  {
    // standard event processing
    return QObject::eventFilter( obj, event );
  }
}

// delay initialization and update collection if it has changed
void QgsCptCityColorRampV2Dialog::showEvent( QShowEvent * e )
{
  // setup collections
  if ( QgsCptCityCollection::collectionRegistry().isEmpty() )
  {
    QgsCptCityCollection::initCollections( true );
  }
  mCollection = QgsCptCityCollection::defaultCollection();
  // if empty collection, try loading again - this may happen after installing new package
  if ( ! mCollection || mCollection->isEmpty() )
  {
    QgsCptCityCollection::initCollections( true );
    mCollection = QgsCptCityCollection::defaultCollection();
  }

  // show information on how to install cpt-city files if none are found
  if ( ! mCollection || mCollection->isEmpty() )
  {
    // QgsDialog dlg( this );
    // dlg.setWindowTitle( tr( "cpt-city gradient files not found" ) );
    QTextEdit *edit = new QTextEdit( 0 );
    edit->setReadOnly( true );
    // not sure if we want this long string to be translated
    QString helpText = tr( "Error - cpt-city gradient files not found.\n\n"
                           "You have two means of installing them:\n\n"
                           "1) Install the \"Color Ramp Manager\" python plugin "
                           "(you must enable Experimental plugins in the plugin manager) "
                           "and use it to download latest cpt-city package.\n"
                           "You can install the entire cpt-city archive or a selection for QGIS.\n\n"
                           "2) Download the complete collection (in svg format) "
                           "and unzip it to your QGis settings directory [%1] .\n\n"
                           "This file can be found at [%2]\nand current file is [%3]"
                         ).arg( QgsApplication::qgisSettingsDirPath()
                              ).arg( "http://soliton.vm.bytemark.co.uk/pub/cpt-city/pkg/"
                                   ).arg( "http://soliton.vm.bytemark.co.uk/pub/cpt-city/pkg/cpt-city-svg-2.02.zip" );
    edit->setText( helpText );
    stackedWidget->addWidget( edit );
    stackedWidget->setCurrentIndex( 1 );
    tabBar->setVisible( false );
    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    // dlg.layout()->addWidget( edit );
    // dlg.resize(500,400);
    // dlg.exec();
    return;
  }

  if ( ! mCollection )
    return;
  QgsDebugMsg( "collection: " + mCollection->collectionName() );

  // model / view
  QgsDebugMsg( "loading model/view objects" );
  if ( mAuthorsModel )
    delete mAuthorsModel;
  mAuthorsModel = new QgsCptCityBrowserModel( mBrowserView, mCollection, "authors" );
  if ( mSelectionsModel )
    delete mSelectionsModel;
  mSelectionsModel = new QgsCptCityBrowserModel( mBrowserView, mCollection, "selections" );
  mModel = mSelectionsModel;
  mBrowserView->setModel( mModel );
  mBrowserView->setSelectionMode( QAbstractItemView::SingleSelection );
  mBrowserView->setIconSize( QSize( 100, 15 ) );
  // provide a horizontal scroll bar instead of using ellipse (...) for longer items
  // mBrowserView->setTextElideMode( Qt::ElideNone );
  // mBrowserView->header()->setResizeMode( 0, QHeaderView::ResizeToContents );
  mBrowserView->header()->resizeSection( 0, 250 );
  mBrowserView->header()->setStretchLastSection( true );

  // setup ui
  tabBar->blockSignals( true );
  tabBar->addTab( tr( "Selections by theme" ) );
  tabBar->addTab( tr( "All by author" ) );
  cboVariantName->setIconSize( QSize( 100, 15 ) );
  lblPreview->installEventFilter( this ); // mouse click on preview label shows svg render

  // populate tree widget - if item not found in selections collection, look for in authors
  // try to apply selection to view
  QModelIndex modelIndex = mModel->findPath( mRamp->schemeName() );
  if ( modelIndex == QModelIndex() )
  {
    modelIndex = mAuthorsModel->findPath( mRamp->schemeName() );
    if ( modelIndex != QModelIndex() )
    {
      tabBar->setCurrentIndex( 1 );
      mModel = mAuthorsModel;
      mBrowserView->setModel( mModel );
    }
  }
  if ( modelIndex != QModelIndex() )
  {
    lblSchemeName->setText( mRamp->schemeName() );
    mBrowserView->setCurrentIndex( modelIndex );
    mBrowserView->scrollTo( modelIndex, QAbstractItemView::PositionAtCenter );
    populateVariants( mRamp->variantName() );
    // updatePreview();
  }
  tabBar->blockSignals( false );
  if ( mCollection->collectionName() == DEFAULT_CPTCITY_COLLECTION )
    tabBar->setCurrentIndex( 1 );

  QDialog::showEvent( e );

  // show error message to use color ramp manager to get more gradients
  if ( mCollection->collectionName() == DEFAULT_CPTCITY_COLLECTION &&
       QgsCptCityCollection::collectionRegistry().count() == 1 )
  {
    QString helpText = tr( "You can download a more complete set of cpt-city gradients "
                           "by installing the \"Color Ramp Manager\" plugin "
                           "(you must enable Experimental plugins in the plugin manager).\n\n"
                         );
    QErrorMessage* msg = new QErrorMessage( this );
    msg->showMessage( helpText, "cpt-city" );
  }
}

#if 0
void QgsCptCityColorRampV2Dialog::refresh()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  refreshModel( QModelIndex() );
  QApplication::restoreOverrideCursor();
}

void QgsCptCityColorRampV2Dialog::refreshModel( const QModelIndex& index )
{
  if ( index.isValid() )
  {
    QgsCptCityDataItem *item = mModel->dataItem( index );
    if ( item )
    {
      QgsDebugMsg( "path = " + item->path() );
    }
    else
    {
      QgsDebugMsg( "invalid item" );
    }
  }

  mModel->refresh( index );

  for ( int i = 0 ; i < mModel->rowCount( index ); i++ )
  {
    QModelIndex idx = mModel->index( i, 0, index );
    if ( mBrowserView->isExpanded( idx ) || !mModel->hasChildren( idx ) )
    {
      refreshModel( idx );
    }
  }
}
#endif
