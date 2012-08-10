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

#include "qgsvectorcolorrampv2.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsdialog.h"

#include <QPushButton>
#include <QTextEdit>

#include <sys/time.h>

/////////

/*
TODO

- implement as model/view
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

  QgsCptCityColorRampV2::loadSchemes( "" );

  // show information on how to install cpt-city files if none are found
  if ( ! QgsCptCityColorRampV2::hasAllSchemes() )
  {
    QTextEdit *edit = new QTextEdit();
    edit->setReadOnly( true );
    // not sure if we want this long string to be translated
    QString helpText = tr( "Error - cpt-city gradient files not found.\n\n"
                           "You have two means of installing them:\n\n"
                           "1) Install the \"Color Ramp Manager\" python plugin "
                           "(you must enable Experimental plugins in the plugin manager) "
                           "and use it to download latest cpt-city package.\n\n"
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
    return;
  }

  // setup ui
  tabBar->addTab( tr( "Selections by theme" ) );
  tabBar->addTab( tr( "All by author" ) );
  treeWidget->setIconSize( QSize( 100, 15 ) );
  cboVariantName->setIconSize( QSize( 100, 15 ) );
  // treeWidget->header()->setResizeMode( 0, QHeaderView::Stretch );
  // treeWidget->header()->setResizeMode( 1, QHeaderView::ResizeToContents );
  lblPreview->installEventFilter( this ); // mouse click on preview label shows svg render

  // populate tree widget - if item not found in selections collection, look for in authors
  on_tabBar_currentChanged( 0 );
  if ( ! treeWidget->currentItem() )
  {
    tabBar->setCurrentIndex( 1 );
    on_tabBar_currentChanged( 1 );
  }

  populateVariants();
  connect( cboVariantName, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setVariantName() ) );
  cboVariantName->setCurrentIndex( cboVariantName->findData( ramp->variantName(), Qt::UserRole ) );

  updatePreview();
}

// inspired from QgsBrowserModel::findPath( QString path )
// but using a QTreeWidget instead of QgsBrowserModel
// would be simpler if we used the model/view framework
QTreeWidgetItem* QgsCptCityColorRampV2Dialog::findPath( QString path )
{
  QTreeWidgetItem *item = 0, *childItem = 0;
  QString itemPath, childPath;
  bool foundParent, foundChild;

  for ( int i = 0; i < treeWidget->topLevelItemCount(); i++ )
  {
    item = treeWidget->topLevelItem( i );
    if ( !item )
      return 0; // an error occurred
    foundParent = false;

    itemPath = item->data( 0, Qt::UserRole ).toString();
    if ( itemPath == path )
    {
      QgsDebugMsg( "Arrived " + itemPath );
      return item; // we have found the item we have been looking for
    }

    // if we are using a selection collection, search for target in the mapping in this group
    if ( mCollection == "selections" )
    {
      itemPath = item->text( 0 );
      foreach ( QString childPath, QgsCptCityColorRampV2::collectionSelections().value( itemPath ) )
      {
        if ( childPath == path )
        {
          foundParent = true;
          break;
        }
      }
    }
    // search for target in parent directory
    else if ( itemPath.endsWith( "/" ) && path.startsWith( itemPath ) )
      foundParent = true;

    // search for target in children
    if ( foundParent )
    {
      // we have found a preceding item: stop searching on this level and go deeper
      foundChild = true;

      while ( foundChild )
      {
        foundChild = false; // assume that the next child item will not be found

        for ( int j = 0; j < item->childCount(); j++ )
        {
          childItem = item->child( j );
          if ( !childItem )
            return 0; // an error occurred
          childPath = childItem->data( 0, Qt::UserRole ).toString();

          if ( childPath == path )
          {
            return childItem; // we have found the item we have been looking for
          }

          if ( childPath.endsWith( "/" ) && path.startsWith( childPath ) )
          {
            // we have found a preceding item: stop searching on this level and go deeper
            foundChild = true;
            item = childItem;
            break;
          }
        }
      }
      break;
    }

  }
  return 0; // not found
}

void QgsCptCityColorRampV2Dialog::populateSchemes( QString view )
{
  QStringList collections;
  QTreeWidgetItem *item, *childItem;

  struct timeval tv1, tv2;
  gettimeofday( &tv1, 0 );

  treeWidget->blockSignals( true );
  treeWidget->clear();

  if ( view == "authors" )
  {
    foreach ( QString collectionName, QgsCptCityColorRampV2::listSchemeCollections() )
    {
      item = makeCollectionItem( collectionName );
      treeWidget->addTopLevelItem( item );
    }
  }
  else if ( view == "selections" )
  {
    QMapIterator< QString, QStringList> it( QgsCptCityColorRampV2::collectionSelections() );
    while ( it.hasNext() )
    {
      it.next();
      QString path = it.key();
      QString descStr = QgsCptCityColorRampV2::collectionNames().value( path );

      item = new QTreeWidgetItem( QStringList() << path << descStr << QString() );
      // item->setData( 0, Qt::UserRole, QString( descStr ) + "/" );  // add / to end to identify it as a collection item
      // item->setData( 0, Qt::UserRole, QString( path ) );

      QFont font;
      font.setBold( true );
      item->setData( 0, Qt::FontRole, QVariant( font ) );
      item->setData( 1, Qt::FontRole, QVariant( font ) );

      // add children schemes and collections
      foreach ( QString childPath, it.value() )
      {
        if ( childPath.endsWith( "/" ) )
        {
          childPath.chop( 1 );
          childItem = makeCollectionItem( childPath );
          childItem->setText( 0, childPath ); //make sure full path is visible
          item->addChild( childItem );
        }
        else
        {
          makeSchemeItem( item, "", childPath );
        }
      }
      treeWidget->addTopLevelItem( item );
    }
  }
  else
  {
    QgsDebugMsg( "invalid view " + view );
  }
  treeWidget->blockSignals( false );

  treeWidget->setColumnWidth( 0, 250 );

  gettimeofday( &tv2, 0 );
  QgsDebugMsg( QString( "done in %1.%2 seconds" ).arg( tv2.tv_sec - tv1.tv_sec
                                                     ).arg(( double )( tv2.tv_usec - tv2.tv_usec ) / 1000000.0 ) );

}

void QgsCptCityColorRampV2Dialog::populateVariants()
{
  QStringList variantList;
  if ( treeWidget->currentItem() )
    variantList = treeWidget->currentItem()->data( 1, Qt::UserRole ).toString().split( " ", QString::SkipEmptyParts );

  cboVariantName->blockSignals( true );
  cboVariantName->clear();

  if ( variantList.isEmpty() )
  {
    cboVariantName->setEnabled( false );
    cboVariantName->setVisible( false );
    setVariantName();
  }
  else
  {
    QString oldVariant = cboVariantName->currentText();
    QgsCptCityColorRampV2 ramp( mRamp->schemeName(), "" );
    QPixmap blankPixmap( cboVariantName->iconSize() );
    blankPixmap.fill( Qt::white );
    QIcon blankIcon( blankPixmap );
    QIcon icon;
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
    // TODO - is this really necessary?
    int idx = cboVariantName->findText( oldVariant );
    if ( idx == -1 ) // not found?
    {
      // use the item in the middle
      idx = cboVariantName->count() / 2;
    }
    cboVariantName->setCurrentIndex( idx );
  }

}

void QgsCptCityColorRampV2Dialog::on_treeWidget_currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous )
{
  Q_UNUSED( previous );
  // directories have name data that ends with /
  QString currentScheme = current->data( 0, Qt::UserRole ).toString();
  if ( ! currentScheme.isNull() && ! currentScheme.endsWith( "/" ) )
  {
    // lblSchemeName->setText( current->data( 0, Qt::UserRole ).toString() );
    lblSchemeName->setText( current->text( 0 ) );
    setSchemeName();
    // populateVariants();
  }
}

void QgsCptCityColorRampV2Dialog::on_treeWidget_itemExpanded( QTreeWidgetItem * item )
{
  // set color count when item is expanded
  QgsCptCityColorRampV2 ramp( "", "" );
  QTreeWidgetItem* childItem = 0;
  QString itemPath, itemDesc, itemVariants;
  QStringList listVariants;
  QPixmap blankPixmap( treeWidget->iconSize() );
  blankPixmap.fill( Qt::white );
  QIcon blankIcon( blankPixmap );

  for ( int i = 0; i < item->childCount(); i++ )
  {
    childItem = item->child( i );

    //skip invalid items or collection items
    if ( ! childItem || childItem->data( 0, Qt::UserRole ).toString().endsWith( "/" ) )
      continue;

    // items with null description need information, those with "" (i.e. unnamed collections) not be checked
    if ( childItem && childItem->text( 1 ).isNull() )
    {
      itemPath = childItem->data( 0, Qt::UserRole ).toString();
      itemDesc = "";
      ramp.setSchemeName( itemPath );
      ramp.setVariantName( "" );

      // if item has variants, use middle item for preview
      itemVariants = childItem->data( 1, Qt::UserRole ).toString();
      if ( ! itemVariants.isNull() )
      {
        listVariants = itemVariants.split( " ", QString::SkipEmptyParts );
        if ( ! listVariants.isEmpty() )
          ramp.setVariantName( listVariants[ listVariants.count() / 2 ] );
        else
          itemVariants = QString();
      }
      // load file and set info
      if ( ramp.loadFile() )
      {
        if ( itemVariants.isNull() )
        {
          int count = ramp.count();
          QgsCptCityColorRampV2::GradientType type = ramp.gradientType();
          if ( type == QgsCptCityColorRampV2::Discrete )
            count--;
          itemDesc = QString::number( count ) + " " + tr( "colors" ) + " - ";
          if ( type == QgsCptCityColorRampV2::Continuous )
            itemDesc += tr( "continuous" );
          else if ( type == QgsCptCityColorRampV2::ContinuousMulti )
            itemDesc += tr( "continuous (multi)" );
          else if ( type == QgsCptCityColorRampV2::Discrete )
            itemDesc += tr( "discrete" );
        }
        else
        {
          itemDesc = QString::number( listVariants.count() ) + " " + tr( "variants" );
        }
        childItem->setText( 1, "   " + itemDesc );
        QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( &ramp, treeWidget->iconSize() );
        childItem->setIcon( 1, icon );
      }
      else
      {
        childItem->setIcon( 1, blankIcon );
        childItem->setText( 1, "" );
      }
    }
  }
}

#if 0
void QgsCptCityColorRampV2Dialog::on_buttonGroupView_buttonClicked( QAbstractButton * button )
{
  if ( button->objectName() == "rbtnAuthor" )
  {
    populateSchemes( "authors" );
  }
  else if ( button->objectName() == "rbtnSelections" )
  {
    populateSchemes( "selections" );
  }
  else
  {
    QgsDebugMsg( "invalid button " + button->objectName() );
  }
}
#endif

void QgsCptCityColorRampV2Dialog::on_tabBar_currentChanged( int index )
{
  if ( index == 0 )
    mCollection = "selections";
  else if ( index == 1 )
    mCollection = "authors";
  else
  {
    QgsDebugMsg( QString( "invalid index %1" ).arg( index ) );
    mCollection = QString();
  }
  populateSchemes( mCollection );

  // restore selection, if any
  if ( ! mCollection.isNull() )
  {
    lblSchemeName->setText( mRamp->schemeName() );
    treeWidget->setCurrentItem( findPath( mRamp->schemeName() ) );
  }
}

void QgsCptCityColorRampV2Dialog::on_pbtnLicenseDetails_pressed()
{
  // prepare dialog
  QgsDialog dlg( this, 0, QDialogButtonBox::Close );
  QVBoxLayout *layout = dlg.layout();
  QString title = tr( "gradient %1 details" ).arg( mRamp->schemeName() + mRamp->variantName() );
  dlg.setWindowTitle( title );
  QTextEdit *textEdit = new QTextEdit( &dlg );
  textEdit->setReadOnly( true );
  layout->addWidget( textEdit );

  // add contents of DESC.xml and COPYING.xml
  QString copyFile = mRamp->copyingFileName();
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
  QString descFile = mRamp->descFileName();
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
  if ( mRamp->schemeName() == "" )
    return;

  mRamp->loadFile();
  // TODO draw checker-board/transparent background
  // for transparent, add  [ pixmap.fill( Qt::transparent ); ] to QgsSymbolLayerV2Utils::colorRampPreviewPixmap

  lblSchemePath->setText( mRamp->schemeName() + mRamp->variantName() );

  // update pixmap
  QSize size( 300, 50 );
  QPixmap pixmap = QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size );
  lblPreview->setPixmap( pixmap );

  // add copyright information from COPYING.xml file
  QMap< QString, QString > copyingMap = mRamp->copyingInfo();
  QString authorStr = copyingMap[ "authors" ];
  if ( authorStr.length() > 80 )
    authorStr.replace( authorStr.indexOf( " ", 80 ), 1, "\n" );
  lblAuthorName->setText( authorStr );
  QString licenseStr = copyingMap[ "license/informal" ];
  if ( copyingMap.contains( "license/year" ) )
    licenseStr += " (" + copyingMap[ "license/year" ] + ")";
  if ( licenseStr.length() > 80 )
    licenseStr.replace( licenseStr.indexOf( " ", 80 ), 1, "\n" );
  if ( copyingMap.contains( "license/url" ) )
    licenseStr += "\n[ " + copyingMap[ "license/url" ] + " ]";
  else
    licenseStr += "\n";
  lblLicenseName->setText( licenseStr );
  if ( copyingMap.contains( "src/link" ) )
    lblSrcLink->setText( copyingMap[ "src/link"] );
  else
    lblSrcLink->setText( "" );
}

void QgsCptCityColorRampV2Dialog::setSchemeName()
{
  if ( treeWidget->currentItem() )
  {
    mRamp->setSchemeName( treeWidget->currentItem()->data( 0, Qt::UserRole ).toString() );
    populateVariants();
  }
}

void QgsCptCityColorRampV2Dialog::setVariantName()
{
  mRamp->setVariantName( cboVariantName->itemData( cboVariantName->currentIndex(), Qt::UserRole ).toString() );
  updatePreview();
}

/* collection items have columns name / path/ and desc/null */
QTreeWidgetItem * QgsCptCityColorRampV2Dialog::makeCollectionItem( const QString& path )
{
  // add root item
  QString descStr = QgsCptCityColorRampV2::collectionNames().value( path );
  if ( descStr.isNull() )
    descStr = "";
  QTreeWidgetItem *item = new QTreeWidgetItem( QStringList() << QFileInfo( path ).baseName() << descStr );
  item->setData( 0, Qt::UserRole, QString( path ) + "/" );  // add / to end to identify it as a collection item
  //set collections bold to identify them
  QFont font;
  font.setBold( true );
  item->setData( 0, Qt::FontRole, QVariant( font ) );
  item->setData( 1, Qt::FontRole, QVariant( font ) );

  // add children collections
  QTreeWidgetItem *childItem;
  foreach ( QString childPath, QgsCptCityColorRampV2::listSchemeCollections( path ) )
  {
    childItem = makeCollectionItem( childPath );
    item->addChild( childItem );
  }

  // add children schemes
  foreach ( QString schemeName, QgsCptCityColorRampV2::schemeMap().value( path ) )
  {
    makeSchemeItem( item, path, schemeName );
  }
  return item;
}

/* scheme items have columns 0 = ( name / path ) and
[if has variants] 1 = ( #variants / variants )
[if has colors]  1 = ( null / null ) which becomes ( <info> / null ) after populating
*/
void QgsCptCityColorRampV2Dialog::makeSchemeItem( QTreeWidgetItem *item, const QString& path, const QString& schemeName )
{
  QString descStr, descData;
  QStringList listVariants;
  QTreeWidgetItem *childItem;

  listVariants = QgsCptCityColorRampV2::schemeVariants().value( path + "/" + schemeName );

  if ( listVariants.count() > 1 )
  {
    descData = listVariants.join( " " );
  }

  childItem = new QTreeWidgetItem( QStringList() << schemeName << descStr );
  if ( path != "" )
    childItem->setData( 0, Qt::UserRole, path + "/" + schemeName );
  else
    childItem->setData( 0, Qt::UserRole, schemeName );
  if ( ! descData.isNull() )
    childItem->setData( 1, Qt::UserRole, descData );
  item->addChild( childItem );
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

