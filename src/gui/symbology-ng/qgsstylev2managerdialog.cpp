/***************************************************************************
    qgsstylev2managerdialog.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstylev2managerdialog.h"

#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"

#include "qgssymbolv2selectordialog.h"
#include "qgsvectorgradientcolorrampv2dialog.h"
#include "qgsvectorrandomcolorrampv2dialog.h"
#include "qgsvectorcolorbrewercolorrampv2dialog.h"
#include "qgscptcitycolorrampv2dialog.h"
#include "qgsstylev2exportimportdialog.h"
#include "qgssmartgroupeditordialog.h"

#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardItemModel>
#include <QAction>
#include <QMenu>

#include "qgsapplication.h"
#include "qgslogger.h"


QgsStyleV2ManagerDialog::QgsStyleV2ManagerDialog( QgsStyleV2* style, QWidget* parent )
    : QDialog( parent ), mStyle( style ), mModified( false )
{
  setupUi( this );
#ifdef Q_WS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/StyleV2Manager/geometry" ).toByteArray() );
  mSplitter->setSizes( QList<int>() << 170 << 540 );
  mSplitter->restoreState( settings.value( "/Windows/StyleV2Manager/splitter" ).toByteArray() );

  tabItemType->setDocumentMode( true );
  searchBox->setPlaceholderText( tr( "Type here to filter symbols..." ) );

  // setup icons
  btnAddItem->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.png" ) ) );
  btnEditItem->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  btnRemoveItem->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.png" ) ) );
  btnShare->setIcon( QIcon( QgsApplication::iconPath( "user.png" ) ) );

  connect( this, SIGNAL( finished( int ) ), this, SLOT( onFinished() ) );

  connect( listItems, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( editItem() ) );

  connect( btnAddItem, SIGNAL( clicked() ), this, SLOT( addItem() ) );
  connect( btnEditItem, SIGNAL( clicked() ), this, SLOT( editItem() ) );
  connect( btnRemoveItem, SIGNAL( clicked() ), this, SLOT( removeItem() ) );

  QMenu *shareMenu = new QMenu( tr( "Share Menu" ), this );
  QAction *exportAction = shareMenu->addAction( tr( "Export" ) );
  QAction *importAction = shareMenu->addAction( tr( "Import" ) );
  connect( exportAction, SIGNAL( triggered() ), this, SLOT( exportItems() ) );
  connect( importAction, SIGNAL( triggered() ), this, SLOT( importItems() ) );
  btnShare->setMenu( shareMenu );

  // Set editing mode off by default
  mGrouppingMode = false;

  QStandardItemModel* model = new QStandardItemModel( listItems );
  listItems->setModel( model );
  listItems->setSelectionMode( QAbstractItemView::ExtendedSelection );

  connect( model, SIGNAL( itemChanged( QStandardItem* ) ), this, SLOT( itemChanged( QStandardItem* ) ) );
  connect( listItems->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( symbolSelected( const QModelIndex& ) ) );

  populateTypes();

  QStandardItemModel* groupModel = new QStandardItemModel( groupTree );
  groupTree->setModel( groupModel );
  groupTree->setHeaderHidden( true );
  populateGroups();
  connect( groupTree->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( groupChanged( const QModelIndex& ) ) );
  connect( groupModel, SIGNAL( itemChanged( QStandardItem* ) ),
           this, SLOT( groupRenamed( QStandardItem* ) ) );

  QMenu *groupMenu = new QMenu( tr( "Group Actions" ), this );
  QAction *groupSymbols = groupMenu->addAction( tr( "Group Symbols" ) );
  QAction *editSmartgroup = groupMenu->addAction( tr( "Edit Smart Group" ) );
  btnManageGroups->setMenu( groupMenu );
  connect( groupSymbols, SIGNAL( triggered() ), this, SLOT( groupSymbolsAction() ) );
  connect( editSmartgroup, SIGNAL( triggered() ), this, SLOT( editSmartgroupAction() ) );

  connect( btnAddGroup, SIGNAL( clicked() ), this, SLOT( addGroup() ) );
  connect( btnRemoveGroup, SIGNAL( clicked() ), this, SLOT( removeGroup() ) );

  on_tabItemType_currentChanged( 0 );

  connect( searchBox, SIGNAL( textChanged( QString ) ), this, SLOT( filterSymbols( QString ) ) );
  tagsLineEdit->installEventFilter( this );

  // Context menu for groupTree
  groupTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( groupTree, SIGNAL( customContextMenuRequested( const QPoint& ) ),
           this, SLOT( grouptreeContextMenu( const QPoint& ) ) );

  // Context menu for listItems
  listItems->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( listItems, SIGNAL( customContextMenuRequested( const QPoint& ) ),
           this, SLOT( listitemsContextMenu( const QPoint& ) ) );

}

void QgsStyleV2ManagerDialog::onFinished()
{
  if ( mModified )
  {
    mStyle->save();
  }

  QSettings settings;
  settings.setValue( "/Windows/StyleV2Manager/geometry", saveGeometry() );
  settings.setValue( "/Windows/StyleV2Manager/splitter", mSplitter->saveState() );
}

void QgsStyleV2ManagerDialog::populateTypes()
{
#if 0
  // save current selection index in types combo
  int current = ( tabItemType->count() > 0 ? tabItemType->currentIndex() : 0 );

// no counting of style items
  int markerCount = 0, lineCount = 0, fillCount = 0;

  QStringList symbolNames = mStyle->symbolNames();
  for ( int i = 0; i < symbolNames.count(); ++i )
  {
    switch ( mStyle->symbolRef( symbolNames[i] )->type() )
    {
      case QgsSymbolV2::Marker:
        markerCount++;
        break;
      case QgsSymbolV2::Line:
        lineCount++;
        break;
      case QgsSymbolV2::Fill:
        fillCount++;
        break;
      default: Q_ASSERT( 0 && "unknown symbol type" );
        break;
    }
  }

  cboItemType->clear();
  cboItemType->addItem( tr( "Marker symbol (%1)" ).arg( markerCount ), QVariant( QgsSymbolV2::Marker ) );
  cboItemType->addItem( tr( "Line symbol (%1)" ).arg( lineCount ), QVariant( QgsSymbolV2::Line ) );
  cboItemType->addItem( tr( "Fill symbol (%1)" ).arg( fillCount ), QVariant( QgsSymbolV2::Fill ) );

  cboItemType->addItem( tr( "Color ramp (%1)" ).arg( mStyle->colorRampCount() ), QVariant( 3 ) );

  // update current index to previous selection
  cboItemType->setCurrentIndex( current );
#endif
}

void QgsStyleV2ManagerDialog::on_tabItemType_currentChanged( int )
{
  // when in Color Ramp tab, add menu to add item button
  if ( currentItemType() == 3 )
  {
    QStringList rampTypes;
    rampTypes << tr( "Gradient" ) << tr( "Random" ) << tr( "ColorBrewer" );
    rampTypes << tr( "cpt-city" ); // todo, only for rasters?
    QMenu* menu = new QMenu( btnAddItem );
    foreach ( QString rampType, rampTypes )
    {
      menu->addAction( rampType );
    }
    btnAddItem->setMenu( menu );
    connect( menu, SIGNAL( triggered( QAction* ) ),
             this, SLOT( addColorRamp( QAction* ) ) );
  }
  else
  {
    if ( btnAddItem->menu() )
    {
      disconnect( btnAddItem->menu(), SIGNAL( triggered( QAction* ) ),
                  this, SLOT( addColorRamp( QAction* ) ) );
      btnAddItem->setMenu( 0 );
    }
  }

  // set icon and grid size, depending on type
  if ( currentItemType() == 1 || currentItemType() == 3 )
  {
    listItems->setIconSize( QSize( 75, 50 ) );
    listItems->setGridSize( QSize( 100, 80 ) );
  }
  else
  {
    listItems->setIconSize( QSize( 50, 50 ) );
    listItems->setGridSize( QSize( 75, 80 ) );
  }

  populateList();
}

void QgsStyleV2ManagerDialog::populateList()
{
  if ( currentItemType() > 3 )
  {
    Q_ASSERT( 0 && "not implemented" );
    return;
  }
  groupChanged( groupTree->selectionModel()->currentIndex() );
}

void QgsStyleV2ManagerDialog::populateSymbols( QStringList symbolNames, bool check )
{
  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( listItems->model() );
  model->clear();

  int type = currentItemType();

  for ( int i = 0; i < symbolNames.count(); ++i )
  {
    QString name = symbolNames[i];
    QgsSymbolV2* symbol = mStyle->symbol( name );
    if ( symbol->type() == type )
    {
      QStandardItem* item = new QStandardItem( name );
      QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( symbol, listItems->iconSize() );
      item->setIcon( icon );
      item->setData( name ); // used to find out original name when user edited the name
      item->setCheckable( check );
      item->setToolTip( name );
      // add to model
      model->appendRow( item );
    }
    delete symbol;
  }
}


void QgsStyleV2ManagerDialog::populateColorRamps( QStringList colorRamps, bool check )
{
  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( listItems->model() );
  model->clear();

  for ( int i = 0; i < colorRamps.count(); ++i )
  {
    QString name = colorRamps[i];
    QgsVectorColorRampV2* ramp = mStyle->colorRamp( name );

    QStandardItem* item = new QStandardItem( name );
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( ramp, listItems->iconSize() );
    item->setIcon( icon );
    item->setData( name ); // used to find out original name when user edited the name
    item->setCheckable( check );
    item->setToolTip( name );
    model->appendRow( item );
    delete ramp;
  }
}

int QgsStyleV2ManagerDialog::currentItemType()
{
  switch ( tabItemType->currentIndex() )
  {
    case 0: return QgsSymbolV2::Marker;
    case 1: return QgsSymbolV2::Line;
    case 2: return QgsSymbolV2::Fill;
    case 3: return 3;
    default: return 0;
  }
}

QString QgsStyleV2ManagerDialog::currentItemName()
{
  QModelIndex index = listItems->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return QString();
  return index.model()->data( index, 0 ).toString();
}

void QgsStyleV2ManagerDialog::addItem()
{
  bool changed = false;
  if ( currentItemType() < 3 )
  {
    changed = addSymbol();
  }
  else if ( currentItemType() == 3 )
  {
    changed = addColorRamp();
  }
  else
  {
    Q_ASSERT( 0 && "not implemented" );
  }

  if ( changed )
  {
    populateList();
    populateTypes();
  }
}

bool QgsStyleV2ManagerDialog::addSymbol()
{
  // create new symbol with current type
  QgsSymbolV2* symbol;
  QString name = tr( "new symbol" );
  switch ( currentItemType() )
  {
    case QgsSymbolV2::Marker:
      symbol = new QgsMarkerSymbolV2();
      name = tr( "new marker" );
      break;
    case QgsSymbolV2::Line:
      symbol = new QgsLineSymbolV2();
      name = tr( "new line" );
      break;
    case QgsSymbolV2::Fill:
      symbol = new QgsFillSymbolV2();
      name = tr( "new fill symbol" );
      break;
    default:
      Q_ASSERT( 0 && "unknown symbol type" );
      return false;
  }

  // get symbol design
  // NOTE : Set the parent widget as "this" to notify the Symbol selector
  //        that, it is being called by Style Manager, so recursive calling
  //        of style manager and symbol selector can be arrested
  //        See also: editSymbol()
  QgsSymbolV2SelectorDialog dlg( symbol, mStyle, NULL, this );
  if ( dlg.exec() == 0 )
  {
    delete symbol;
    return false;
  }

  // get unique name
  bool nameInvalid = true;

  while ( nameInvalid )
  {
    bool ok;
    name = QInputDialog::getText( this, tr( "Symbol Name" ),
                                  tr( "Please enter a name for new symbol:" ),
                                  QLineEdit::Normal, name, &ok );
    if ( !ok )
    {
      delete symbol;
      return false;
    }
    // validate name
    if ( name.isEmpty() )
    {
      QMessageBox::warning( this, tr( "Save symbol" ),
                            tr( "Cannot save symbol without name. Enter a name." ) );
    }
    else if ( mStyle->symbolNames().contains( name ) )
    {
      int res = QMessageBox::warning( this, tr( "Save symbol" ),
                                      tr( "Symbol with name '%1' already exists. Overwrite?" )
                                      .arg( name ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res == QMessageBox::Yes )
      {
        nameInvalid = false;
      }
    }
    else
    {
      // valid name
      nameInvalid = false;
    }
  }

  // add new symbol to style and re-populate the list
  mStyle->addSymbol( name, symbol, true );
  // TODO groups and tags
  mModified = true;
  return true;
}


QString QgsStyleV2ManagerDialog::addColorRampStatic( QWidget* parent, QgsStyleV2* style, QString rampType )
{
  // let the user choose the color ramp type if rampType is not given
  bool ok = true;
  if ( rampType.isEmpty() )
  {
    QStringList rampTypes;
    rampTypes << tr( "Gradient" ) << tr( "Random" ) << tr( "ColorBrewer" );
    rampTypes << tr( "cpt-city" ); // todo, only for rasters?
    rampType = QInputDialog::getItem( parent, tr( "Color ramp type" ),
                                      tr( "Please select color ramp type:" ), rampTypes, 0, false, &ok );
  }
  if ( !ok || rampType.isEmpty() )
    return QString();

  QString name = tr( "new ramp" );

  QgsVectorColorRampV2 *ramp = NULL;
  if ( rampType == tr( "Gradient" ) )
  {
    QgsVectorGradientColorRampV2* gradRamp = new QgsVectorGradientColorRampV2();
    QgsVectorGradientColorRampV2Dialog dlg( gradRamp, parent );
    if ( !dlg.exec() )
    {
      delete gradRamp;
      return QString();
    }
    ramp = gradRamp;
    name = tr( "new gradient ramp" );
  }
  else if ( rampType == tr( "Random" ) )
  {
    QgsVectorRandomColorRampV2* randRamp = new QgsVectorRandomColorRampV2();
    QgsVectorRandomColorRampV2Dialog dlg( randRamp, parent );
    if ( !dlg.exec() )
    {
      delete randRamp;
      return QString();
    }
    ramp = randRamp;
    name = tr( "new random ramp" );
  }
  else if ( rampType == tr( "ColorBrewer" ) )
  {
    QgsVectorColorBrewerColorRampV2* brewerRamp = new QgsVectorColorBrewerColorRampV2();
    QgsVectorColorBrewerColorRampV2Dialog dlg( brewerRamp, parent );
    if ( !dlg.exec() )
    {
      delete brewerRamp;
      return QString();
    }
    ramp = brewerRamp;
    name = brewerRamp->schemeName() + QString::number( brewerRamp->colors() );
  }
  else if ( rampType == tr( "cpt-city" ) )
  {
    QgsCptCityColorRampV2* cptCityRamp = new QgsCptCityColorRampV2( "", "" );
    QgsCptCityColorRampV2Dialog dlg( cptCityRamp, parent );
    if ( !dlg.exec() )
    {
      delete cptCityRamp;
      return QString();
    }
    // name = dlg.selectedName();
    name = QFileInfo( cptCityRamp->schemeName() ).baseName() + cptCityRamp->variantName();
    if ( dlg.saveAsGradientRamp() )
    {
      ramp = cptCityRamp->cloneGradientRamp();
      delete cptCityRamp;
    }
    else
    {
      ramp = cptCityRamp;
    }
  }
  else
  {
    // Q_ASSERT( 0 && "invalid ramp type" );
    // bailing out is rather harsh!
    QgsDebugMsg( "invalid ramp type " + rampType );
    return QString();
  }

  // get unique name
  bool nameInvalid = true;

  while ( nameInvalid )
  {
    bool ok;
    name = QInputDialog::getText( parent, tr( "Color Ramp Name" ),
                                  tr( "Please enter a name for new color ramp:" ),
                                  QLineEdit::Normal, name, &ok );
    if ( !ok )
    {
      delete ramp;
      return QString();
    }
    // validate name
    if ( name.isEmpty() )
    {
      QMessageBox::warning( parent, tr( "Save Color Ramp" ),
                            tr( "Cannot save color ramp without name. Enter a name." ) );
    }
    else if ( style->colorRampNames().contains( name ) )
    {
      int res = QMessageBox::warning( parent, tr( "Save color ramp" ),
                                      tr( "Color ramp with name '%1' already exists. Overwrite?" )
                                      .arg( name ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res == QMessageBox::Yes )
      {
        nameInvalid = false;
      }
    }
    else
    {
      // valid name
      nameInvalid = false;
    }
  }

  // add new symbol to style and re-populate the list
  style->addColorRamp( name, ramp, true );
  // TODO groups and tags, using saveColorRamp
  return name;
}


bool QgsStyleV2ManagerDialog::addColorRamp()
{
  return addColorRamp( 0 );
}

bool QgsStyleV2ManagerDialog::addColorRamp( QAction* action )
{
  // pass the action text, which is the color ramp type
  QString rampName = addColorRampStatic( this , mStyle,
                                         action ? action->text() : QString() );
  if ( !rampName.isEmpty() )
  {
    mModified = true;
    populateList();
    return true;
  }

  return false;
}

void QgsStyleV2ManagerDialog::editItem()
{
  bool changed = false;
  if ( currentItemType() < 3 )
  {
    changed = editSymbol();
  }
  else if ( currentItemType() == 3 )
  {
    changed = editColorRamp();
  }
  else
  {
    Q_ASSERT( 0 && "not implemented" );
  }

  if ( changed )
    populateList();
}

bool QgsStyleV2ManagerDialog::editSymbol()
{
  QString symbolName = currentItemName();
  if ( symbolName.isEmpty() )
    return false;

  QgsSymbolV2* symbol = mStyle->symbol( symbolName );

  // let the user edit the symbol and update list when done
  QgsSymbolV2SelectorDialog dlg( symbol, mStyle, NULL , this );
  if ( dlg.exec() == 0 )
  {
    delete symbol;
    return false;
  }

  // by adding symbol to style with the same name the old effectively gets overwritten
  mStyle->addSymbol( symbolName, symbol, true );
  mModified = true;
  return true;
}

bool QgsStyleV2ManagerDialog::editColorRamp()
{
  QString name = currentItemName();
  if ( name.isEmpty() )
    return false;

  QgsVectorColorRampV2* ramp = mStyle->colorRamp( name );

  if ( ramp->type() == "gradient" )
  {
    QgsVectorGradientColorRampV2* gradRamp = static_cast<QgsVectorGradientColorRampV2*>( ramp );
    QgsVectorGradientColorRampV2Dialog dlg( gradRamp, this );
    if ( !dlg.exec() )
    {
      delete ramp;
      return false;
    }
  }
  else if ( ramp->type() == "random" )
  {
    QgsVectorRandomColorRampV2* randRamp = static_cast<QgsVectorRandomColorRampV2*>( ramp );
    QgsVectorRandomColorRampV2Dialog dlg( randRamp, this );
    if ( !dlg.exec() )
    {
      delete ramp;
      return false;
    }
  }
  else if ( ramp->type() == "colorbrewer" )
  {
    QgsVectorColorBrewerColorRampV2* brewerRamp = static_cast<QgsVectorColorBrewerColorRampV2*>( ramp );
    QgsVectorColorBrewerColorRampV2Dialog dlg( brewerRamp, this );
    if ( !dlg.exec() )
    {
      delete ramp;
      return false;
    }
  }
  else if ( ramp->type() == "cpt-city" )
  {
    QgsCptCityColorRampV2* cptCityRamp = static_cast<QgsCptCityColorRampV2*>( ramp );
    QgsCptCityColorRampV2Dialog dlg( cptCityRamp, this );
    if ( !dlg.exec() )
    {
      delete ramp;
      return false;
    }
    if ( dlg.saveAsGradientRamp() )
    {
      ramp = cptCityRamp->cloneGradientRamp();
      delete cptCityRamp;
    }
  }
  else
  {
    Q_ASSERT( 0 && "invalid ramp type" );
  }

  mStyle->addColorRamp( name, ramp, true );
  mModified = true;
  return true;
}


void QgsStyleV2ManagerDialog::removeItem()
{
  bool changed = false;
  if ( currentItemType() < 3 )
  {
    changed = removeSymbol();
  }
  else if ( currentItemType() == 3 )
  {
    changed = removeColorRamp();
  }
  else
  {
    Q_ASSERT( 0 && "not implemented" );
  }

  if ( changed )
  {
    populateList();
    populateTypes();
  }
}

bool QgsStyleV2ManagerDialog::removeSymbol()
{
  QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
  foreach ( QModelIndex index, indexes )
  {
    QString symbolName = index.data().toString();
    // delete from style and update list
    if ( !symbolName.isEmpty() )
      mStyle->removeSymbol( symbolName );
  }
  mModified = true;
  return true;
}

bool QgsStyleV2ManagerDialog::removeColorRamp()
{
  QString rampName = currentItemName();
  if ( rampName.isEmpty() )
    return false;

  mStyle->removeColorRamp( rampName );
  mModified = true;
  return true;
}

void QgsStyleV2ManagerDialog::itemChanged( QStandardItem* item )
{
  // an item has been edited
  QString oldName = item->data().toString();

  bool changed = false;
  if ( currentItemType() < 3 )
  {
    changed = mStyle->renameSymbol( oldName, item->text() );
  }
  else if ( currentItemType() == 3 )
  {
    changed = mStyle->renameColorRamp( oldName, item->text() );
  }

  if ( changed )
  {
    populateList();
    mModified = true;
  }

}

void QgsStyleV2ManagerDialog::exportItems()
{
  QgsStyleV2ExportImportDialog dlg( mStyle, this, QgsStyleV2ExportImportDialog::Export );
  dlg.exec();
}

void QgsStyleV2ManagerDialog::importItems()
{
  QgsStyleV2ExportImportDialog dlg( mStyle, this, QgsStyleV2ExportImportDialog::Import );
  dlg.exec();
  populateList();
  populateGroups();
}

void QgsStyleV2ManagerDialog::setBold( QStandardItem* item )
{
  QFont font = item->font();
  font.setBold( true );
  item->setFont( font );
}

void QgsStyleV2ManagerDialog::populateGroups()
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel*>( groupTree->model() );
  model->clear();

  QStandardItem *allSymbols = new QStandardItem( tr( "All Symbols" ) );
  allSymbols->setData( "all" );
  allSymbols->setEditable( false );
  setBold( allSymbols );
  model->appendRow( allSymbols );

  QStandardItem *group = new QStandardItem( "" ); //require empty name to get first order groups
  group->setData( "groups" );
  group->setEditable( false );
  buildGroupTree( group );
  group->setText( tr( "Groups" ) );//set title later
  QStandardItem *ungrouped = new QStandardItem( tr( "Ungrouped" ) );
  ungrouped->setData( 0 );
  setBold( ungrouped );
  setBold( group );
  group->appendRow( ungrouped );
  model->appendRow( group );

  QStandardItem *tag = new QStandardItem( tr( "Smart Groups" ) );
  tag->setData( "smartgroups" );
  tag->setEditable( false );
  setBold( tag );
  QgsSymbolGroupMap sgMap = mStyle->smartgroupsListMap();
  QgsSymbolGroupMap::const_iterator i = sgMap.constBegin();
  while ( i != sgMap.constEnd() )
  {
    QStandardItem *item = new QStandardItem( i.value() );
    item->setData( i.key() );
    tag->appendRow( item );
    ++i;
  }
  model->appendRow( tag );

  // expand things in the grouo tree
  int rows = model->rowCount( model->indexFromItem( model->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    groupTree->setExpanded( model->indexFromItem( model->item( i ) ), true );
  }
}

void QgsStyleV2ManagerDialog::buildGroupTree( QStandardItem* &parent )
{
  QgsSymbolGroupMap groups = mStyle->childGroupNames( parent->text() );
  QgsSymbolGroupMap::const_iterator i = groups.constBegin();
  while ( i != groups.constEnd() )
  {
    QStandardItem *item = new QStandardItem( i.value() );
    item->setData( i.key() );
    parent->appendRow( item );
    buildGroupTree( item );
    ++i;
  }
}

void QgsStyleV2ManagerDialog::groupChanged( const QModelIndex& index )
{
  QStringList symbolNames;
  QStringList groupSymbols;

  QgsStyleV2::StyleEntity type = currentItemType() < 3 ? QgsStyleV2::SymbolEntity : QgsStyleV2::ColorrampEntity;
  if ( currentItemType() > 3 )
  {
    QgsDebugMsg( "Entity not implemented" );
    return;
  }

  QString category = index.data( Qt::UserRole + 1 ).toString();
  if ( category == "all" || category == "groups" || category == "smartgroups" )
  {
    enableGroupInputs( false );
    if ( category == "groups" || category == "smartgroups" )
    {
      btnAddGroup->setEnabled( true );
    }
    symbolNames = currentItemType() < 3 ? mStyle->symbolNames() : mStyle->colorRampNames();
  }
  else
  {
    //determine groups and tags
    if ( index.parent().data( Qt::UserRole + 1 ) == "smartgroups" )
    {
      btnAddGroup->setEnabled( false );
      btnRemoveGroup->setEnabled( true );
      btnManageGroups->setEnabled( true );
      int groupId = index.data( Qt::UserRole + 1 ).toInt();
      symbolNames = mStyle->symbolsOfSmartgroup( type, groupId );
    }
    else // then it must be a group
    {
      if (( !index.data( Qt::UserRole + 1 ).toInt() && ( index.data() == "Ungrouped" ) ) || mGrouppingMode )
        enableGroupInputs( false );
      else
        enableGroupInputs( true );
      int groupId = index.data( Qt::UserRole + 1 ).toInt();
      symbolNames = mStyle->symbolsOfGroup( type, groupId );
      if ( mGrouppingMode && groupId )
      {
        groupSymbols = symbolNames;
        symbolNames += mStyle->symbolsOfGroup( type, 0 );
      }
    }
  }

  if ( currentItemType() < 3 )
  {
    populateSymbols( symbolNames, mGrouppingMode );
  }
  else if ( currentItemType() == 3 )
  {
    populateColorRamps( symbolNames, mGrouppingMode );
  }
  if ( mGrouppingMode )
    setSymbolsChecked( groupSymbols );
}

void QgsStyleV2ManagerDialog::addGroup()
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel*>( groupTree->model() );
  QModelIndex parentIndex = groupTree->currentIndex();

  // Violation 1: Creating sub-groups of system defined groups
  QString parentData = parentIndex.data( Qt::UserRole + 1 ).toString();
  if ( parentData == "all" || ( parentIndex.data() == "Ungrouped" && parentData == "0" ) )
  {
    int err = QMessageBox::critical( this, tr( "Invalid Selection" ),
                                     tr( "The parent group you have selected is not user editable.\n"
                                         "Kindly select a user defined group." ) );
    if ( err )
      return;
  }

  // Violation 2: Creating a nested tag
  if ( parentIndex.parent().data( Qt::UserRole + 1 ).toString() == "smartgroups" )
  {
    int err = QMessageBox::critical( this, tr( "Operation Not Allowed" ),
                                     tr( "Creation of nested smart groups are not allowed\n"
                                         "Select the 'Smart Group' to create a new group." ) );
    if ( err )
      return;
  }

  QString itemName;
  QVariant itemData;
  bool isGroup = true;

  // create a smart group if that is selected
  if ( parentIndex.data( Qt::UserRole + 1 ).toString() == "smartgroups" )
  {
    QgsSmartGroupEditorDialog dlg( mStyle, this );
    if ( dlg.exec() == QDialog::Rejected )
      return;
    int id = mStyle->addSmartgroup( dlg.smartgroupName(), dlg.conditionOperator(), dlg.conditionMap() );
    if ( !id )
      return;
    itemData = QVariant( id );
    itemName = dlg.smartgroupName();
    isGroup = false;
  }
  else
  {
    itemName = QString( tr( "New Group" ) );
    itemData = QVariant( "newgroup" );
  }

  // Else create a simple child-group to the selected
  QStandardItem *parentItem = model->itemFromIndex( parentIndex );
  QStandardItem *childItem = new QStandardItem( itemName );
  childItem->setData( itemData );
  parentItem->appendRow( childItem );

  groupTree->setCurrentIndex( childItem->index() );
  if ( isGroup )
  {
    groupTree->edit( childItem->index() );
  }
}

void QgsStyleV2ManagerDialog::removeGroup()
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel*>( groupTree->model() );
  QModelIndex index = groupTree->currentIndex();

  // Violation: removing system groups
  QString data = index.data( Qt::UserRole + 1 ).toString();
  if ( data == "all" || data == "groups" || data == "smartgroups" || index.data() == "Ungrouped" )
  {
    int err = QMessageBox::critical( this, tr( "Invalid selection" ),
                                     tr( "Cannot delete system defined categories.\n"
                                         "Kindly select a group or smart group you might want to delete." ) );
    if ( err )
      return;
  }

  QStandardItem *parentItem = model->itemFromIndex( index.parent() );
  if ( parentItem->data( Qt::UserRole + 1 ).toString() == "smartgroups" )
  {
    mStyle->remove( QgsStyleV2::SmartgroupEntity, index.data( Qt::UserRole + 1 ).toInt() );
  }
  else
  {
    mStyle->remove( QgsStyleV2::GroupEntity, index.data( Qt::UserRole + 1 ).toInt() );
    QStandardItem *item = model->itemFromIndex( index );
    if ( item->hasChildren() )
    {
      QStandardItem *parent = item->parent();
      for ( int i = 0; i < item->rowCount(); i++ )
      {
        parent->appendRow( item->takeChild( i ) );
      }
    }
  }
  parentItem->removeRow( index.row() );
}

void QgsStyleV2ManagerDialog::groupRenamed( QStandardItem * item )
{
  QString data = item->data( Qt::UserRole + 1 ).toString();
  QgsDebugMsg( "Symbol group edited: data=" + data + " text=" + item->text() );
  if ( data == "newgroup" )
  {
    int id;
    if ( item->parent()->data( Qt::UserRole + 1 ).toString() == "groups" )
    {
      id = mStyle->addGroup( item->text() );
    }
    else
    {
      int parentid = item->parent()->data( Qt::UserRole + 1 ).toInt();
      id = mStyle->addGroup( item->text(), parentid );
    }
    if ( !id )
    {
      QMessageBox::critical( this, tr( "Error!" ),
                             tr( "New group could not be created.\n"
                                 "There was a problem with your symbol database." ) );
      item->parent()->removeRow( item->row() );
      return;
    }
    else
    {
      item->setData( id );
    }
  }
  else
  {
    int id = item->data( Qt::UserRole + 1 ).toInt();
    QString name = item->text();
    if ( item->parent()->data( Qt::UserRole + 1 ) == "smartgroups" )
    {
      mStyle->rename( QgsStyleV2::SmartgroupEntity, id, name );
    }
    else
    {
      mStyle->rename( QgsStyleV2::GroupEntity, id, name );
    }
  }
}

void QgsStyleV2ManagerDialog::groupSymbolsAction()
{

  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel*>( groupTree->model() );
  QStandardItemModel *model = qobject_cast<QStandardItemModel*>( listItems->model() );
  QAction *senderAction = qobject_cast<QAction*>( sender() );

  if ( mGrouppingMode )
  {
    mGrouppingMode = false;
    senderAction->setText( tr( "Group Symbols" ) );
    // disconnect slot which handles regrouping
    disconnect( model, SIGNAL( itemChanged( QStandardItem* ) ),
                this, SLOT( regrouped( QStandardItem* ) ) );

    // disabel all items except groups in groupTree
    enableItemsForGroupingMode( true );
    groupChanged( groupTree->currentIndex() );

    // Finally: Reconnect all Symbol editing functionalities
    connect( treeModel, SIGNAL( itemChanged( QStandardItem* ) ),
             this, SLOT( groupRenamed( QStandardItem* ) ) );
    connect( model, SIGNAL( itemChanged( QStandardItem* ) ),
             this, SLOT( itemChanged( QStandardItem* ) ) );
    // Reset the selection mode
    listItems->setSelectionMode( QAbstractItemView::ExtendedSelection );
  }
  else
  {
    bool validGroup = false;
    // determine whether it is a valid group
    QModelIndex present = groupTree->currentIndex();
    while ( present.parent().isValid() )
    {
      if ( present.parent().data() == "Groups" )
      {
        validGroup = true;
        break;
      }
      else
        present = present.parent();
    }
    if ( !validGroup )
      return;

    mGrouppingMode = true;
    // Change the text menu
    senderAction->setText( tr( "Finish Grouping" ) );
    // Remove all Symbol editing functionalities
    disconnect( treeModel, SIGNAL( itemChanged( QStandardItem* ) ),
                this, SLOT( groupRenamed( QStandardItem* ) ) );
    disconnect( model, SIGNAL( itemChanged( QStandardItem* ) ),
                this, SLOT( itemChanged( QStandardItem* ) ) );

    // disabel all items except groups in groupTree
    enableItemsForGroupingMode( false );
    groupChanged( groupTree->currentIndex() );
    btnManageGroups->setEnabled( true );


    // Connect to slot which handles regrouping
    connect( model, SIGNAL( itemChanged( QStandardItem* ) ),
             this, SLOT( regrouped( QStandardItem* ) ) );

    // No selection should be possible
    listItems->setSelectionMode( QAbstractItemView::NoSelection );
  }
}

void QgsStyleV2ManagerDialog::regrouped( QStandardItem *item )
{
  QgsStyleV2::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyleV2::SymbolEntity : QgsStyleV2::ColorrampEntity;
  if ( currentItemType() > 3 )
  {
    QgsDebugMsg( "Unknown style entity" );
    return;
  }
  int groupid = groupTree->currentIndex().data( Qt::UserRole + 1 ).toInt();
  QString symbolName = item->text();
  bool regrouped;
  if ( item->checkState() == Qt::Checked )
    regrouped = mStyle->group( type, symbolName, groupid );
  else
    regrouped = mStyle->group( type, symbolName, 0 );
  if ( !regrouped )
  {
    int er = QMessageBox::critical( this, tr( "Database Error" ),
                                    tr( "There was a problem with the Symbols database while regrouping." ) );
    // call the slot again to get back to normal
    if ( er )
      groupSymbolsAction();
  }
}

void QgsStyleV2ManagerDialog::setSymbolsChecked( QStringList symbols )
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel*>( listItems->model() );
  foreach ( const QString symbol, symbols )
  {
    QList<QStandardItem*> items = model->findItems( symbol );
    foreach ( QStandardItem* item, items )
      item->setCheckState( Qt::Checked );
  }
}

void QgsStyleV2ManagerDialog::filterSymbols( QString qword )
{
  QStringList items;
  if ( currentItemType() == 3 )
  {
    items = mStyle->findSymbols( QgsStyleV2::ColorrampEntity, qword );
    populateColorRamps( items );
  }
  else
  {
    items = mStyle->findSymbols( QgsStyleV2::SymbolEntity, qword );
    populateSymbols( items );
  }
}

void QgsStyleV2ManagerDialog::tagsChanged()
{
  QModelIndexList indexes =  listItems->selectionModel()->selection().indexes();
  QStringList addtags;
  QStringList removetags;

  QStringList oldtags = mTagList;
  QStringList newtags = tagsLineEdit->text().split( ",", QString::SkipEmptyParts );

  QgsStyleV2::StyleEntity type;
  if ( currentItemType() < 3 )
  {
    type = QgsStyleV2::SymbolEntity;
  }
  else if ( currentItemType() == 3 )
  {
    type = QgsStyleV2::ColorrampEntity;
  }
  else
  {
    QgsDebugMsg( "Unknown Style Entity!" );
    return;
  }
  // compare old with new to find removed tags
  foreach ( const QString &tag, oldtags )
  {
    if ( !newtags.contains( tag ) )
      removetags.append( tag );
  }
  if ( removetags.size() > 0 )
  {
    foreach ( QModelIndex index, indexes )
    {
      mStyle->detagSymbol( type, index.data().toString(), removetags );
    }
  }
  // compare new with old to find added tags
  foreach ( const QString &tag, newtags )
  {
    if ( !oldtags.contains( tag ) )
      addtags.append( tag );
  }
  if ( addtags.size() > 0 )
  {
    foreach ( QModelIndex index, indexes )
    {
      mStyle->tagSymbol( type, index.data().toString(), addtags );
    }
  }
}

void QgsStyleV2ManagerDialog::symbolSelected( const QModelIndex& index )
{
  // Populate the tags for the symbol
  tagsLineEdit->clear();
  QStandardItem *item = static_cast<QStandardItemModel*>( listItems->model() )->itemFromIndex( index );
  QgsStyleV2::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyleV2::SymbolEntity : QgsStyleV2::ColorrampEntity;
  mTagList = mStyle->tagsOfSymbol( type, item->data().toString() );
  tagsLineEdit->setText( mTagList.join( "," ) );
}

void QgsStyleV2ManagerDialog::enableSymbolInputs( bool enable )
{
  groupTree->setEnabled( enable );
  btnAddGroup->setEnabled( enable );
  btnRemoveGroup->setEnabled( enable );
  btnManageGroups->setEnabled( enable );
  searchBox->setEnabled( enable );
  tagsLineEdit->setEnabled( enable );
}

void QgsStyleV2ManagerDialog::enableGroupInputs( bool enable )
{
  btnAddGroup->setEnabled( enable );
  btnRemoveGroup->setEnabled( enable );
  btnManageGroups->setEnabled( enable );
}

void QgsStyleV2ManagerDialog::enableItemsForGroupingMode( bool enable )
{
  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel*>( groupTree->model() );
  for ( int i = 0; i < treeModel->rowCount(); i++ )
  {
    if ( treeModel->item( i )->data() != "groups" )
    {
      treeModel->item( i )->setEnabled( enable );
    }
    if ( treeModel->item( i )->data() == "groups" )
    {
      treeModel->item( i )->setEnabled( enable );
      for ( int k = 0; k < treeModel->item( i )->rowCount(); k++ )
      {
        if ( !treeModel->item( i )->child( k )->data().toInt() )
          treeModel->item( i )->child( k )->setEnabled( enable );
      }
    }
    if ( treeModel->item( i )->data() == "smartgroups" )
    {
      for ( int j = 0; j < treeModel->item( i )->rowCount(); j++ )
      {
        treeModel->item( i )->child( j )->setEnabled( enable );
      }
    }
  }

  // The buttons
  // NOTE: if you ever change the layout name in the .ui file edit here too
  for ( int i = 0; i < symbolBtnsLayout->count(); i++ )
  {
    symbolBtnsLayout->itemAt( i )->widget()->setEnabled( enable );
  }

}

void QgsStyleV2ManagerDialog::grouptreeContextMenu( const QPoint& point )
{
  QPoint globalPos = groupTree->viewport()->mapToGlobal( point );

  QModelIndex index = groupTree->indexAt( point );
  QgsDebugMsg( "Now you clicked: " + index.data().toString() );

  QMenu groupMenu;

  if ( index.parent().isValid() && ( index.data().toString() != "Ungrouped" ) )
  {
    if ( index.parent().data( Qt::UserRole + 1 ).toString() == "smartgroups" )
    {
      groupMenu.addAction( tr( "Edit Group" ) );
    }
    else
    {
      groupMenu.addAction( tr( "Add Group" ) );
    }
    groupMenu.addAction( tr( "Remove Group" ) );
  }
  else if ( index.data( Qt::UserRole + 1 ) == "groups" || index.data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    groupMenu.addAction( tr( "Add Group" ) );
  }


  QAction* selectedItem = groupMenu.exec( globalPos );

  if ( selectedItem )
  {
    if ( selectedItem->text() == tr( "Add Group" ) )
      addGroup();
    else if ( selectedItem->text() == tr( "Remove Group" ) )
      removeGroup();
    else if ( selectedItem->text() == tr( "Edit Group" ) )
      editSmartgroupAction();
  }
}

void QgsStyleV2ManagerDialog::listitemsContextMenu( const QPoint& point )
{
  QPoint globalPos = listItems->viewport()->mapToGlobal( point );

  QMenu *groupMenu = new QMenu( this );
  QMenu *groupList = new QMenu( this );
  groupList->setTitle( tr( "Apply Group" ) );

  QStringList groups = mStyle->groupNames();
  foreach ( QString group, groups )
  {
    groupList->addAction( group );
  }
  groupMenu->addMenu( groupList );
  groupMenu->addAction( tr( "Un-group" ) );

  QAction* selectedItem = groupMenu->exec( globalPos );

  if ( selectedItem )
  {
    QgsStyleV2::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyleV2::SymbolEntity : QgsStyleV2::ColorrampEntity;
    if ( currentItemType() > 3 )
    {
      QgsDebugMsg( "unknow entity type" );
      return;
    }
    int groupId = 0;
    if ( selectedItem->text() != tr( "Un-group" ) )
    {
      groupId = mStyle->groupId( selectedItem->text() );
    }
    QModelIndexList indexes =  listItems->selectionModel()->selectedIndexes();
    foreach ( QModelIndex index, indexes )
    {
      mStyle->group( type, index.data().toString(), groupId );
    }
    populateList();

    QgsDebugMsg( "Selected Action: " + selectedItem->text() );
  }
}

void QgsStyleV2ManagerDialog::editSmartgroupAction()
{
  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel*>( groupTree->model() );

  // determine whether it is a valid group
  QModelIndex present = groupTree->currentIndex();
  if ( present.parent().data( Qt::UserRole + 1 ) != "smartgroups" )
  {
    QMessageBox::critical( this, tr( "Invalid Selection" ),
                           tr( "You have not selected a Smart Group. Kindly select a Smart Group to edit." ) );
    return;
  }
  QStandardItem* item = treeModel->itemFromIndex( present );

  QgsSmartGroupEditorDialog dlg( mStyle, this );
  QgsSmartConditionMap map = mStyle->smartgroup( present.data( Qt::UserRole + 1 ).toInt() );
  dlg.setSmartgroupName( item->text() );
  dlg.setOperator( mStyle->smartgroupOperator( item->data().toInt() ) );
  dlg.setConditionMap( map );

  if ( dlg.exec() == QDialog::Rejected )
    return;

  mStyle->remove( QgsStyleV2::SmartgroupEntity, item->data().toInt() );
  int id = mStyle->addSmartgroup( dlg.smartgroupName(), dlg.conditionOperator(), dlg.conditionMap() );
  if ( !id )
  {
    QMessageBox::critical( this, tr( "Database Error!" ),
                           tr( "There was some error while editing the smart group." ) );
    return;
  }
  item->setText( dlg.smartgroupName() );
  item->setData( id );

  groupChanged( present );
}

bool QgsStyleV2ManagerDialog::eventFilter( QObject *obj, QEvent *event )
{

  if (( obj == tagsLineEdit ) && ( event->type() == QEvent::FocusOut ) )
  {
    tagsChanged();
    return true;
  }
  return false;
}

