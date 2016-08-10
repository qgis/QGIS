/***************************************************************************
    qgsstylemanagerdialog.cpp
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

#include "qgsstylemanagerdialog.h"

#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorcolorramp.h"

#include "qgssymbolselectordialog.h"
#include "qgsvectorgradientcolorrampdialog.h"
#include "qgsvectorrandomcolorrampdialog.h"
#include "qgsvectorcolorbrewercolorrampdialog.h"
#include "qgscptcitycolorrampdialog.h"
#include "qgsstyleexportimportdialog.h"
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

QgsStyleManagerDialog::QgsStyleManagerDialog( QgsStyle* style, QWidget* parent )
    : QDialog( parent )
    , mStyle( style )
    , mModified( false )
{
  setupUi( this );
#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  QSettings settings;

  restoreGeometry( settings.value( "/Windows/StyleV2Manager/geometry" ).toByteArray() );
  mSplitter->setSizes( QList<int>() << 170 << 540 );
  mSplitter->restoreState( settings.value( "/Windows/StyleV2Manager/splitter" ).toByteArray() );

  tabItemType->setDocumentMode( true );
  searchBox->setPlaceholderText( tr( "Type here to filter symbols..." ) );

  connect( this, SIGNAL( finished( int ) ), this, SLOT( onFinished() ) );

  connect( listItems, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( editItem() ) );

  connect( btnAddItem, SIGNAL( clicked() ), this, SLOT( addItem() ) );
  connect( actnEditItem, SIGNAL( triggered( bool ) ), this, SLOT( editItem() ) );
  connect( actnRemoveItem, SIGNAL( triggered( bool ) ), this, SLOT( removeItem() ) );

  btnRemoveItem->setDefaultAction( actnRemoveItem );
  btnEditItem->setDefaultAction( actnEditItem );

  QMenu *shareMenu = new QMenu( tr( "Share menu" ), this );
  shareMenu->addAction( actnExportAsPNG );
  shareMenu->addAction( actnExportAsSVG );
  QAction *exportAction = new QAction( tr( "Export..." ), this );
  shareMenu->addAction( exportAction );
  QAction *importAction = new QAction( tr( "Import..." ), this );
  shareMenu->addAction( importAction );
  exportAction->setIcon( QIcon( QgsApplication::iconPath( "mActionFileSave.svg" ) ) );
  importAction->setIcon( QIcon( QgsApplication::iconPath( "mActionFileOpen.svg" ) ) );
  connect( actnExportAsPNG, SIGNAL( triggered() ), this, SLOT( exportItemsPNG() ) );
  connect( actnExportAsSVG, SIGNAL( triggered() ), this, SLOT( exportItemsSVG() ) );
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
  connect( listItems->selectionModel(), SIGNAL( selectionChanged( const QItemSelection, const QItemSelection ) ),
           this, SLOT( selectedSymbolsChanged( const QItemSelection&, const QItemSelection& ) ) );

  populateTypes();

  QStandardItemModel* groupModel = new QStandardItemModel( groupTree );
  groupTree->setModel( groupModel );
  groupTree->setHeaderHidden( true );
  populateGroups();
  connect( groupTree->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( groupChanged( const QModelIndex& ) ) );
  connect( groupModel, SIGNAL( itemChanged( QStandardItem* ) ),
           this, SLOT( groupRenamed( QStandardItem* ) ) );

  QMenu *groupMenu = new QMenu( tr( "Group actions" ), this );
  connect( actnGroupSymbols, SIGNAL( triggered() ), this, SLOT( groupSymbolsAction() ) );
  groupMenu->addAction( actnGroupSymbols );
  connect( actnFinishGrouping, SIGNAL( triggered() ), this, SLOT( groupSymbolsAction() ) );
  actnFinishGrouping->setVisible( false );
  groupMenu->addAction( actnFinishGrouping );
  groupMenu->addAction( actnEditSmartGroup );
  btnManageGroups->setMenu( groupMenu );

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

  // Menu for the "Add item" toolbutton when in colorramp mode
  QStringList rampTypes;
  rampTypes << tr( "Gradient" ) << tr( "Random" ) << tr( "ColorBrewer" );
  rampTypes << tr( "cpt-city" ); // todo, only for rasters?
  mMenuBtnAddItemColorRamp = new QMenu( this );
  Q_FOREACH ( const QString& rampType, rampTypes )
    mMenuBtnAddItemColorRamp->addAction( new QAction( rampType, this ) );
  connect( mMenuBtnAddItemColorRamp, SIGNAL( triggered( QAction* ) ),
           this, SLOT( addColorRamp( QAction* ) ) );

  // Context menu for symbols/colorramps. The menu entries for every group are created when displaying the menu.
  mGroupMenu = new QMenu( this );
  mGroupListMenu = new QMenu( mGroupMenu );
  mGroupListMenu->setTitle( tr( "Add to group" ) );
  mGroupListMenu->setEnabled( false );
  mGroupMenu->addMenu( mGroupListMenu );
  actnUngroup->setData( 0 );
  connect( actnUngroup, SIGNAL( triggered( bool ) ), this, SLOT( groupSelectedSymbols() ) );
  mGroupMenu->addAction( actnUngroup );
  mGroupMenu->addSeparator()->setParent( this );
  mGroupMenu->addAction( actnRemoveItem );
  mGroupMenu->addAction( actnEditItem );
  mGroupMenu->addSeparator()->setParent( this );
  mGroupMenu->addAction( actnExportAsPNG );
  mGroupMenu->addAction( actnExportAsSVG );

  // Context menu for the group tree
  mGroupTreeContextMenu = new QMenu( this );
  connect( actnEditSmartGroup, SIGNAL( triggered( bool ) ), this, SLOT( editSmartgroupAction() ) );
  mGroupTreeContextMenu->addAction( actnEditSmartGroup );
  connect( actnAddGroup, SIGNAL( triggered( bool ) ), this, SLOT( addGroup() ) );
  mGroupTreeContextMenu->addAction( actnAddGroup );
  connect( actnRemoveGroup, SIGNAL( triggered( bool ) ), this, SLOT( removeGroup() ) );
  mGroupTreeContextMenu->addAction( actnRemoveGroup );

  on_tabItemType_currentChanged( 0 );
}

void QgsStyleManagerDialog::onFinished()
{
  if ( mModified )
  {
    mStyle->save();
  }

  QSettings settings;
  settings.setValue( "/Windows/StyleV2Manager/geometry", saveGeometry() );
  settings.setValue( "/Windows/StyleV2Manager/splitter", mSplitter->saveState() );
}

void QgsStyleManagerDialog::populateTypes()
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
      case QgsSymbol::Marker:
        markerCount++;
        break;
      case QgsSymbol::Line:
        lineCount++;
        break;
      case QgsSymbol::Fill:
        fillCount++;
        break;
      default:
        Q_ASSERT( 0 && "unknown symbol type" );
        break;
    }
  }

  cboItemType->clear();
  cboItemType->addItem( tr( "Marker symbol (%1)" ).arg( markerCount ), QVariant( QgsSymbol::Marker ) );
  cboItemType->addItem( tr( "Line symbol (%1)" ).arg( lineCount ), QVariant( QgsSymbol::Line ) );
  cboItemType->addItem( tr( "Fill symbol (%1)" ).arg( fillCount ), QVariant( QgsSymbol::Fill ) );

  cboItemType->addItem( tr( "Color ramp (%1)" ).arg( mStyle->colorRampCount() ), QVariant( 3 ) );

  // update current index to previous selection
  cboItemType->setCurrentIndex( current );
#endif
}

void QgsStyleManagerDialog::on_tabItemType_currentChanged( int )
{
  // when in Color Ramp tab, add menu to add item button and hide "Export symbols as PNG/SVG"
  bool flag = currentItemType() != 3;
  btnAddItem->setMenu( flag ? nullptr : mMenuBtnAddItemColorRamp );
  actnExportAsPNG->setVisible( flag );
  actnExportAsSVG->setVisible( flag );

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

void QgsStyleManagerDialog::populateList()
{
  if ( currentItemType() > 3 )
  {
    Q_ASSERT( 0 && "not implemented" );
    return;
  }
  groupChanged( groupTree->selectionModel()->currentIndex() );
}

void QgsStyleManagerDialog::populateSymbols( const QStringList& symbolNames, bool check )
{
  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( listItems->model() );
  model->clear();

  int type = currentItemType();

  for ( int i = 0; i < symbolNames.count(); ++i )
  {
    QString name = symbolNames[i];
    QgsSymbol* symbol = mStyle->symbol( name );
    if ( symbol && symbol->type() == type )
    {
      QStandardItem* item = new QStandardItem( name );
      QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol, listItems->iconSize() );
      item->setIcon( icon );
      item->setData( name ); // used to find out original name when user edited the name
      item->setCheckable( check );
      item->setToolTip( name );
      // add to model
      model->appendRow( item );
    }
    delete symbol;
  }
  selectedSymbolsChanged( QItemSelection(), QItemSelection() );
  symbolSelected( listItems->currentIndex() );
}


void QgsStyleManagerDialog::populateColorRamps( const QStringList& colorRamps, bool check )
{
  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( listItems->model() );
  model->clear();

  for ( int i = 0; i < colorRamps.count(); ++i )
  {
    QString name = colorRamps[i];
    QgsVectorColorRamp* ramp = mStyle->colorRamp( name );

    QStandardItem* item = new QStandardItem( name );
    QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp, listItems->iconSize() );
    item->setIcon( icon );
    item->setData( name ); // used to find out original name when user edited the name
    item->setCheckable( check );
    item->setToolTip( name );
    model->appendRow( item );
    delete ramp;
  }
  selectedSymbolsChanged( QItemSelection(), QItemSelection() );
  symbolSelected( listItems->currentIndex() );
}

int QgsStyleManagerDialog::currentItemType()
{
  switch ( tabItemType->currentIndex() )
  {
    case 0:
      return QgsSymbol::Marker;
    case 1:
      return QgsSymbol::Line;
    case 2:
      return QgsSymbol::Fill;
    case 3:
      return 3;
    default:
      return 0;
  }
}

QString QgsStyleManagerDialog::currentItemName()
{
  QModelIndex index = listItems->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return QString();
  return index.model()->data( index, 0 ).toString();
}

void QgsStyleManagerDialog::addItem()
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

bool QgsStyleManagerDialog::addSymbol()
{
  // create new symbol with current type
  QgsSymbol* symbol;
  QString name = tr( "new symbol" );
  switch ( currentItemType() )
  {
    case QgsSymbol::Marker:
      symbol = new QgsMarkerSymbol();
      name = tr( "new marker" );
      break;
    case QgsSymbol::Line:
      symbol = new QgsLineSymbol();
      name = tr( "new line" );
      break;
    case QgsSymbol::Fill:
      symbol = new QgsFillSymbol();
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
  QgsSymbolSelectorDialog dlg( symbol, mStyle, nullptr, this );
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


QString QgsStyleManagerDialog::addColorRampStatic( QWidget* parent, QgsStyle* style, QString rampType )
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

  QgsVectorColorRamp *ramp = nullptr;
  if ( rampType == tr( "Gradient" ) )
  {
    QgsVectorGradientColorRamp* gradRamp = new QgsVectorGradientColorRamp();
    QgsVectorGradientColorRampDialog dlg( gradRamp, parent );
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
    QgsVectorRandomColorRamp* randRamp = new QgsVectorRandomColorRamp();
    QgsVectorRandomColorRampDialog dlg( randRamp, parent );
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
    QgsVectorColorBrewerColorRamp* brewerRamp = new QgsVectorColorBrewerColorRamp();
    QgsVectorColorBrewerColorRampDialog dlg( brewerRamp, parent );
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
    QgsCptCityColorRamp* cptCityRamp = new QgsCptCityColorRamp( "", "" );
    QgsCptCityColorRampDialog dlg( cptCityRamp, parent );
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


bool QgsStyleManagerDialog::addColorRamp()
{
  return addColorRamp( nullptr );
}

bool QgsStyleManagerDialog::addColorRamp( QAction* action )
{
  // pass the action text, which is the color ramp type
  QString rampName = addColorRampStatic( this, mStyle,
                                         action ? action->text() : QString() );
  if ( !rampName.isEmpty() )
  {
    mModified = true;
    populateList();
    return true;
  }

  return false;
}

void QgsStyleManagerDialog::editItem()
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

bool QgsStyleManagerDialog::editSymbol()
{
  QString symbolName = currentItemName();
  if ( symbolName.isEmpty() )
    return false;

  QgsSymbol* symbol = mStyle->symbol( symbolName );

  // let the user edit the symbol and update list when done
  QgsSymbolSelectorDialog dlg( symbol, mStyle, nullptr, this );
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

bool QgsStyleManagerDialog::editColorRamp()
{
  QString name = currentItemName();
  if ( name.isEmpty() )
    return false;

  QgsVectorColorRamp* ramp = mStyle->colorRamp( name );

  if ( ramp->type() == "gradient" )
  {
    QgsVectorGradientColorRamp* gradRamp = static_cast<QgsVectorGradientColorRamp*>( ramp );
    QgsVectorGradientColorRampDialog dlg( gradRamp, this );
    if ( !dlg.exec() )
    {
      delete ramp;
      return false;
    }
  }
  else if ( ramp->type() == "random" )
  {
    QgsVectorRandomColorRamp* randRamp = static_cast<QgsVectorRandomColorRamp*>( ramp );
    QgsVectorRandomColorRampDialog dlg( randRamp, this );
    if ( !dlg.exec() )
    {
      delete ramp;
      return false;
    }
  }
  else if ( ramp->type() == "colorbrewer" )
  {
    QgsVectorColorBrewerColorRamp* brewerRamp = static_cast<QgsVectorColorBrewerColorRamp*>( ramp );
    QgsVectorColorBrewerColorRampDialog dlg( brewerRamp, this );
    if ( !dlg.exec() )
    {
      delete ramp;
      return false;
    }
  }
  else if ( ramp->type() == "cpt-city" )
  {
    QgsCptCityColorRamp* cptCityRamp = static_cast<QgsCptCityColorRamp*>( ramp );
    QgsCptCityColorRampDialog dlg( cptCityRamp, this );
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


void QgsStyleManagerDialog::removeItem()
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

bool QgsStyleManagerDialog::removeSymbol()
{
  QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm removal" ),
       QString( tr( "Do you really want to remove %n symbol(s)?", nullptr, indexes.count() ) ),
       QMessageBox::Yes,
       QMessageBox::No ) )
    return false;

  Q_FOREACH ( const QModelIndex& index, indexes )
  {
    QString symbolName = index.data().toString();
    // delete from style and update list
    if ( !symbolName.isEmpty() )
      mStyle->removeSymbol( symbolName );
  }
  mModified = true;
  return true;
}

bool QgsStyleManagerDialog::removeColorRamp()
{
  QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm removal" ),
       QString( tr( "Do you really want to remove %n ramps(s)?", nullptr, indexes.count() ) ),
       QMessageBox::Yes,
       QMessageBox::No ) )
    return false;

  Q_FOREACH ( const QModelIndex& index, indexes )
  {
    QString rampName = index.data().toString();
    // delete from style and update list
    if ( !rampName.isEmpty() )
      mStyle->removeColorRamp( rampName );
  }
  mModified = true;
  return true;
}

void QgsStyleManagerDialog::itemChanged( QStandardItem* item )
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
  else
  {
    QMessageBox::critical( this, tr( "Cannot rename item" ),
                           tr( "Name is already taken by another item. Choose a different name." ) );
    item->setText( oldName );
  }
}

void QgsStyleManagerDialog::exportItemsPNG()
{
  QString dir = QFileDialog::getExistingDirectory( this, tr( "Exported selected symbols as PNG" ),
                QDir::home().absolutePath(),
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks );
  exportSelectedItemsImages( dir, "png", QSize( 32, 32 ) );
}

void QgsStyleManagerDialog::exportItemsSVG()
{
  QString dir = QFileDialog::getExistingDirectory( this, tr( "Exported selected symbols as SVG" ),
                QDir::home().absolutePath(),
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks );
  exportSelectedItemsImages( dir, "svg", QSize( 32, 32 ) );
}


void QgsStyleManagerDialog::exportSelectedItemsImages( const QString& dir, const QString& format, QSize size )
{
  if ( dir.isEmpty() )
    return;

  QModelIndexList indexes =  listItems->selectionModel()->selection().indexes();
  Q_FOREACH ( const QModelIndex& index, indexes )
  {
    QString name = index.data().toString();
    QString path = dir + '/' + name + '.' + format;
    QgsSymbol *sym = mStyle->symbol( name );
    sym->exportImage( path, format, size );
  }
}

void QgsStyleManagerDialog::exportItems()
{
  QgsStyleExportImportDialog dlg( mStyle, this, QgsStyleExportImportDialog::Export );
  dlg.exec();
}

void QgsStyleManagerDialog::importItems()
{
  QgsStyleExportImportDialog dlg( mStyle, this, QgsStyleExportImportDialog::Import );
  dlg.exec();
  populateList();
  populateGroups();
}

void QgsStyleManagerDialog::setBold( QStandardItem* item )
{
  QFont font = item->font();
  font.setBold( true );
  item->setFont( font );
}

void QgsStyleManagerDialog::populateGroups()
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

void QgsStyleManagerDialog::buildGroupTree( QStandardItem* &parent )
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

void QgsStyleManagerDialog::groupChanged( const QModelIndex& index )
{
  QStringList symbolNames;
  QStringList groupSymbols;

  QgsStyle::StyleEntity type = currentItemType() < 3 ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
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
      actnAddGroup->setEnabled( true );
    }
    symbolNames = currentItemType() < 3 ? mStyle->symbolNames() : mStyle->colorRampNames();
  }
  else
  {
    //determine groups and tags
    if ( index.parent().data( Qt::UserRole + 1 ) == "smartgroups" )
    {
      btnAddGroup->setEnabled( false );
      actnAddGroup->setEnabled( false );
      btnRemoveGroup->setEnabled( true );
      actnRemoveGroup->setEnabled( true );
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

  actnEditSmartGroup->setVisible( false );
  actnAddGroup->setVisible( false );
  actnRemoveGroup->setVisible( false );
  actnGroupSymbols->setVisible( false );
  actnFinishGrouping->setVisible( false );

  if ( index.parent().isValid() && ( index.data().toString() != "Ungrouped" ) )
  {
    if ( index.parent().data( Qt::UserRole + 1 ).toString() == "smartgroups" )
    {
      actnEditSmartGroup->setVisible( !mGrouppingMode );
    }
    else
    {
      actnAddGroup->setVisible( !mGrouppingMode );
      actnGroupSymbols->setVisible( !mGrouppingMode );
      actnFinishGrouping->setVisible( mGrouppingMode );
    }
    actnRemoveGroup->setVisible( true );
  }
  else if ( index.data( Qt::UserRole + 1 ) == "groups" || index.data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    actnAddGroup->setVisible( !mGrouppingMode );
  }
}

void QgsStyleManagerDialog::addGroup()
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
  bool isGroup = true;

  int id;
  if ( parentData == "smartgroups" )
  {
    // create a smart group

    QgsSmartGroupEditorDialog dlg( mStyle, this );
    if ( dlg.exec() == QDialog::Rejected )
      return;
    id = mStyle->addSmartgroup( dlg.smartgroupName(), dlg.conditionOperator(), dlg.conditionMap() );
    if ( !id )
      return;
    itemName = dlg.smartgroupName();
    isGroup = false;
  }
  else
  {
    // create a simple child-group to the selected

    itemName = QString( tr( "New Group" ) );
    int parentid = ( parentData == "groups" ) ? 0 : parentData.toInt(); // parentid is 0 for top-level groups
    id = mStyle->addGroup( itemName, parentid );
    if ( !id )
    {
      QMessageBox::critical( this, tr( "Error!" ),
                             tr( "New group could not be created.\n"
                                 "There was a problem with your symbol database." ) );
      return;
    }
  }

  QStandardItem *parentItem = model->itemFromIndex( parentIndex );
  QStandardItem *childItem = new QStandardItem( itemName );
  childItem->setData( id );
  parentItem->appendRow( childItem );

  groupTree->setCurrentIndex( childItem->index() );
  if ( isGroup )
  {
    groupTree->edit( childItem->index() );
  }
}

void QgsStyleManagerDialog::removeGroup()
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
    mStyle->remove( QgsStyle::SmartgroupEntity, index.data( Qt::UserRole + 1 ).toInt() );
  }
  else
  {
    mStyle->remove( QgsStyle::GroupEntity, index.data( Qt::UserRole + 1 ).toInt() );
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

void QgsStyleManagerDialog::groupRenamed( QStandardItem * item )
{
  QgsDebugMsg( "Symbol group edited: data=" + item->data( Qt::UserRole + 1 ).toString() + " text=" + item->text() );
  int id = item->data( Qt::UserRole + 1 ).toInt();
  QString name = item->text();
  if ( item->parent()->data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    mStyle->rename( QgsStyle::SmartgroupEntity, id, name );
  }
  else
  {
    mStyle->rename( QgsStyle::GroupEntity, id, name );
  }
}

void QgsStyleManagerDialog::groupSymbolsAction()
{

  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel*>( groupTree->model() );
  QStandardItemModel *model = qobject_cast<QStandardItemModel*>( listItems->model() );

  if ( mGrouppingMode )
  {
    mGrouppingMode = false;
    actnGroupSymbols->setVisible( true );
    actnFinishGrouping->setVisible( false );
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
    // Change visibility of actions
    actnGroupSymbols->setVisible( false );
    actnFinishGrouping->setVisible( true );
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

void QgsStyleManagerDialog::regrouped( QStandardItem *item )
{
  QgsStyle::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
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

void QgsStyleManagerDialog::setSymbolsChecked( const QStringList& symbols )
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel*>( listItems->model() );
  Q_FOREACH ( const QString& symbol, symbols )
  {
    QList<QStandardItem*> items = model->findItems( symbol );
    Q_FOREACH ( QStandardItem* item, items )
      item->setCheckState( Qt::Checked );
  }
}

void QgsStyleManagerDialog::filterSymbols( const QString& qword )
{
  QStringList items;
  if ( currentItemType() == 3 )
  {
    items = mStyle->findSymbols( QgsStyle::ColorrampEntity, qword );
    populateColorRamps( items );
  }
  else
  {
    items = mStyle->findSymbols( QgsStyle::SymbolEntity, qword );
    populateSymbols( items );
  }
}

void QgsStyleManagerDialog::tagsChanged()
{
  QModelIndexList indexes =  listItems->selectionModel()->selection().indexes();
  QStringList addtags;
  QStringList removetags;

  QStringList oldtags = mTagList;
  QStringList newtags = tagsLineEdit->text().split( ',', QString::SkipEmptyParts );

  QgsStyle::StyleEntity type;
  if ( currentItemType() < 3 )
  {
    type = QgsStyle::SymbolEntity;
  }
  else if ( currentItemType() == 3 )
  {
    type = QgsStyle::ColorrampEntity;
  }
  else
  {
    QgsDebugMsg( "Unknown Style Entity!" );
    return;
  }
  // compare old with new to find removed tags
  Q_FOREACH ( const QString &tag, oldtags )
  {
    if ( !newtags.contains( tag ) )
      removetags.append( tag );
  }
  if ( !removetags.isEmpty() )
  {
    Q_FOREACH ( const QModelIndex& index, indexes )
    {
      mStyle->detagSymbol( type, index.data().toString(), removetags );
    }
  }
  // compare new with old to find added tags
  Q_FOREACH ( const QString &tag, newtags )
  {
    if ( !oldtags.contains( tag ) )
      addtags.append( tag );
  }
  if ( !addtags.isEmpty() )
  {
    Q_FOREACH ( const QModelIndex& index, indexes )
    {
      mStyle->tagSymbol( type, index.data().toString(), addtags );
    }
  }
}

void QgsStyleManagerDialog::symbolSelected( const QModelIndex& index )
{
  // Populate the tags for the symbol
  tagsLineEdit->clear();
  if ( index.isValid() )
  {
    QStandardItem *item = static_cast<QStandardItemModel*>( listItems->model() )->itemFromIndex( index );
    QgsStyle::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
    mTagList = mStyle->tagsOfSymbol( type, item->data().toString() );
    tagsLineEdit->setText( mTagList.join( "," ) );
  }

  actnEditItem->setEnabled( index.isValid() && !mGrouppingMode );
}

void QgsStyleManagerDialog::selectedSymbolsChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  bool nothingSelected = listItems->selectionModel()->selectedIndexes().empty();
  actnRemoveItem->setDisabled( nothingSelected );
  mGroupListMenu->setDisabled( nothingSelected );
  actnUngroup->setDisabled( nothingSelected );
  actnExportAsPNG->setDisabled( nothingSelected );
  actnExportAsSVG->setDisabled( nothingSelected );
  actnEditItem->setDisabled( nothingSelected );
}

void QgsStyleManagerDialog::enableSymbolInputs( bool enable )
{
  groupTree->setEnabled( enable );
  btnAddGroup->setEnabled( enable );
  actnAddGroup->setEnabled( enable );
  btnRemoveGroup->setEnabled( enable );
  actnRemoveGroup->setEnabled( enable );
  btnManageGroups->setEnabled( enable || mGrouppingMode ); // always enabled in grouping mode, as it is the only way to leave grouping mode
  searchBox->setEnabled( enable );
  tagsLineEdit->setEnabled( enable );
}

void QgsStyleManagerDialog::enableGroupInputs( bool enable )
{
  btnAddGroup->setEnabled( enable );
  btnRemoveGroup->setEnabled( enable );
  actnRemoveGroup->setEnabled( enable );
  btnManageGroups->setEnabled( enable || mGrouppingMode ); // always enabled in grouping mode, as it is the only way to leave grouping mode
}

void QgsStyleManagerDialog::enableItemsForGroupingMode( bool enable )
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
    QWidget *w = qobject_cast<QWidget*>( symbolBtnsLayout->itemAt( i )->widget() );
    if ( w )
      w->setEnabled( enable );
  }

  // The actions
  actnRemoveItem->setEnabled( enable );
  actnEditItem->setEnabled( enable );
}

void QgsStyleManagerDialog::grouptreeContextMenu( QPoint point )
{
  QPoint globalPos = groupTree->viewport()->mapToGlobal( point );

  QModelIndex index = groupTree->indexAt( point );
  QgsDebugMsg( "Now you clicked: " + index.data().toString() );

  if ( index.isValid() && !mGrouppingMode )
    mGroupTreeContextMenu->popup( globalPos );
}

void QgsStyleManagerDialog::listitemsContextMenu( QPoint point )
{
  QPoint globalPos = listItems->viewport()->mapToGlobal( point );

  // Clear all actions and create new actions for every group
  mGroupListMenu->clear();

  QAction* a;
  QList<int> groupIds = mStyle->groupIds();
  Q_FOREACH ( int groupId, groupIds )
  {
    a = new QAction( mStyle->groupName( groupId ), mGroupListMenu );
    a->setData( groupId );
    connect( a, SIGNAL( triggered( bool ) ), this, SLOT( groupSelectedSymbols() ) );
    mGroupListMenu->addAction( a );
  }

  mGroupMenu->popup( globalPos );
}

void QgsStyleManagerDialog::groupSelectedSymbols()
{
  QAction* selectedItem = qobject_cast<QAction*>( sender() );

  if ( selectedItem )
  {
    QgsStyle::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
    if ( currentItemType() > 3 )
    {
      QgsDebugMsg( "unknown entity type" );
      return;
    }
    int groupId = selectedItem->data().toInt();
    QModelIndexList indexes =  listItems->selectionModel()->selectedIndexes();
    Q_FOREACH ( const QModelIndex& index, indexes )
    {
      mStyle->group( type, index.data().toString(), groupId );
    }
    populateList();

    QgsDebugMsg( "Selected Action: " + selectedItem->text() );
  }
}

void QgsStyleManagerDialog::editSmartgroupAction()
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

  mStyle->remove( QgsStyle::SmartgroupEntity, item->data().toInt() );
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

bool QgsStyleManagerDialog::eventFilter( QObject *obj, QEvent *event )
{

  if (( obj == tagsLineEdit ) && ( event->type() == QEvent::FocusOut ) )
  {
    tagsChanged();
    return true;
  }
  return false;
}

