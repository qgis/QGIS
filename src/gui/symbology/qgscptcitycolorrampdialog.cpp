/***************************************************************************
    qgscptcitycolorrampdialog.cpp
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

#include "qgscptcitycolorrampdialog.h"

#include "qgscptcityarchive.h"
#include "qgscolorramp.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsdialog.h"
#include "qgssymbollayerutils.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QMessageBox>
#include <QSortFilterProxyModel>

/////////

// TODO
// - fix Diverging children when first show Selections
// - fix crash on Diverging?


QgsCptCityColorRampDialog::QgsCptCityColorRampDialog( const QgsCptCityColorRamp &ramp, QWidget *parent )
  : QDialog( parent )
  , mRamp( ramp )
  , mArchiveViewType( QgsCptCityBrowserModel::Selections )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  connect( mTreeView, &QTreeView::clicked, this, &QgsCptCityColorRampDialog::mTreeView_clicked );
  connect( mListWidget, &QListWidget::itemClicked, this, &QgsCptCityColorRampDialog::mListWidget_itemClicked );
  connect( mListWidget, &QListWidget::itemSelectionChanged, this, &QgsCptCityColorRampDialog::mListWidget_itemSelectionChanged );
  connect( tabBar, &QTabBar::currentChanged, this, &QgsCptCityColorRampDialog::tabBar_currentChanged );
  connect( pbtnLicenseDetails, &QToolButton::pressed, this, &QgsCptCityColorRampDialog::pbtnLicenseDetails_pressed );
  connect( cboVariantName, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsCptCityColorRampDialog::cboVariantName_currentIndexChanged );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsCptCityColorRampDialog::showHelp );

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  const QgsSettings settings;
  mSplitter->setSizes( QList<int>() << 250 << 550 );
  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/CptCityColorRampV2Dialog/splitter" ) ).toByteArray() );

  mModel = mAuthorsModel = mSelectionsModel = nullptr; //mListModel = 0;
  mTreeFilter = nullptr;

  QgsCptCityArchive::initDefaultArchive();
  mArchive = QgsCptCityArchive::defaultArchive();

  // show information on how to install cpt-city files if none are found
  if ( ! mArchive || mArchive->isEmpty() )
  {
    // QgsDialog dlg( this );
    // dlg.setWindowTitle( tr( "Cpt-city Gradient Files Not Found" ) );
    QTextEdit *edit = new QTextEdit( nullptr );
    edit->setReadOnly( true );
    // not sure if we want this long string to be translated
    const QString helpText = tr( "Error - cpt-city gradient files not found.\n\n"
                                 "You have two means of installing them:\n\n"
                                 "1) Install the \"Color Ramp Manager\" python plugin "
                                 "(you must enable Experimental plugins in the plugin manager) "
                                 "and use it to download latest cpt-city package.\n"
                                 "You can install the entire cpt-city archive or a selection for QGIS.\n\n"
                                 "2) Download the complete archive (in svg format) "
                                 "and unzip it to your QGIS settings directory [%1] .\n\n"
                                 "This file can be found at [%2]\nand current file is [%3]"
                               ).arg( QgsApplication::qgisSettingsDirPath(),
                                      QStringLiteral( "http://soliton.vm.bytemark.co.uk/pub/cpt-city/pkg/" ),
                                      QStringLiteral( "http://soliton.vm.bytemark.co.uk/pub/cpt-city/pkg/cpt-city-svg-2.07.zip" ) );
    edit->setText( helpText );
    mStackedWidget->addWidget( edit );
    mStackedWidget->setCurrentIndex( 1 );
    tabBar->setVisible( false );
    // dlg.layout()->addWidget( edit );
    // dlg.resize(500,400);
    // dlg.exec();
    return;
  }

  if ( ! mArchive )
    return;
  QgsDebugMsg( "archive: " + mArchive->archiveName() );

  QgsDebugMsg( QStringLiteral( "ramp name= %1 variant= %2 - %3 variants" ).arg( mRamp.schemeName(), mRamp.variantName() ).arg( mRamp.variantList().count() ) );

  // model / view
  QgsDebugMsg( QStringLiteral( "loading model/view objects" ) );

  delete mAuthorsModel;
  mAuthorsModel = new QgsCptCityBrowserModel( this, mArchive, QgsCptCityBrowserModel::Authors );

  delete mSelectionsModel;
  mSelectionsModel = new QgsCptCityBrowserModel( this, mArchive, QgsCptCityBrowserModel::Selections );
  setTreeModel( mSelectionsModel );

  mTreeView->setSelectionMode( QAbstractItemView::SingleSelection );
  mTreeView->setColumnHidden( 1, true );
  QgsDebugMsg( QStringLiteral( "done loading model/view objects" ) );

  // setup ui
  tabBar->blockSignals( true );
  tabBar->addTab( tr( "Selections by theme" ) );
  tabBar->addTab( tr( "All by author" ) );
  cboVariantName->setIconSize( QSize( 100, 15 ) );
  lblPreview->installEventFilter( this ); // mouse click on preview label shows svg render

  updateUi();

  tabBar->blockSignals( false );

  connect( this, &QDialog::finished, this, &QgsCptCityColorRampDialog::onFinished );

}

void QgsCptCityColorRampDialog::setRamp( const QgsCptCityColorRamp &ramp )
{
  mRamp = ramp;
  updateUi();
}

void QgsCptCityColorRampDialog::populateVariants()
{
  const QStringList variantList = mRamp.variantList();

  QgsDebugMsg( QStringLiteral( "ramp %1%2 has %3 variants" ).arg( mRamp.schemeName(), mRamp.variantName() ).arg( variantList.count() ) );

  cboVariantName->blockSignals( true );
  cboVariantName->clear();

  if ( variantList.isEmpty() )
  {
    cboVariantName->setEnabled( false );
    cboVariantName->setVisible( false );
    cboVariantName_currentIndexChanged( -1 );
  }
  else
  {
    // populate variant combobox
    const QString oldVariant = cboVariantName->currentText();
    QgsCptCityColorRamp ramp( mRamp.schemeName(), mRamp.variantList(), QString() );
    QPixmap blankPixmap( cboVariantName->iconSize() );
    blankPixmap.fill( Qt::white );
    const QIcon blankIcon( blankPixmap );
    int index;

    const auto constVariantList = variantList;
    for ( const QString &variant : constVariantList )
    {
      QString variantStr = variant;
      if ( variantStr.startsWith( '-' ) || variantStr.startsWith( '_' ) )
        variantStr.remove( 0, 1 );
      cboVariantName->addItem( ' ' + variantStr );
      index = cboVariantName->count() - 1;
      cboVariantName->setItemData( index, variant, Qt::UserRole );

      ramp.setVariantName( variant );
      if ( ramp.loadFile() )
        cboVariantName->setItemIcon( index,
                                     QgsSymbolLayerUtils::colorRampPreviewIcon( &ramp, cboVariantName->iconSize() ) );
      else
        cboVariantName->setItemIcon( index, blankIcon );
      cboVariantName->setItemData( index, Qt::AlignHCenter, Qt::TextAlignmentRole );
    }

    cboVariantName->blockSignals( false );

    // try to set the original variant again (if exists)
    int idx = -1;
    QString newVariant = mRamp.variantName();
    QgsDebugMsg( QStringLiteral( "variant= %1 - %2 variants" ).arg( mRamp.variantName() ).arg( mRamp.variantList().count() ) );
    if ( newVariant != QString() )
    {
      if ( newVariant.startsWith( '-' ) || newVariant.startsWith( '_' ) )
        newVariant.remove( 0, 1 );
      newVariant = ' ' + newVariant;
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

    cboVariantName->setEnabled( true );
    cboVariantName->setVisible( true );
  }

}

void QgsCptCityColorRampDialog::mTreeView_clicked( const QModelIndex &index )
{
  const QModelIndex &sourceIndex = mTreeFilter->mapToSource( index );
  QgsCptCityDataItem *item = mModel->dataItem( sourceIndex );
  if ( ! item )
    return;
  QgsDebugMsg( QStringLiteral( "item %1 clicked" ).arg( item->name() ) );
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  updateTreeView( item );
}

void QgsCptCityColorRampDialog::updateTreeView( QgsCptCityDataItem *item, bool resetRamp )
{
  if ( ! item )
  {
    QgsDebugMsg( QStringLiteral( "invalid item" ) );
    return;
  }
  if ( item->type() == QgsCptCityDataItem::Directory )
  {
    if ( resetRamp )
    {
      mRamp.setName( QString(), QString() );
      QgsDebugMsg( QStringLiteral( "variant= %1 - %2 variants" ).arg( mRamp.variantName() ).arg( mRamp.variantList().count() ) );
      lblSchemeName->clear();
      populateVariants();
    }
    updateListWidget( item );
    lblSchemePath->setText( item->path() );
    lblCollectionInfo->setText( QStringLiteral( "%1 (%2)" ).arg( item->info() ).arg( item->leafCount() ) );
    updateCopyingInfo( QgsCptCityArchive::copyingInfo( mArchive->copyingFileName( item->path() ) ) );
  }
  else if ( item->type() == QgsCptCityDataItem::Selection )
  {
    lblSchemePath->clear();
    clearCopyingInfo();
    updateListWidget( item );
    lblCollectionInfo->setText( QStringLiteral( "%1 (%2)" ).arg( item->info() ).arg( item->leafCount() ) );
  }
  else if ( item->type() == QgsCptCityDataItem::AllRamps )
  {
    lblSchemePath->clear();
    clearCopyingInfo();
    updateListWidget( item );
    lblCollectionInfo->setText( tr( "All Ramps (%1)" ).arg( item->leafCount() ) );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "item %1 has invalid type %2" ).arg( item->path() ).arg( static_cast<int>( item->type() ) ) );
  }
}

void QgsCptCityColorRampDialog::mListWidget_itemClicked( QListWidgetItem *item )
{
  QgsCptCityColorRampItem *rampItem = mListRamps.at( item->data( Qt::UserRole ).toInt() );
  if ( rampItem )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
    lblSchemeName->setText( QFileInfo( rampItem->name() ).fileName() );
    mRamp.copy( &rampItem->ramp() );
    QgsDebugMsg( QStringLiteral( "variant= %1 - %2 variants" ).arg( mRamp.variantName() ).arg( mRamp.variantList().count() ) );
    populateVariants();
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "invalid item" ) );
  }
}

void QgsCptCityColorRampDialog::mListWidget_itemSelectionChanged()
{
  if ( mListWidget->selectedItems().isEmpty() )
  {
    mRamp.setName( QString(), QString() );
  }
}

void QgsCptCityColorRampDialog::tabBar_currentChanged( int index )
{
  if ( index == 0 )
  {
    setTreeModel( mSelectionsModel );
    mArchiveViewType = QgsCptCityBrowserModel::Selections;
  }
  else if ( index == 1 )
  {
    setTreeModel( mAuthorsModel );
    mArchiveViewType = QgsCptCityBrowserModel::Authors;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "invalid index %1" ).arg( index ) );
    setTreeModel( mAuthorsModel );
    mArchiveViewType = QgsCptCityBrowserModel::Authors;
  }

  mListWidget->blockSignals( true );
  updateRamp();
  mListWidget->blockSignals( false );
}


void QgsCptCityColorRampDialog::pbtnLicenseDetails_pressed()
{
  QString path, title, copyFile, descFile;

  // get basic information, depending on if is color ramp or directory
  QgsCptCityDataItem *item = mModel->dataItem( mTreeFilter->mapToSource( mTreeView->currentIndex() ) );
  if ( ! item )
    return;

  path = item->path();
  if ( item->type() == QgsCptCityDataItem::Directory )
  {
    title = tr( "%1 Directory Details" ).arg( item->path() );
  }
  else if ( item->type() == QgsCptCityColorRampItem::Directory )
  {
    title = tr( "%1 Gradient Details" ).arg( path );
  }
  else
  {
    return;
  }
  copyFile = mArchive->copyingFileName( path );
  descFile = mArchive->descFileName( path );

  // prepare dialog
  QgsDialog dlg( this, Qt::WindowFlags(), QDialogButtonBox::Close );
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
  textEdit->insertPlainText( QStringLiteral( "===================" ) );
  textEdit->insertPlainText( QStringLiteral( " copying " ) );
  textEdit->insertPlainText( QStringLiteral( "===================\n" ) );
  textEdit->insertPlainText( copyText );
  textEdit->insertPlainText( QStringLiteral( "\n" ) );
  textEdit->insertPlainText( QStringLiteral( "==================" ) );
  textEdit->insertPlainText( QStringLiteral( " description " ) );
  textEdit->insertPlainText( QStringLiteral( "==================\n" ) );
  textEdit->insertPlainText( descText );
  textEdit->moveCursor( QTextCursor::Start );

  dlg.resize( 600, 600 );
  dlg.exec();
}

void QgsCptCityColorRampDialog::updatePreview( bool clear )
{
  const QSize size = lblPreview->size();

  if ( clear || mRamp.schemeName().isEmpty() )
  {
    lblSchemeName->clear();
    lblSchemePath->clear();
    lblLicensePreview->clear();
    QPixmap blankPixmap( size );
    blankPixmap.fill( Qt::transparent );
    lblPreview->setPixmap( blankPixmap );
    return;
  }

  mRamp.loadFile();

  lblSchemePath->setText( mRamp.schemeName() + mRamp.variantName() );

  // update pixmap
  // TODO draw checker-board/transparent background
  // for transparent, add  [ pixmap.fill( Qt::transparent ); ] to QgsSymbolLayerUtils::colorRampPreviewPixmap
  const QPixmap pixmap = QgsSymbolLayerUtils::colorRampPreviewPixmap( &mRamp, size );
  lblPreview->setPixmap( pixmap );

  // add copyright information from COPYING.xml file
  updateCopyingInfo( mRamp.copyingInfo() );
}

void QgsCptCityColorRampDialog::clearCopyingInfo()
{
  updateCopyingInfo( QMap< QString, QString >() );
}

void QgsCptCityColorRampDialog::updateCopyingInfo( const QMap< QString, QString > &copyingMap )
{
  QString authorStr = copyingMap.value( QStringLiteral( "authors" ) );
  if ( authorStr.length() > 80 )
    authorStr.replace( authorStr.indexOf( ' ', 80 ), 1, QStringLiteral( "\n" ) );
  lblAuthorName->setText( authorStr );
  QString licenseStr = copyingMap.value( QStringLiteral( "license/informal" ) );
  if ( copyingMap.contains( QStringLiteral( "license/year" ) ) )
    licenseStr += " (" + copyingMap.value( QStringLiteral( "license/year" ) ) + ')';
  if ( licenseStr.length() > 80 )
    licenseStr.replace( licenseStr.indexOf( ' ', 80 ), 1, QStringLiteral( "\n" ) );
  if ( copyingMap.contains( QStringLiteral( "license/url" ) ) )
    licenseStr += "\n[ " + copyingMap.value( QStringLiteral( "license/url" ) ) + " ]";
  else
    licenseStr += '\n';
  lblLicenseName->setText( licenseStr );
  licenseStr.replace( '\n', QLatin1String( "  " ) );
  lblLicensePreview->setText( licenseStr );
  lblLicensePreview->setCursorPosition( 0 );
  if ( copyingMap.contains( QStringLiteral( "src/link" ) ) )
    lblSrcLink->setText( copyingMap.value( QStringLiteral( "src/link" ) ) );
  else
    lblSrcLink->clear();
}

void QgsCptCityColorRampDialog::cboVariantName_currentIndexChanged( int index )
{
  Q_UNUSED( index )
  if ( cboVariantName->currentIndex() != -1 )
    mRamp.setVariantName( cboVariantName->currentData( Qt::UserRole ).toString() );
  QgsDebugMsg( QStringLiteral( "variant= %1 - %2 variants" ).arg( mRamp.variantName() ).arg( mRamp.variantList().count() ) );
  updatePreview();
  emit changed();
}

void QgsCptCityColorRampDialog::onFinished()
{
  // save settings
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/CptCityColorRampV2Dialog/splitter" ), mSplitter->saveState() );
}

void QgsCptCityColorRampDialog::showHelp()
{
  // show error message to use color ramp manager to get more gradients
  const QString helpText = tr( "You can download a more complete set of cpt-city gradients "
                               "by installing the \"Color Ramp Manager\" plugin "
                               "(you must enable Experimental plugins in the plugin manager).\n\n"
                             );
  QMessageBox *msg = new QMessageBox( this );
  msg->setWindowTitle( tr( "Download More Cpt-city Gradients" ) );
  msg->setText( helpText );
  msg->exec();
}

void QgsCptCityColorRampDialog::updateUi()
{
  // look for item, if not found in selections archive, look for in authors
  QgsDebugMsg( "looking for ramp " + mRamp.schemeName() );
  if ( !mRamp.schemeName().isEmpty() )
  {
    bool found = updateRamp();
    if ( ! found )
    {
      tabBar->setCurrentIndex( 1 );
      setTreeModel( mAuthorsModel );
      found = updateRamp();
      // if not found, go back to selections model
      if ( ! found )
      {
        tabBar->setCurrentIndex( 0 );
        setTreeModel( mSelectionsModel );
      }
    }
    if ( found )
      mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  }
  else
  {
    updateRamp();
  }
}

bool QgsCptCityColorRampDialog::saveAsGradientRamp() const
{
  QgsDebugMsg( QStringLiteral( "result: %1 checked: %2" ).arg( result() ).arg( cboConvertStandard->isChecked() ) );
  // if "save as standard gradient" is checked, convert to QgsVectorGradientColorRamp
  return ( result() == Accepted && cboConvertStandard->isChecked() );
}

QDialogButtonBox *QgsCptCityColorRampDialog::buttonBox() const
{
  return mButtonBox;
}

void QgsCptCityColorRampDialog::updateListWidget( QgsCptCityDataItem *item )
{
  mListWidget->blockSignals( true );
  mListWidget->clear();
  mListRamps.clear();
  QgsCptCityCollectionItem *colItem = qobject_cast<QgsCptCityCollectionItem *>( item );
  if ( colItem )
  {
    QgsDebugMsg( "path= " + item->path() );
    // recursively get children ramps
    QVector<QgsCptCityDataItem *> childrenRamps = colItem->childrenRamps( true );
    for ( int i = 0; i < childrenRamps.count(); i++ )
    {
      QgsCptCityColorRampItem *rampItem = qobject_cast<QgsCptCityColorRampItem *>( childrenRamps[i] );
      if ( ! rampItem )
      {
        QgsDebugMsg( "invalid item " + childrenRamps[i]->path() );
        continue;
      }
      QListWidgetItem *listItem = new QListWidgetItem();
      listItem->setText( rampItem->shortInfo() );
      listItem->setIcon( rampItem->icon( QSize( 75, 50 ) ) );
      listItem->setToolTip( rampItem->path() + '\n' + rampItem->info() );
      listItem->setData( Qt::UserRole, QVariant( i ) );
      mListWidget->addItem( listItem );
      mListRamps << rampItem;
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "invalid item" ) );
  }
  mListWidget->blockSignals( false );
}

// this function is for a svg preview, available if the svg files have been processed with svgx
// e.g. for f in `ls */*/*/*/*.svg`; do echo $f ; svgx -p -t svg $f > tmp1.svg; mv tmp1.svg $f; done
// perhaps a future version of the cpt-city svg gradients will have them by default
bool QgsCptCityColorRampDialog::eventFilter( QObject *obj, QEvent *event )
{
  const QSize size = lblPreview->size();

  if ( event->type() == QEvent::MouseButtonPress )
  {
    // create preview from svg file if supported - depends on file versions
    const QPixmap pixmap( mRamp.fileName() );
    if ( ! pixmap.isNull() )
      lblPreview->setPixmap( pixmap.scaled( size ) );
    return true;
  }
  else if ( event->type() == QEvent::MouseButtonRelease )
  {
    // restore preview
    const QPixmap pixmap = QgsSymbolLayerUtils::colorRampPreviewPixmap( &mRamp, size );
    lblPreview->setPixmap( pixmap );
    return true;
  }
  else
  {
    // standard event processing
    return QDialog::eventFilter( obj, event );
  }
}

bool QgsCptCityColorRampDialog::updateRamp()
{
  mListWidget->clear();
  mListRamps.clear();
  cboVariantName->clear();
  clearCopyingInfo();
  lblCollectionInfo->clear();

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  updatePreview( true );

  QgsDebugMsg( "schemeName= " + mRamp.schemeName() );
  if ( mRamp.schemeName().isEmpty() )
  {
    showAll();
    return false;
  }

  // search for item in model
  const QModelIndex modelIndex = mModel->findPath( mRamp.schemeName() );
  if ( modelIndex == QModelIndex() )
  {
    return false;
  }
  QgsCptCityColorRampItem *childItem =
    qobject_cast<QgsCptCityColorRampItem *>( mModel->dataItem( modelIndex ) );
  if ( ! childItem )
    return false;
  if ( mRamp.schemeName() != childItem->ramp().schemeName() )
    return false;

  // found child, set mRamp variantList
  // mRamp.copy( &childItem->ramp() );
  mRamp.setVariantList( childItem->ramp().variantList() );

  // found child, update tree
  QgsDebugMsg( QStringLiteral( "found item %1" ).arg( mRamp.schemeName() ) );
  lblSchemeName->setText( QFileInfo( mRamp.schemeName() ).fileName() );
  const QModelIndex parentIndex = modelIndex.parent();
  const QModelIndex selIndex = mTreeFilter->mapFromSource( parentIndex );

  // QgsDebugMsg(QString("parent row=%1 path=%2 parentRow=%3").arg(parentIndex.row()).arg(mModel->dataItem( parentIndex )->path()).arg(parentIndex.parent().row()));
  mTreeView->setCurrentIndex( selIndex );
  mTreeView->setExpanded( selIndex, true );
  mTreeView->scrollTo( selIndex, QAbstractItemView::PositionAtCenter );
  updateTreeView( mModel->dataItem( parentIndex ), false );

  // update listWidget, find appropriate item in mListRamps
  for ( int i = 0; i < mListRamps.count(); i++ )
  {
    if ( mListRamps.at( i ) == childItem )
    {
      QgsDebugMsg( QStringLiteral( "found matching item %1 target=%2" ).arg( mListRamps.at( i )->path(), childItem->path() ) );
      QListWidgetItem *listItem = mListWidget->item( i );
      mListWidget->setCurrentItem( listItem );
      // mListWidget_itemClicked( listItem );
      populateVariants();
      mListWidget->scrollToItem( listItem, QAbstractItemView::EnsureVisible );
      // mListView->selectionModel()->select( childIndex, QItemSelectionModel::Select );
      mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
      emit changed();
      return true;
    }
  }

  return false;
}

void QgsCptCityColorRampDialog::showAll()
{
  const QModelIndex modelIndex = mModel->findPath( QString() );
  if ( modelIndex != QModelIndex() )
  {
    const QModelIndex selIndex = mTreeFilter->mapFromSource( modelIndex );
    mTreeView->setCurrentIndex( selIndex );
    mTreeView->setExpanded( selIndex, true );
    mTreeView->scrollTo( selIndex, QAbstractItemView::PositionAtCenter );
    updateTreeView( mModel->dataItem( modelIndex ), false );
  }
}

void QgsCptCityColorRampDialog::setTreeModel( QgsCptCityBrowserModel *model )
{
  mModel = model;

  delete mTreeFilter;
  mTreeFilter = new TreeFilterProxyModel( this, mModel );
  mTreeView->setModel( mTreeFilter );
}

#if 0
void QgsCptCityColorRampDialog::refresh()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  refreshModel( QModelIndex() );
  QApplication::restoreOverrideCursor();
}

void QgsCptCityColorRampDialog::refreshModel( const QModelIndex &index )
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
      QgsDebugMsg( QStringLiteral( "invalid item" ) );
    }
  }

  mModel->refresh( index );

  for ( int i = 0; i < mModel->rowCount( index ); i++ )
  {
    QModelIndex idx = mModel->index( i, 0, index );
    if ( mTreeView->isExpanded( idx ) || !mModel->hasChildren( idx ) )
    {
      refreshModel( idx );
    }
  }
}
#endif

/// @cond PRIVATE

TreeFilterProxyModel::TreeFilterProxyModel( QObject *parent, QgsCptCityBrowserModel *model )
  : QSortFilterProxyModel( parent )
  , mModel( model )
{
  setSourceModel( mModel );
}

bool TreeFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QgsCptCityDataItem *item = mModel->dataItem( mModel->index( sourceRow, 0, sourceParent ) );
  return ( item && !( item->type() == QgsCptCityDataItem::ColorRamp ) );
}


///@endcond
