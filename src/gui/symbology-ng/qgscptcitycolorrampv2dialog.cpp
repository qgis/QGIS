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

#include <QPushButton>
#include <QTextEdit>

#if 0 // unused
static void updateColorButton( QAbstractButton* button, QColor color )
{
  QPixmap p( 20, 20 );
  p.fill( color );
  button->setIcon( QIcon( p ) );
}
#endif

/////////

QgsCptCityColorRampV2Dialog::QgsCptCityColorRampV2Dialog( QgsCptCityColorRampV2* ramp, QWidget* parent )
    : QDialog( parent ), mRamp( ramp )
{

  setupUi( this );

  QgsCptCityColorRampV2::loadSchemes( "" );
  // QgsCptCityColorRampV2::loadSchemes( "cb" );

  // show information on how to install cpt-city files if none are found
  if ( ! QgsCptCityColorRampV2::hasAllSchemes() )
  {
    QTextEdit *edit = new QTextEdit();
    edit->setReadOnly( true );
    // not sure if we want this long string to be translated
    QString helpText = tr( "Error - cpt-city gradient files not found.\n\n"
                           "Please download the complete collection (in svg format) "
                           "and unzip it to your QGis settings directory [%1] .\n\n"
                           "This file can be found at [%2]\nand current file is [%3]"
                         ).arg( QgsApplication::qgisSettingsDirPath()
                              ).arg( "http://soliton.vm.bytemark.co.uk/pub/cpt-city/pkg/"
                                   ).arg( "http://soliton.vm.bytemark.co.uk/pub/cpt-city/pkg/cpt-city-svg-2.02.zip" );
    edit->setText( helpText );
    stackedWidget->addWidget( edit );
    stackedWidget->setCurrentIndex( 1 );
    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    return;
  }

  populateSchemes( "author" );
  treeWidget->setCurrentItem( findPath( ramp->schemeName() ) );
  populateVariants();
  cboVariantName->setCurrentIndex( cboVariantName->findData( ramp->variantName(), Qt::UserRole ) );
  connect( cboVariantName, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setVariantName() ) );
  updatePreview();
}

// inspired from QgsBrowserModel::findPath( QString path )
// but using a QTreeWidget instead of QgsBrowserModel
// would be simpler if we used the model/view framework
QTreeWidgetItem* QgsCptCityColorRampV2Dialog::findPath( QString path )
{
  QTreeWidgetItem *item = 0, *childItem = 0;
  QString itemPath, childPath;

  for ( int i = 0; i < treeWidget->topLevelItemCount(); i++ )
  {
    item = treeWidget->topLevelItem( i );
    if ( !item )
      return 0; // an error occurred
    itemPath = item->data( 0, Qt::UserRole ).toString();

    if ( itemPath == path )
    {
      QgsDebugMsg( "Arrived " + itemPath );
      return item; // we have found the item we have been looking for
    }

    if ( itemPath.endsWith( "/" ) && path.startsWith( itemPath ) )
    {
      // we have found a preceding item: stop searching on this level and go deeper
      bool foundChild = true;

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
  if ( view == "author" )
  {
    treeWidget->blockSignals( true );
    treeWidget->clear();
    foreach( QString collectionName, QgsCptCityColorRampV2::listSchemeCollections() )
    {
      item = makeCollectionItem( collectionName );
      treeWidget->addTopLevelItem( item );
    }
    treeWidget->blockSignals( false );
  }
  else if ( view == "selections" )
  {
    treeWidget->blockSignals( true );
    treeWidget->clear();

    QMapIterator< QString, QStringList> it( QgsCptCityColorRampV2::collectionSelections() );
    while ( it.hasNext() )
    {
      it.next();
      QString path = it.key();
      QString descStr = QgsCptCityColorRampV2::collectionNames().value( path );

      // TODO shorten child names and fix / at beginning of name
      // TODO when collection has only 1 item (e.g. es_skywalker), bring up one level
      item = new QTreeWidgetItem( QStringList() << path << descStr << QString() );
      QFont font;
      font.setBold( true );
      item->setData( 0, Qt::FontRole, QVariant( font ) );
      item->setData( 1, Qt::FontRole, QVariant( font ) );

      // add children schemes and collections
      foreach( QString childPath, it.value() )
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
    treeWidget->blockSignals( false );
  }
  else
  {
    QgsDebugMsg( "invalid view " + view );
    return;
  }

  treeWidget->setColumnWidth( 0, 250 );
}

void QgsCptCityColorRampV2Dialog::populateVariants()
{
  if ( ! treeWidget->currentItem() )
    return;

  cboVariantName->blockSignals( true );

  QString oldVariant = cboVariantName->currentText();
  cboVariantName->clear();

  foreach( QString variant, treeWidget->currentItem()->data( 1, Qt::UserRole ).toString().split( " ", QString::SkipEmptyParts ) )
  {
    QString variantStr = variant;
    if ( variantStr.startsWith( "-" ) || variantStr.startsWith( "_" ) )
      variantStr.remove( 0, 1 );
    cboVariantName->addItem( variantStr );
    cboVariantName->setItemData( cboVariantName->count() - 1, variant, Qt::UserRole );
  }

  cboVariantName->blockSignals( false );

  if ( cboVariantName->count() > 0 )
  {
    cboVariantName->setEnabled( true );
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
  else
  {
    cboVariantName->setEnabled( false );
    setVariantName();
  }

}

void QgsCptCityColorRampV2Dialog::on_treeWidget_currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous )
{
  Q_UNUSED( previous );
  // directories have name data that ends with /
  if ( ! current->data( 0, Qt::UserRole ).toString().endsWith( "/" ) )
  {
    lblSchemeName->setText( current->data( 0, Qt::UserRole ).toString() );
    setSchemeName();
    // populateVariants();
  }
}

void QgsCptCityColorRampV2Dialog::on_treeWidget_itemExpanded( QTreeWidgetItem * item )
{
  // set color count when item is expanded
  QgsCptCityColorRampV2 ramp( "", "" );
  QTreeWidgetItem* childItem = 0;
  QString itemPath, itemDesc;

  for ( int i = 0; i < item->childCount(); i++ )
  {
    childItem = item->child( i );

    // items with null description need information, those with "" (i.e. unnamed collections) not be checked
    // TODO set type when there are variants - based on the first file
    if ( childItem && childItem->text( 1 ).isNull() )
    {
      itemPath = childItem->data( 0, Qt::UserRole ).toString();
      itemDesc = "";
      ramp.setSchemeName( itemPath );
      if ( ramp.loadFile() )
      {
        itemDesc = QString::number( ramp.count() ) + " " + tr( "colors" ) + " - ";
        if ( ramp.isContinuous() )
          itemDesc += tr( "continuous" );
        else
          itemDesc += tr( "discrete" );
      }
      childItem->setText( 1, "     " + itemDesc );
    }
  }
}

void QgsCptCityColorRampV2Dialog::on_buttonGroupView_buttonClicked( QAbstractButton * button )
{
  if ( button->objectName() == "rbtnAuthor" )
  {
    populateSchemes( "author" );
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

void QgsCptCityColorRampV2Dialog::updatePreview()
{
  QSize size( 300, 40 );
  mRamp->loadFile();
  lblPreview->setPixmap( QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size ) );
}

void QgsCptCityColorRampV2Dialog::setSchemeName()
{
  if ( treeWidget->currentItem() )
  {
    mRamp->setSchemeName( treeWidget->currentItem()->data( 0, Qt::UserRole ).toString() );
    // setVariantName();

    // populate list of variants
    populateVariants();

    // updatePreview();
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
  foreach( QString childPath, QgsCptCityColorRampV2::listSchemeCollections( path ) )
  {
    childItem = makeCollectionItem( childPath );
    item->addChild( childItem );
  }

  // add children schemes
  foreach( QString schemeName, QgsCptCityColorRampV2::schemeMap().value( path ) )
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
  QString descStr, descData;//, variantStr;
  QString curName, curVariant;
  QStringList listVariant;
  QTreeWidgetItem *childItem;

  // // descStr = schemeName;
  listVariant = QgsCptCityColorRampV2::schemeVariants().value( path + "/" + schemeName );

  if ( listVariant.count() > 1 )
  {
    // variantStr = QString::number( listVariant.count() ) + " " + tr( "variants" );
    descStr = "     " + QString::number( listVariant.count() ) + " " + tr( "variants" );
    descData = listVariant.join( " " );
  }

  childItem = new QTreeWidgetItem( QStringList() << schemeName << descStr );
  childItem->setData( 0, Qt::UserRole, path + "/" + schemeName );
  if ( ! descData.isNull() )
    childItem->setData( 1, Qt::UserRole, descData );
  item->addChild( childItem );

  // create a preview icon using five color variant
  // QgsCptCityColorRampV2* r = new QgsCptCityColorRampV2( schemeName, 5 );
  // QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( r, iconSize );
  // delete r;

}

