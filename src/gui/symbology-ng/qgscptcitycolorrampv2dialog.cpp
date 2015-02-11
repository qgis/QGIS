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

#include "qgscptcityarchive.h"
#include "qgsvectorcolorrampv2.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsdialog.h"

#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QMessageBox>
#include <QSortFilterProxyModel>

/////////

// TODO
// - fix Diverging children when first show Selections
// - fix crash on Diverging?

class TreeFilterProxyModel : public QSortFilterProxyModel
{
    //  Q_OBJECT

  public:
    TreeFilterProxyModel( QObject *parent, QgsCptCityBrowserModel* model )
        : QSortFilterProxyModel( parent ), mModel( model )
    { setSourceModel( mModel ); }

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override
    {
      QgsCptCityDataItem* item = mModel->dataItem( mModel->index( sourceRow, 0, sourceParent ) );
      return ( item && !( item->type() == QgsCptCityDataItem::ColorRamp ) );
    }
    // bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

  private:
    QgsCptCityBrowserModel* mModel;
};


// ----------------------

QgsCptCityColorRampV2Dialog::QgsCptCityColorRampV2Dialog( QgsCptCityColorRampV2* ramp, QWidget* parent )
    : QDialog( parent )
    , mRamp( 0 )
    , mArchiveViewType( QgsCptCityBrowserModel::Selections )
{
  setupUi( this );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/CptCityColorRampV2Dialog/geometry" ).toByteArray() );
  mSplitter->setSizes( QList<int>() << 250 << 550 );
  mSplitter->restoreState( settings.value( "/Windows/CptCityColorRampV2Dialog/splitter" ).toByteArray() );

  mModel = mAuthorsModel = mSelectionsModel = 0; //mListModel = 0;
  mTreeFilter = 0;

  QgsCptCityArchive::initDefaultArchive();
  mArchive = QgsCptCityArchive::defaultArchive();

  // show information on how to install cpt-city files if none are found
  if ( ! mArchive || mArchive->isEmpty() )
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
                           "2) Download the complete archive (in svg format) "
                           "and unzip it to your QGIS settings directory [%1] .\n\n"
                           "This file can be found at [%2]\nand current file is [%3]"
                         ).arg( QgsApplication::qgisSettingsDirPath()
                              ).arg( "http://soliton.vm.bytemark.co.uk/pub/cpt-city/pkg/"
                                   ).arg( "http://soliton.vm.bytemark.co.uk/pub/cpt-city/pkg/cpt-city-svg-2.07.zip" );
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

  if ( ramp )
  {
    mRamp = ramp;
  }
  else
  {
    mRamp = new QgsCptCityColorRampV2( "", "", false );
    ramp = mRamp;
  }
  QgsDebugMsg( QString( "ramp name= %1 variant= %2 - %3 variants" ).arg( ramp->schemeName() ).arg( ramp->variantName() ).arg( ramp->variantList().count() ) );

  // model / view
  QgsDebugMsg( "loading model/view objects" );
  if ( mAuthorsModel )
    delete mAuthorsModel;
  mAuthorsModel = new QgsCptCityBrowserModel( this, mArchive, QgsCptCityBrowserModel::Authors );
  if ( mSelectionsModel )
    delete mSelectionsModel;
  mSelectionsModel = new QgsCptCityBrowserModel( this, mArchive, QgsCptCityBrowserModel::Selections );
  setTreeModel( mSelectionsModel );

  mTreeView->setSelectionMode( QAbstractItemView::SingleSelection );
  mTreeView->setColumnHidden( 1, true );
  QgsDebugMsg( "done loading model/view objects" );

  // setup ui
  tabBar->blockSignals( true );
  tabBar->addTab( tr( "Selections by theme" ) );
  tabBar->addTab( tr( "All by author" ) );
  cboVariantName->setIconSize( QSize( 100, 15 ) );
  lblPreview->installEventFilter( this ); // mouse click on preview label shows svg render

  // look for item, if not found in selections archive, look for in authors
  QgsDebugMsg( "looking for ramp " + mRamp->schemeName() );
  if ( mRamp->schemeName() != "" )
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
      buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  }
  else
  {
    updateRamp();
  }

  tabBar->blockSignals( false );

  connect( this, SIGNAL( finished( int ) ), this, SLOT( onFinished() ) );

}

QgsCptCityColorRampV2Dialog::~QgsCptCityColorRampV2Dialog()
{
}

void QgsCptCityColorRampV2Dialog::populateVariants()
{
  QStringList variantList = mRamp->variantList();

  QgsDebugMsg( QString( "ramp %1%2 has %3 variants" ).arg( mRamp->schemeName() ).arg( mRamp->variantName() ).arg( variantList.count() ) );

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
    // populate variant combobox
    QString oldVariant = cboVariantName->currentText();
    QgsCptCityColorRampV2 ramp( mRamp->schemeName(), mRamp->variantList(), QString() );
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

    // try to set the original variant again (if exists)
    int idx = -1;
    QString newVariant = mRamp->variantName();
    QgsDebugMsg( QString( "variant= %1 - %2 variants" ).arg( mRamp->variantName() ).arg( mRamp->variantList().count() ) );
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

    cboVariantName->setEnabled( true );
    cboVariantName->setVisible( true );
  }

}

void QgsCptCityColorRampV2Dialog::on_mTreeView_clicked( const QModelIndex &index )
{
  const QModelIndex &sourceIndex = mTreeFilter->mapToSource( index );
  QgsCptCityDataItem *item = mModel->dataItem( sourceIndex );
  if ( ! item )
    return;
  QgsDebugMsg( QString( "item %1 clicked" ).arg( item->name() ) );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  updateTreeView( item );
}

void QgsCptCityColorRampV2Dialog::updateTreeView( QgsCptCityDataItem *item, bool resetRamp )
{
  if ( ! item )
  {
    QgsDebugMsg( "invalid item" );
    return;
  }
  if ( item->type() == QgsCptCityDataItem::Directory )
  {
    if ( resetRamp )
    {
      mRamp->setName( "", "" );
      QgsDebugMsg( QString( "variant= %1 - %2 variants" ).arg( mRamp->variantName() ).arg( mRamp->variantList().count() ) );
      lblSchemeName->setText( "" );
      populateVariants();
    }
    updateListWidget( item );
    lblSchemePath->setText( item->path() );
    lblCollectionInfo->setText( QString( "%1 (%2)" ).arg( item->info() ).arg( item->leafCount() ) );
    updateCopyingInfo( mArchive->copyingInfo( mArchive->copyingFileName( item->path() ) ) );
  }
  else if ( item->type() == QgsCptCityDataItem::Selection )
  {
    lblSchemePath->setText( "" );
    clearCopyingInfo();
    updateListWidget( item );
    lblCollectionInfo->setText( QString( "%1 (%2)" ).arg( item->info() ).arg( item->leafCount() ) );
  }
  else if ( item->type() == QgsCptCityDataItem::AllRamps )
  {
    lblSchemePath->setText( "" );
    clearCopyingInfo();
    updateListWidget( item );
    lblCollectionInfo->setText( tr( "All Ramps (%1)" ).arg( item->leafCount() ) );
  }
  else
  {
    QgsDebugMsg( QString( "item %1 has invalid type %2" ).arg( item->path() ).arg(( int )item->type() ) );
  }
}

void QgsCptCityColorRampV2Dialog::on_mListWidget_itemClicked( QListWidgetItem * item )
{
  QgsCptCityColorRampItem *rampItem = mListRamps.at( item->data( Qt::UserRole ).toInt() );
  if ( rampItem )
  {
    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
    lblSchemeName->setText( QFileInfo( rampItem->name() ).fileName() );
    mRamp->copy( &rampItem->ramp() );
    QgsDebugMsg( QString( "variant= %1 - %2 variants" ).arg( mRamp->variantName() ).arg( mRamp->variantList().count() ) );
    populateVariants();
  }
  else
  {
    QgsDebugMsg( "invalid item" );
  }
}

void QgsCptCityColorRampV2Dialog::on_mListWidget_itemSelectionChanged()
{
  if ( mListWidget->selectedItems().count() == 0 )
  {
    mRamp->setName( "", "" );
  }
}

void QgsCptCityColorRampV2Dialog::on_tabBar_currentChanged( int index )
{
  QgsDebugMsg( "Entered" );
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
    QgsDebugMsg( QString( "invalid index %1" ).arg( index ) );
    setTreeModel( mAuthorsModel );
    mArchiveViewType = QgsCptCityBrowserModel::Authors;
  }

  mListWidget->blockSignals( true );
  updateRamp();
  mListWidget->blockSignals( false );
}


void QgsCptCityColorRampV2Dialog::on_pbtnLicenseDetails_pressed()
{
  QString path, title, copyFile, descFile;

  // get basic information, depending on if is color ramp or directory
  QgsCptCityDataItem *item =
    dynamic_cast< QgsCptCityDataItem* >( mModel->dataItem( mTreeFilter->mapToSource( mTreeView->currentIndex() ) ) );
  if ( ! item )
    return;

  path = item->path();
  if ( item->type() == QgsCptCityDataItem::Directory )
  {
    title = tr( "%1 directory details" ).arg( item->path() );
  }
  else if ( item->type() == QgsCptCityColorRampItem::Directory )
  {
    title = tr( "%1 gradient details" ).arg( path );
  }
  else
  {
    return;
  }
  copyFile = mArchive->copyingFileName( path );
  descFile = mArchive->descFileName( path );

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

void QgsCptCityColorRampV2Dialog::updatePreview( bool clear )
{
  QSize size = lblPreview->size();

  if ( clear || mRamp->schemeName() == "" )
  {
    lblSchemeName->setText( "" );
    lblSchemePath->setText( "" );
    lblLicensePreview->setText( "" );
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

void QgsCptCityColorRampV2Dialog::clearCopyingInfo()
{
  updateCopyingInfo( QMap< QString, QString >() );
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
  licenseStr.replace( "\n", "  " );
  lblLicensePreview->setText( licenseStr );
  lblLicensePreview->setCursorPosition( 0 );
  if ( copyingMap.contains( "src/link" ) )
    lblSrcLink->setText( copyingMap.value( "src/link" ) );
  else
    lblSrcLink->setText( "" );
}

void QgsCptCityColorRampV2Dialog::on_cboVariantName_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( cboVariantName->currentIndex() != -1 )
    mRamp->setVariantName( cboVariantName->itemData( cboVariantName->currentIndex(), Qt::UserRole ).toString() );
  QgsDebugMsg( QString( "variant= %1 - %2 variants" ).arg( mRamp->variantName() ).arg( mRamp->variantList().count() ) );
  updatePreview();
}

void QgsCptCityColorRampV2Dialog::onFinished()
{
  // save settings
  QSettings settings;
  settings.setValue( "/Windows/CptCityColorRampV2Dialog/geometry", saveGeometry() );
  settings.setValue( "/Windows/CptCityColorRampV2Dialog/splitter", mSplitter->saveState() );
}

void QgsCptCityColorRampV2Dialog::on_buttonBox_helpRequested()
{
  // show error message to use color ramp manager to get more gradients
  QString helpText = tr( "You can download a more complete set of cpt-city gradients "
                         "by installing the \"Color Ramp Manager\" plugin "
                         "(you must enable Experimental plugins in the plugin manager).\n\n"
                       );
  QMessageBox* msg = new QMessageBox( this );
  msg->setText( helpText );
  msg->exec();
}

bool QgsCptCityColorRampV2Dialog::saveAsGradientRamp() const
{
  QgsDebugMsg( QString( "result: %1 checked: %2" ).arg( result() ).arg( cboConvertStandard->isChecked() ) );
  // if "save as standard gradient" is checked, convert to QgsVectorGradientColorRampV2
  return ( result() == Accepted && cboConvertStandard->isChecked() );
}

void QgsCptCityColorRampV2Dialog::updateListWidget( QgsCptCityDataItem *item )
{
  mListWidget->blockSignals( true );
  mListWidget->clear();
  mListRamps.clear();
  QgsCptCityCollectionItem* colItem = dynamic_cast<QgsCptCityCollectionItem*>( item );
  if ( colItem )
  {
    QgsDebugMsg( "path= " + item->path() );
    // recursively get children ramps
    QVector<QgsCptCityDataItem*> childrenRamps = colItem->childrenRamps( true );
    for ( int i = 0; i < childrenRamps.count(); i++ )
    {
      QgsCptCityColorRampItem* rampItem = dynamic_cast<QgsCptCityColorRampItem*>( childrenRamps[i] );
      if ( ! rampItem )
      {
        QgsDebugMsg( "invalid item " + childrenRamps[i]->path() );
        continue;
      }
      QListWidgetItem* listItem = new QListWidgetItem();
      listItem->setText( rampItem->shortInfo() );
      listItem->setIcon( rampItem->icon( QSize( 75, 50 ) ) );
      listItem->setToolTip( rampItem->path() + "\n" + rampItem->info() );
      listItem->setData( Qt::UserRole, QVariant( i ) );
      mListWidget->addItem( listItem );
      mListRamps << rampItem;
    }
  }
  else
  {
    QgsDebugMsg( "invalid item" );
  }
  mListWidget->blockSignals( false );
}

// this function is for a svg preview, available if the svg files have been processed with svgx
// e.g. for f in `ls */*/*/*/*.svg`; do echo $f ; svgx -p -t svg $f > tmp1.svg; mv tmp1.svg $f; done
// perhaps a future version of the cpt-city svg gradients will have them by default
bool QgsCptCityColorRampV2Dialog::eventFilter( QObject *obj, QEvent *event )
{
  QSize size = lblPreview->size();

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

bool QgsCptCityColorRampV2Dialog::updateRamp()
{
  QgsDebugMsg( "Entered" );
  mListWidget->clear();
  mListRamps.clear();
  cboVariantName->clear();
  clearCopyingInfo();
  lblCollectionInfo->clear();

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  updatePreview( true );

  QgsDebugMsg( "schemeName= " + mRamp->schemeName() );
  if ( mRamp->schemeName() == "" )
  {
    showAll();
    return false;
  }

  // search for item in model
  QModelIndex modelIndex = mModel->findPath( mRamp->schemeName() );
  if ( modelIndex == QModelIndex() )
  {
    return false;
  }
  QgsCptCityColorRampItem* childItem =
    dynamic_cast<QgsCptCityColorRampItem*>( mModel->dataItem( modelIndex ) );
  if ( ! childItem )
    return false;
  if ( mRamp->schemeName() != childItem->ramp().schemeName() )
    return false;

  // found child, set mRamp variantList
  // mRamp->copy( &childItem->ramp() );
  mRamp->setVariantList( childItem->ramp().variantList() );

  // found child, update tree
  QgsDebugMsg( QString( "found item %1" ).arg( mRamp->schemeName() ) );
  lblSchemeName->setText( QFileInfo( mRamp->schemeName() ).fileName() );
  QModelIndex parentIndex = modelIndex.parent();
  QModelIndex selIndex = mTreeFilter->mapFromSource( parentIndex );

  // QgsDebugMsg(QString("parent row=%1 path=%2 parentRow=%3").arg(parentIndex.row()).arg(mModel->dataItem( parentIndex )->path()).arg(parentIndex.parent().row()));
  mTreeView->setCurrentIndex( selIndex );
  mTreeView->setExpanded( selIndex, true );
  mTreeView->scrollTo( selIndex, QAbstractItemView::PositionAtCenter );
  updateTreeView( mModel->dataItem( parentIndex ), false );

  // update listWidget, find appropriate item in mListRamps
  for ( int i = 0; i < mListRamps.count(); i++ )
  {
    if ( mListRamps[i] == childItem )
    {
      QgsDebugMsg( QString( "found matching item %1 target=%2" ).arg( mListRamps[i]->path() ).arg( childItem->path() ) );
      QListWidgetItem* listItem = mListWidget->item( i );
      mListWidget->setCurrentItem( listItem );
      // on_mListWidget_itemClicked( listItem );
      populateVariants();
      mListWidget->scrollToItem( listItem, QAbstractItemView::EnsureVisible );
      // mListView->selectionModel()->select( childIndex, QItemSelectionModel::Select );
      buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
      return true;
    }
  }

  return false;
}

void QgsCptCityColorRampV2Dialog::showAll()
{
  QModelIndex modelIndex = mModel->findPath( "" );
  if ( modelIndex != QModelIndex() )
  {
    QModelIndex selIndex = mTreeFilter->mapFromSource( modelIndex );
    mTreeView->setCurrentIndex( selIndex );
    mTreeView->setExpanded( selIndex, true );
    mTreeView->scrollTo( selIndex, QAbstractItemView::PositionAtCenter );
    updateTreeView( mModel->dataItem( modelIndex ), false );
  }
}

void QgsCptCityColorRampV2Dialog::setTreeModel( QgsCptCityBrowserModel* model )
{
  QgsDebugMsg( "Entered" );
  mModel = model;

  if ( mTreeFilter )
    delete mTreeFilter;
  mTreeFilter = new TreeFilterProxyModel( this, mModel );
  mTreeView->setModel( mTreeFilter );
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
    if ( mTreeView->isExpanded( idx ) || !mModel->hasChildren( idx ) )
    {
      refreshModel( idx );
    }
  }
}
#endif
