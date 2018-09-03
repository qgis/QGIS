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
#include "qgsstylesavedialog.h"

#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"

#include "qgssymbolselectordialog.h"
#include "qgsgradientcolorrampdialog.h"
#include "qgslimitedrandomcolorrampdialog.h"
#include "qgscolorbrewercolorrampdialog.h"
#include "qgspresetcolorrampdialog.h"
#include "qgscptcitycolorrampdialog.h"
#include "qgsstyleexportimportdialog.h"
#include "qgssmartgroupeditordialog.h"
#include "qgssettings.h"

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QMenu>

#include "qgsapplication.h"
#include "qgslogger.h"

QgsStyleManagerDialog::QgsStyleManagerDialog( QgsStyle *style, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
  , mStyle( style )
{
  setupUi( this );
  connect( tabItemType, &QTabWidget::currentChanged, this, &QgsStyleManagerDialog::tabItemType_currentChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsStyleManagerDialog::showHelp );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsStyleManagerDialog::onClose );

#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  QgsSettings settings;

  restoreGeometry( settings.value( QStringLiteral( "Windows/StyleV2Manager/geometry" ) ).toByteArray() );
  mSplitter->setSizes( QList<int>() << 170 << 540 );
  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/StyleV2Manager/splitter" ) ).toByteArray() );

  tabItemType->setDocumentMode( true );
  searchBox->setShowSearchIcon( true );
  searchBox->setPlaceholderText( tr( "Filter symbols…" ) );

  connect( this, &QDialog::finished, this, &QgsStyleManagerDialog::onFinished );

  connect( listItems, &QAbstractItemView::doubleClicked, this, &QgsStyleManagerDialog::editItem );

  connect( btnAddItem, &QPushButton::clicked, this, [ = ]( bool ) { addItem(); }
         );
  connect( btnEditItem, &QPushButton::clicked, this, [ = ]( bool ) { editItem(); }
         );
  connect( actnEditItem, &QAction::triggered, this, [ = ]( bool ) { editItem(); }
         );
  connect( btnRemoveItem, &QPushButton::clicked, this, [ = ]( bool ) { removeItem(); }
         );
  connect( actnRemoveItem, &QAction::triggered, this, [ = ]( bool ) { removeItem(); }
         );

  QMenu *shareMenu = new QMenu( tr( "Share Menu" ), this );
  QAction *exportAction = new QAction( tr( "Export Item(s)…" ), this );
  exportAction->setIcon( QIcon( QgsApplication::iconPath( "mActionFileSave.svg" ) ) );
  shareMenu->addAction( exportAction );
  QAction *importAction = new QAction( tr( "Import Item(s)…" ), this );
  importAction->setIcon( QIcon( QgsApplication::iconPath( "mActionFileOpen.svg" ) ) );
  shareMenu->addAction( importAction );
  shareMenu->addSeparator();
  shareMenu->addAction( actnExportAsPNG );
  shareMenu->addAction( actnExportAsSVG );
  connect( actnExportAsPNG, &QAction::triggered, this, &QgsStyleManagerDialog::exportItemsPNG );
  connect( actnExportAsSVG, &QAction::triggered, this, &QgsStyleManagerDialog::exportItemsSVG );
  connect( exportAction, &QAction::triggered, this, &QgsStyleManagerDialog::exportItems );
  connect( importAction, &QAction::triggered, this, &QgsStyleManagerDialog::importItems );
  btnShare->setMenu( shareMenu );

  QStandardItemModel *model = new QStandardItemModel( listItems );
  listItems->setModel( model );
  listItems->setSelectionMode( QAbstractItemView::ExtendedSelection );

  connect( model, &QStandardItemModel::itemChanged, this, &QgsStyleManagerDialog::itemChanged );
  connect( listItems->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsStyleManagerDialog::symbolSelected );
  connect( listItems->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &QgsStyleManagerDialog::selectedSymbolsChanged );

  populateTypes();

  QStandardItemModel *groupModel = new QStandardItemModel( groupTree );
  groupTree->setModel( groupModel );
  groupTree->setHeaderHidden( true );
  populateGroups();
  groupTree->setCurrentIndex( groupTree->model()->index( 0, 0 ) );

  connect( groupTree->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsStyleManagerDialog::groupChanged );
  connect( groupModel, &QStandardItemModel::itemChanged,
           this, &QgsStyleManagerDialog::groupRenamed );

  QMenu *groupMenu = new QMenu( tr( "Group Actions" ), this );
  connect( actnTagSymbols, &QAction::triggered, this, &QgsStyleManagerDialog::tagSymbolsAction );
  groupMenu->addAction( actnTagSymbols );
  connect( actnFinishTagging, &QAction::triggered, this, &QgsStyleManagerDialog::tagSymbolsAction );
  actnFinishTagging->setVisible( false );
  groupMenu->addAction( actnFinishTagging );
  groupMenu->addAction( actnEditSmartGroup );
  btnManageGroups->setMenu( groupMenu );

  connect( searchBox, &QLineEdit::textChanged, this, &QgsStyleManagerDialog::filterSymbols );

  // Context menu for groupTree
  groupTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( groupTree, &QWidget::customContextMenuRequested,
           this, &QgsStyleManagerDialog::grouptreeContextMenu );

  // Context menu for listItems
  listItems->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( listItems, &QWidget::customContextMenuRequested,
           this, &QgsStyleManagerDialog::listitemsContextMenu );

  // Menu for the "Add item" toolbutton when in colorramp mode
  QStringList rampTypes;
  rampTypes << tr( "Gradient" ) << tr( "Color presets" ) << tr( "Random" ) << tr( "Catalog: cpt-city" );
  rampTypes << tr( "Catalog: ColorBrewer" );
  mMenuBtnAddItemColorRamp = new QMenu( this );
  for ( const QString &rampType : qgis::as_const( rampTypes ) )
    mMenuBtnAddItemColorRamp->addAction( new QAction( rampType, this ) );
  connect( mMenuBtnAddItemColorRamp, &QMenu::triggered,
           this, static_cast<bool ( QgsStyleManagerDialog::* )( QAction * )>( &QgsStyleManagerDialog::addColorRamp ) );

  // Context menu for symbols/colorramps. The menu entries for every group are created when displaying the menu.
  mGroupMenu = new QMenu( this );
  connect( actnAddFavorite, &QAction::triggered, this, &QgsStyleManagerDialog::addFavoriteSelectedSymbols );
  mGroupMenu->addAction( actnAddFavorite );
  connect( actnRemoveFavorite, &QAction::triggered, this, &QgsStyleManagerDialog::removeFavoriteSelectedSymbols );
  mGroupMenu->addAction( actnRemoveFavorite );
  mGroupMenu->addSeparator()->setParent( this );
  mGroupListMenu = new QMenu( mGroupMenu );
  mGroupListMenu->setTitle( tr( "Add to Tag" ) );
  mGroupListMenu->setEnabled( false );
  mGroupMenu->addMenu( mGroupListMenu );
  actnDetag->setData( 0 );
  connect( actnDetag, &QAction::triggered, this, &QgsStyleManagerDialog::detagSelectedSymbols );
  mGroupMenu->addAction( actnDetag );
  mGroupMenu->addSeparator()->setParent( this );
  mGroupMenu->addAction( actnRemoveItem );
  mGroupMenu->addAction( actnEditItem );
  mGroupMenu->addSeparator()->setParent( this );
  mGroupMenu->addAction( actnExportAsPNG );
  mGroupMenu->addAction( actnExportAsSVG );

  // Context menu for the group tree
  mGroupTreeContextMenu = new QMenu( this );
  connect( actnEditSmartGroup, &QAction::triggered, this, &QgsStyleManagerDialog::editSmartgroupAction );
  mGroupTreeContextMenu->addAction( actnEditSmartGroup );
  connect( actnAddTag, &QAction::triggered, this, [ = ]( bool ) { addTag(); }
         );
  mGroupTreeContextMenu->addAction( actnAddTag );
  connect( actnAddSmartgroup, &QAction::triggered, this, [ = ]( bool ) { addSmartgroup(); }
         );
  mGroupTreeContextMenu->addAction( actnAddSmartgroup );
  connect( actnRemoveGroup, &QAction::triggered, this, &QgsStyleManagerDialog::removeGroup );
  mGroupTreeContextMenu->addAction( actnRemoveGroup );

  tabItemType_currentChanged( 0 );

  connect( mStyle, &QgsStyle::symbolSaved, this, &QgsStyleManagerDialog::populateList );
  connect( mStyle, &QgsStyle::groupsModified, this, &QgsStyleManagerDialog::populateGroups );
}

void QgsStyleManagerDialog::onFinished()
{
  if ( mModified )
  {
    mStyle->save();
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/StyleV2Manager/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/StyleV2Manager/splitter" ), mSplitter->saveState() );
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

void QgsStyleManagerDialog::tabItemType_currentChanged( int )
{
  // when in Color Ramp tab, add menu to add item button and hide "Export symbols as PNG/SVG"
  bool flag = currentItemType() != 3;
  searchBox->setPlaceholderText( flag ? tr( "Filter symbols…" ) : tr( "Filter color ramps…" ) );
  btnAddItem->setMenu( flag ? nullptr : mMenuBtnAddItemColorRamp );
  actnExportAsPNG->setVisible( flag );
  actnExportAsSVG->setVisible( flag );

  listItems->setIconSize( QSize( 100, 90 ) );
  listItems->setGridSize( QSize( 120, 110 ) );

  populateList();
}

void QgsStyleManagerDialog::populateList()
{
  if ( currentItemType() > 3 )
  {
    Q_ASSERT( false && "not implemented" );
    return;
  }
  groupChanged( groupTree->selectionModel()->currentIndex() );
}

void QgsStyleManagerDialog::populateSymbols( const QStringList &symbolNames, bool check )
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( listItems->model() );
  model->clear();

  int type = currentItemType();
  for ( int i = 0; i < symbolNames.count(); ++i )
  {
    QString name = symbolNames[i];
    std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( name ) );
    if ( symbol && symbol->type() == type )
    {
      QStringList tags = mStyle->tagsOfSymbol( QgsStyle::SymbolEntity, name );
      QStandardItem *item = new QStandardItem( name );
      QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol.get(), listItems->iconSize(), 18 );
      item->setIcon( icon );
      item->setData( name ); // used to find out original name when user edited the name
      item->setCheckable( check );
      item->setToolTip( QStringLiteral( "<b>%1</b><br><i>%2</i>" ).arg( name, tags.count() > 0 ? tags.join( QStringLiteral( ", " ) ) : tr( "Not tagged" ) ) );
      // add to model
      model->appendRow( item );
    }
  }
  selectedSymbolsChanged( QItemSelection(), QItemSelection() );
  symbolSelected( listItems->currentIndex() );
}


void QgsStyleManagerDialog::populateColorRamps( const QStringList &colorRamps, bool check )
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( listItems->model() );
  model->clear();

  for ( int i = 0; i < colorRamps.count(); ++i )
  {
    QString name = colorRamps[i];
    std::unique_ptr< QgsColorRamp > ramp( mStyle->colorRamp( name ) );

    QStandardItem *item = new QStandardItem( name );
    QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp.get(), listItems->iconSize(), 18 );
    item->setIcon( icon );
    item->setData( name ); // used to find out original name when user edited the name
    item->setCheckable( check );
    item->setToolTip( name );
    model->appendRow( item );
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
    Q_ASSERT( false && "not implemented" );
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
  QgsSymbol *symbol = nullptr;
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
      Q_ASSERT( false && "unknown symbol type" );
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

  QgsStyleSaveDialog saveDlg( this );
  if ( !saveDlg.exec() )
  {
    delete symbol;
    return false;
  }

  name = saveDlg.name();

  // request valid/unique name
  bool nameInvalid = true;
  while ( nameInvalid )
  {
    // validate name
    if ( name.isEmpty() )
    {
      QMessageBox::warning( this, tr( "Save Symbol" ),
                            tr( "Cannot save symbol without name. Enter a name." ) );
    }
    else if ( mStyle->symbolNames().contains( name ) )
    {
      int res = QMessageBox::warning( this, tr( "Save Symbol" ),
                                      tr( "Symbol with name '%1' already exists. Overwrite?" )
                                      .arg( name ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res == QMessageBox::Yes )
      {
        mStyle->removeSymbol( name );
        nameInvalid = false;
      }
    }
    else
    {
      // valid name
      nameInvalid = false;
    }
    if ( nameInvalid )
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
    }
  }

  QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new symbol to style and re-populate the list
  mStyle->addSymbol( name, symbol );
  mStyle->saveSymbol( name, symbol, saveDlg.isFavorite(), symbolTags );

  mModified = true;
  return true;
}


QString QgsStyleManagerDialog::addColorRampStatic( QWidget *parent, QgsStyle *style, QString rampType )
{
  // let the user choose the color ramp type if rampType is not given
  bool ok = true;
  if ( rampType.isEmpty() )
  {
    QStringList rampTypes;
    rampTypes << tr( "Gradient" ) << tr( "Color presets" ) << tr( "Random" ) << tr( "Catalog: cpt-city" );
    rampTypes << tr( "Catalog: ColorBrewer" );
    rampType = QInputDialog::getItem( parent, tr( "Color Ramp Type" ),
                                      tr( "Please select color ramp type:" ), rampTypes, 0, false, &ok );
  }
  if ( !ok || rampType.isEmpty() )
    return QString();

  QString name = tr( "new ramp" );

  std::unique_ptr< QgsColorRamp  > ramp;
  if ( rampType == tr( "Gradient" ) )
  {
    QgsGradientColorRampDialog dlg( QgsGradientColorRamp(), parent );
    if ( !dlg.exec() )
    {
      return QString();
    }
    ramp.reset( dlg.ramp().clone() );
    name = tr( "new gradient ramp" );
  }
  else if ( rampType == tr( "Random" ) )
  {
    QgsLimitedRandomColorRampDialog dlg( QgsLimitedRandomColorRamp(), parent );
    if ( !dlg.exec() )
    {
      return QString();
    }
    ramp.reset( dlg.ramp().clone() );
    name = tr( "new random ramp" );
  }
  else if ( rampType == tr( "Catalog: ColorBrewer" ) )
  {
    QgsColorBrewerColorRampDialog dlg( QgsColorBrewerColorRamp(), parent );
    if ( !dlg.exec() )
    {
      return QString();
    }
    ramp.reset( dlg.ramp().clone() );
    name = dlg.ramp().schemeName() + QString::number( dlg.ramp().colors() );
  }
  else if ( rampType == tr( "Color presets" ) )
  {
    QgsPresetColorRampDialog dlg( QgsPresetSchemeColorRamp(), parent );
    if ( !dlg.exec() )
    {
      return QString();
    }
    ramp.reset( dlg.ramp().clone() );
    name = tr( "new preset ramp" );
  }
  else if ( rampType == tr( "Catalog: cpt-city" ) )
  {
    QgsCptCityColorRampDialog dlg( QgsCptCityColorRamp( QString(), QString() ), parent );
    if ( !dlg.exec() )
    {
      return QString();
    }
    // name = dlg.selectedName();
    name = QFileInfo( dlg.ramp().schemeName() ).baseName() + dlg.ramp().variantName();
    if ( dlg.saveAsGradientRamp() )
    {
      ramp.reset( dlg.ramp().cloneGradientRamp() );
    }
    else
    {
      ramp.reset( dlg.ramp().clone() );
    }
  }
  else
  {
    // Q_ASSERT( 0 && "invalid ramp type" );
    // bailing out is rather harsh!
    QgsDebugMsg( QStringLiteral( "invalid ramp type %1" ).arg( rampType ) );
    return QString();
  }

  QgsStyleSaveDialog saveDlg( parent, QgsStyle::ColorrampEntity );
  if ( !saveDlg.exec() )
  {
    return QString();
  }

  name = saveDlg.name();

  // get valid/unique name
  bool nameInvalid = true;
  while ( nameInvalid )
  {
    // validate name
    if ( name.isEmpty() )
    {
      QMessageBox::warning( parent, tr( "Save Color Ramp" ),
                            tr( "Cannot save color ramp without name. Enter a name." ) );
    }
    else if ( style->colorRampNames().contains( name ) )
    {
      int res = QMessageBox::warning( parent, tr( "Save Color Ramp" ),
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
    if ( nameInvalid )
    {
      bool ok;
      name = QInputDialog::getText( parent, tr( "Color Ramp Name" ),
                                    tr( "Please enter a name for new color ramp:" ),
                                    QLineEdit::Normal, name, &ok );
      if ( !ok )
      {
        return QString();
      }
    }
  }

  QStringList colorRampTags = saveDlg.tags().split( ',' );
  QgsColorRamp *r = ramp.release();

  // add new symbol to style and re-populate the list
  style->addColorRamp( name, r );
  style->saveColorRamp( name, r, saveDlg.isFavorite(), colorRampTags );

  return name;
}

void QgsStyleManagerDialog::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

bool QgsStyleManagerDialog::addColorRamp()
{
  return addColorRamp( nullptr );
}

bool QgsStyleManagerDialog::addColorRamp( QAction *action )
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
    Q_ASSERT( false && "not implemented" );
  }

  if ( changed )
    populateList();
}

bool QgsStyleManagerDialog::editSymbol()
{
  QString symbolName = currentItemName();
  if ( symbolName.isEmpty() )
    return false;

  std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( symbolName ) );

  // let the user edit the symbol and update list when done
  QgsSymbolSelectorDialog dlg( symbol.get(), mStyle, nullptr, this );
  if ( dlg.exec() == 0 )
  {
    return false;
  }

  // by adding symbol to style with the same name the old effectively gets overwritten
  mStyle->addSymbol( symbolName, symbol.release(), true );
  mModified = true;
  return true;
}

bool QgsStyleManagerDialog::editColorRamp()
{
  QString name = currentItemName();
  if ( name.isEmpty() )
    return false;

  std::unique_ptr< QgsColorRamp > ramp( mStyle->colorRamp( name ) );

  if ( ramp->type() == QLatin1String( "gradient" ) )
  {
    QgsGradientColorRamp *gradRamp = static_cast<QgsGradientColorRamp *>( ramp.get() );
    QgsGradientColorRampDialog dlg( *gradRamp, this );
    if ( !dlg.exec() )
    {
      return false;
    }
    ramp.reset( dlg.ramp().clone() );
  }
  else if ( ramp->type() == QLatin1String( "random" ) )
  {
    QgsLimitedRandomColorRamp *randRamp = static_cast<QgsLimitedRandomColorRamp *>( ramp.get() );
    QgsLimitedRandomColorRampDialog dlg( *randRamp, this );
    if ( !dlg.exec() )
    {
      return false;
    }
    ramp.reset( dlg.ramp().clone() );
  }
  else if ( ramp->type() == QLatin1String( "colorbrewer" ) )
  {
    QgsColorBrewerColorRamp *brewerRamp = static_cast<QgsColorBrewerColorRamp *>( ramp.get() );
    QgsColorBrewerColorRampDialog dlg( *brewerRamp, this );
    if ( !dlg.exec() )
    {
      return false;
    }
    ramp.reset( dlg.ramp().clone() );
  }
  else if ( ramp->type() == QLatin1String( "preset" ) )
  {
    QgsPresetSchemeColorRamp *presetRamp = static_cast<QgsPresetSchemeColorRamp *>( ramp.get() );
    QgsPresetColorRampDialog dlg( *presetRamp, this );
    if ( !dlg.exec() )
    {
      return false;
    }
    ramp.reset( dlg.ramp().clone() );
  }
  else if ( ramp->type() == QLatin1String( "cpt-city" ) )
  {
    QgsCptCityColorRamp *cptCityRamp = static_cast<QgsCptCityColorRamp *>( ramp.get() );
    QgsCptCityColorRampDialog dlg( *cptCityRamp, this );
    if ( !dlg.exec() )
    {
      return false;
    }
    if ( dlg.saveAsGradientRamp() )
    {
      ramp.reset( dlg.ramp().cloneGradientRamp() );
    }
    else
    {
      ramp.reset( dlg.ramp().clone() );
    }
  }
  else
  {
    Q_ASSERT( false && "invalid ramp type" );
  }

  mStyle->addColorRamp( name, ramp.release(), true );
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
    Q_ASSERT( false && "not implemented" );
  }

  if ( changed )
  {
    populateList();
    populateTypes();
  }
}

bool QgsStyleManagerDialog::removeSymbol()
{
  const QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Symbol" ),
       QString( tr( "Do you really want to remove %n symbol(s)?", nullptr, indexes.count() ) ),
       QMessageBox::Yes,
       QMessageBox::No ) )
    return false;

  QgsTemporaryCursorOverride override( Qt::WaitCursor );

  for ( const QModelIndex &index : indexes )
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
  const QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Color Ramp" ),
       QString( tr( "Do you really want to remove %n ramp(s)?", nullptr, indexes.count() ) ),
       QMessageBox::Yes,
       QMessageBox::No ) )
    return false;

  QgsTemporaryCursorOverride override( Qt::WaitCursor );

  for ( const QModelIndex &index : indexes )
  {
    QString rampName = index.data().toString();
    // delete from style and update list
    if ( !rampName.isEmpty() )
      mStyle->removeColorRamp( rampName );
  }
  mModified = true;
  return true;
}

void QgsStyleManagerDialog::itemChanged( QStandardItem *item )
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
    QMessageBox::critical( this, tr( "Save Item" ),
                           tr( "Name is already taken by another item. Choose a different name." ) );
    item->setText( oldName );
  }
}

void QgsStyleManagerDialog::exportItemsPNG()
{
  QString dir = QFileDialog::getExistingDirectory( this, tr( "Export Selected Symbols as PNG" ),
                QDir::home().absolutePath(),
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks );
  exportSelectedItemsImages( dir, QStringLiteral( "png" ), QSize( 32, 32 ) );
}

void QgsStyleManagerDialog::exportItemsSVG()
{
  QString dir = QFileDialog::getExistingDirectory( this, tr( "Export Selected Symbols as SVG" ),
                QDir::home().absolutePath(),
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks );
  exportSelectedItemsImages( dir, QStringLiteral( "svg" ), QSize( 32, 32 ) );
}


void QgsStyleManagerDialog::exportSelectedItemsImages( const QString &dir, const QString &format, QSize size )
{
  if ( dir.isEmpty() )
    return;

  const QModelIndexList indexes = listItems->selectionModel()->selection().indexes();
  for ( const QModelIndex &index : indexes )
  {
    QString name = index.data().toString();
    QString path = dir + '/' + name + '.' + format;
    std::unique_ptr< QgsSymbol > sym( mStyle->symbol( name ) );
    if ( sym )
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

void QgsStyleManagerDialog::setBold( QStandardItem *item )
{
  QFont font = item->font();
  font.setBold( true );
  item->setFont( font );
}

void QgsStyleManagerDialog::populateGroups()
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( groupTree->model() );
  model->clear();

  QStandardItem *favoriteSymbols = new QStandardItem( tr( "Favorites" ) );
  favoriteSymbols->setData( "favorite" );
  favoriteSymbols->setEditable( false );
  setBold( favoriteSymbols );
  model->appendRow( favoriteSymbols );

  QStandardItem *allSymbols = new QStandardItem( tr( "All" ) );
  allSymbols->setData( "all" );
  allSymbols->setEditable( false );
  setBold( allSymbols );
  model->appendRow( allSymbols );

  QStandardItem *taggroup = new QStandardItem( QString() ); //require empty name to get first order groups
  taggroup->setData( "tags" );
  taggroup->setEditable( false );
  QStringList tags = mStyle->tags();
  tags.sort();
  for ( const QString &tag : qgis::as_const( tags ) )
  {
    QStandardItem *item = new QStandardItem( tag );
    item->setData( mStyle->tagId( tag ) );
    taggroup->appendRow( item );
  }
  taggroup->setText( tr( "Tags" ) );//set title later
  setBold( taggroup );
  model->appendRow( taggroup );

  QStandardItem *smart = new QStandardItem( tr( "Smart Groups" ) );
  smart->setData( "smartgroups" );
  smart->setEditable( false );
  setBold( smart );
  QgsSymbolGroupMap sgMap = mStyle->smartgroupsListMap();
  QgsSymbolGroupMap::const_iterator i = sgMap.constBegin();
  while ( i != sgMap.constEnd() )
  {
    QStandardItem *item = new QStandardItem( i.value() );
    item->setData( i.key() );
    smart->appendRow( item );
    ++i;
  }
  model->appendRow( smart );

  // expand things in the group tree
  int rows = model->rowCount( model->indexFromItem( model->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    groupTree->setExpanded( model->indexFromItem( model->item( i ) ), true );
  }
}

void QgsStyleManagerDialog::groupChanged( const QModelIndex &index )
{
  QStringList symbolNames;
  QStringList groupSymbols;

  QgsStyle::StyleEntity type = currentItemType() < 3 ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
  if ( currentItemType() > 3 )
  {
    QgsDebugMsg( QStringLiteral( "Entity not implemented" ) );
    return;
  }

  QString category = index.data( Qt::UserRole + 1 ).toString();
  if ( category == QLatin1String( "all" ) || category == QLatin1String( "tags" ) || category == QLatin1String( "smartgroups" ) )
  {
    enableGroupInputs( false );
    if ( category == QLatin1String( "tags" ) )
    {
      actnAddTag->setEnabled( true );
      actnAddSmartgroup->setEnabled( false );
    }
    else if ( category == QLatin1String( "smartgroups" ) )
    {
      actnAddTag->setEnabled( false );
      actnAddSmartgroup->setEnabled( true );
    }
    symbolNames = currentItemType() < 3 ? mStyle->symbolNames() : mStyle->colorRampNames();
  }
  else if ( category == QLatin1String( "favorite" ) )
  {
    enableGroupInputs( false );
    symbolNames = mStyle->symbolsOfFavorite( type );
  }
  else if ( index.parent().data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    actnRemoveGroup->setEnabled( true );
    btnManageGroups->setEnabled( true );
    int groupId = index.data( Qt::UserRole + 1 ).toInt();
    symbolNames = mStyle->symbolsOfSmartgroup( type, groupId );
  }
  else // tags
  {
    enableGroupInputs( true );
    int tagId = index.data( Qt::UserRole + 1 ).toInt();
    symbolNames = mStyle->symbolsWithTag( type, tagId );
    if ( mGrouppingMode && tagId )
    {
      groupSymbols = symbolNames;
      symbolNames = type == QgsStyle::SymbolEntity ? mStyle->symbolNames() : mStyle->colorRampNames();
    }
  }

  symbolNames.sort();
  if ( currentItemType() < 3 )
  {
    populateSymbols( symbolNames, mGrouppingMode );
  }
  else if ( currentItemType() == 3 )
  {
    populateColorRamps( symbolNames, mGrouppingMode );
  }

  if ( mGrouppingMode )
  {
    setSymbolsChecked( groupSymbols );
  }

  actnEditSmartGroup->setVisible( false );
  actnAddTag->setVisible( false );
  actnAddSmartgroup->setVisible( false );
  actnRemoveGroup->setVisible( false );
  actnTagSymbols->setVisible( false );
  actnFinishTagging->setVisible( false );

  if ( index.parent().isValid() )
  {
    if ( index.parent().data( Qt::UserRole + 1 ).toString() == QLatin1String( "smartgroups" ) )
    {
      actnEditSmartGroup->setVisible( !mGrouppingMode );
    }
    else if ( index.parent().data( Qt::UserRole + 1 ).toString() == QLatin1String( "tags" ) )
    {
      actnAddTag->setVisible( !mGrouppingMode );
      actnTagSymbols->setVisible( !mGrouppingMode );
      actnFinishTagging->setVisible( mGrouppingMode );
    }
    actnRemoveGroup->setVisible( true );
  }
  else if ( index.data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    actnAddSmartgroup->setVisible( !mGrouppingMode );
  }
  else if ( index.data( Qt::UserRole + 1 ) == "tags" )
  {
    actnAddTag->setVisible( !mGrouppingMode );
  }
}

int QgsStyleManagerDialog::addTag()
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( groupTree->model() );
  QModelIndex index;
  for ( int i = 0; i < groupTree->model()->rowCount(); i++ )
  {
    index = groupTree->model()->index( i, 0 );
    QString data = index.data( Qt::UserRole + 1 ).toString();
    if ( data == QLatin1String( "tags" ) )
    {
      break;
    }
  }

  QString itemName;
  int id;
  bool ok;
  itemName = QInputDialog::getText( this, tr( "Add Tag" ),
                                    tr( "Please enter name for the new tag:" ), QLineEdit::Normal, tr( "New tag" ), &ok ).trimmed();
  if ( !ok || itemName.isEmpty() )
    return 0;

  int check = mStyle->tagId( itemName );
  if ( check > 0 )
  {
    QMessageBox::critical( this, tr( "Add Tag" ),
                           tr( "Tag name already exists in your symbol database." ) );
    return 0;
  }
  id = mStyle->addTag( itemName );
  if ( !id )
  {
    QMessageBox::critical( this, tr( "Add Tag" ),
                           tr( "New tag could not be created.\n"
                               "There was a problem with your symbol database." ) );
    return 0;
  }

  QStandardItem *parentItem = model->itemFromIndex( index );
  QStandardItem *childItem = new QStandardItem( itemName );
  childItem->setData( id );
  parentItem->appendRow( childItem );

  return id;
}

int QgsStyleManagerDialog::addSmartgroup()
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( groupTree->model() );
  QModelIndex index;
  for ( int i = 0; i < groupTree->model()->rowCount(); i++ )
  {
    index = groupTree->model()->index( i, 0 );
    QString data = index.data( Qt::UserRole + 1 ).toString();
    if ( data == QLatin1String( "smartgroups" ) )
    {
      break;
    }
  }

  QString itemName;
  int id;
  QgsSmartGroupEditorDialog dlg( mStyle, this );
  if ( dlg.exec() == QDialog::Rejected )
    return 0;
  id = mStyle->addSmartgroup( dlg.smartgroupName(), dlg.conditionOperator(), dlg.conditionMap() );
  if ( !id )
    return 0;
  itemName = dlg.smartgroupName();

  QStandardItem *parentItem = model->itemFromIndex( index );
  QStandardItem *childItem = new QStandardItem( itemName );
  childItem->setData( id );
  parentItem->appendRow( childItem );

  return id;
}

void QgsStyleManagerDialog::removeGroup()
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( groupTree->model() );
  QModelIndex index = groupTree->currentIndex();

  // do not allow removal of system-defined groupings
  QString data = index.data( Qt::UserRole + 1 ).toString();
  if ( data == QLatin1String( "all" ) || data == QLatin1String( "favorite" ) || data == QLatin1String( "tags" ) || index.data() == "smartgroups" )
  {
    int err = QMessageBox::critical( this, tr( "Remove Group" ),
                                     tr( "Invalid selection. Cannot delete system defined categories.\n"
                                         "Kindly select a group or smart group you might want to delete." ) );
    if ( err )
      return;
  }

  QStandardItem *parentItem = model->itemFromIndex( index.parent() );
  if ( parentItem->data( Qt::UserRole + 1 ).toString() == QLatin1String( "smartgroups" ) )
  {
    mStyle->remove( QgsStyle::SmartgroupEntity, index.data( Qt::UserRole + 1 ).toInt() );
  }
  else
  {
    mStyle->remove( QgsStyle::TagEntity, index.data( Qt::UserRole + 1 ).toInt() );
  }
  parentItem->removeRow( index.row() );
}

void QgsStyleManagerDialog::groupRenamed( QStandardItem *item )
{
  QgsDebugMsg( QStringLiteral( "Symbol group edited: data=%1 text=%2" ).arg( item->data( Qt::UserRole + 1 ).toString(), item->text() ) );
  int id = item->data( Qt::UserRole + 1 ).toInt();
  QString name = item->text();
  if ( item->parent()->data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    mStyle->rename( QgsStyle::SmartgroupEntity, id, name );
  }
  else
  {
    mStyle->rename( QgsStyle::TagEntity, id, name );
  }
}

void QgsStyleManagerDialog::tagSymbolsAction()
{

  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel *>( groupTree->model() );
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( listItems->model() );

  if ( mGrouppingMode )
  {
    mGrouppingMode = false;
    actnTagSymbols->setVisible( true );
    actnFinishTagging->setVisible( false );
    // disconnect slot which handles regrouping
    disconnect( model, &QStandardItemModel::itemChanged,
                this, &QgsStyleManagerDialog::regrouped );

    // disabel all items except groups in groupTree
    enableItemsForGroupingMode( true );
    groupChanged( groupTree->currentIndex() );

    // Finally: Reconnect all Symbol editing functionalities
    connect( treeModel, &QStandardItemModel::itemChanged,
             this, &QgsStyleManagerDialog::groupRenamed );
    connect( model, &QStandardItemModel::itemChanged,
             this, &QgsStyleManagerDialog::itemChanged );
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
      if ( present.parent().data() == "Tags" )
      {
        validGroup = true;
        break;
      }
      present = present.parent();
    }
    if ( !validGroup )
      return;

    mGrouppingMode = true;
    // Change visibility of actions
    actnTagSymbols->setVisible( false );
    actnFinishTagging->setVisible( true );
    // Remove all Symbol editing functionalities
    disconnect( treeModel, &QStandardItemModel::itemChanged,
                this, &QgsStyleManagerDialog::groupRenamed );
    disconnect( model, &QStandardItemModel::itemChanged,
                this, &QgsStyleManagerDialog::itemChanged );

    // disabel all items except groups in groupTree
    enableItemsForGroupingMode( false );
    groupChanged( groupTree->currentIndex() );
    btnManageGroups->setEnabled( true );


    // Connect to slot which handles regrouping
    connect( model, &QStandardItemModel::itemChanged,
             this, &QgsStyleManagerDialog::regrouped );

    // No selection should be possible
    listItems->setSelectionMode( QAbstractItemView::NoSelection );
  }
}

void QgsStyleManagerDialog::regrouped( QStandardItem *item )
{
  QgsStyle::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
  if ( currentItemType() > 3 )
  {
    QgsDebugMsg( QStringLiteral( "Unknown style entity" ) );
    return;
  }

  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel *>( groupTree->model() );
  QString tag = treeModel->itemFromIndex( groupTree->currentIndex() )->text();

  QString symbolName = item->text();
  bool regrouped;
  if ( item->checkState() == Qt::Checked )
    regrouped = mStyle->tagSymbol( type, symbolName, QStringList( tag ) );
  else
    regrouped = mStyle->detagSymbol( type, symbolName, QStringList( tag ) );
  if ( !regrouped )
  {
    int er = QMessageBox::critical( this, tr( "Group Items" ),
                                    tr( "There was a problem with the symbols database while regrouping." ) );
    // call the slot again to get back to normal
    if ( er )
      tagSymbolsAction();
  }
}

void QgsStyleManagerDialog::setSymbolsChecked( const QStringList &symbols )
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( listItems->model() );
  for ( const QString &symbol : symbols )
  {
    const QList<QStandardItem *> items = model->findItems( symbol );
    for ( QStandardItem *item : items )
      item->setCheckState( Qt::Checked );
  }
}

void QgsStyleManagerDialog::filterSymbols( const QString &qword )
{
  QStringList items;
  items = mStyle->findSymbols( currentItemType() < 3 ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity, qword );
  items.sort();
  if ( currentItemType() == 3 )
  {
    populateColorRamps( items );
  }
  else
  {
    populateSymbols( items );
  }
}

void QgsStyleManagerDialog::symbolSelected( const QModelIndex &index )
{
  actnEditItem->setEnabled( index.isValid() && !mGrouppingMode );
}

void QgsStyleManagerDialog::selectedSymbolsChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  bool nothingSelected = listItems->selectionModel()->selectedIndexes().empty();
  actnRemoveItem->setDisabled( nothingSelected );
  actnAddFavorite->setDisabled( nothingSelected );
  actnRemoveFavorite->setDisabled( nothingSelected );
  mGroupListMenu->setDisabled( nothingSelected );
  actnDetag->setDisabled( nothingSelected );
  actnExportAsPNG->setDisabled( nothingSelected );
  actnExportAsSVG->setDisabled( nothingSelected );
  actnEditItem->setDisabled( nothingSelected );
}

void QgsStyleManagerDialog::enableSymbolInputs( bool enable )
{
  groupTree->setEnabled( enable );
  btnAddTag->setEnabled( enable );
  btnAddSmartgroup->setEnabled( enable );
  actnAddTag->setEnabled( enable );
  actnAddSmartgroup->setEnabled( enable );
  actnRemoveGroup->setEnabled( enable );
  btnManageGroups->setEnabled( enable || mGrouppingMode ); // always enabled in grouping mode, as it is the only way to leave grouping mode
  searchBox->setEnabled( enable );
}

void QgsStyleManagerDialog::enableGroupInputs( bool enable )
{
  actnRemoveGroup->setEnabled( enable );
  btnManageGroups->setEnabled( enable || mGrouppingMode ); // always enabled in grouping mode, as it is the only way to leave grouping mode
}

void QgsStyleManagerDialog::enableItemsForGroupingMode( bool enable )
{
  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel *>( groupTree->model() );
  for ( int i = 0; i < treeModel->rowCount(); i++ )
  {
    treeModel->item( i )->setEnabled( enable );

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
    QWidget *w = qobject_cast<QWidget *>( symbolBtnsLayout->itemAt( i )->widget() );
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
  QgsDebugMsg( QStringLiteral( "Now you clicked: %1" ).arg( index.data().toString() ) );

  if ( index.isValid() && !mGrouppingMode )
    mGroupTreeContextMenu->popup( globalPos );
}

void QgsStyleManagerDialog::listitemsContextMenu( QPoint point )
{
  QPoint globalPos = listItems->viewport()->mapToGlobal( point );

  // Clear all actions and create new actions for every group
  mGroupListMenu->clear();

  QAction *a = nullptr;
  QStringList tags = mStyle->tags();
  tags.sort();
  for ( const QString &tag : qgis::as_const( tags ) )
  {
    a = new QAction( tag, mGroupListMenu );
    a->setData( tag );
    connect( a, &QAction::triggered, this, [ = ]( bool ) { tagSelectedSymbols(); }
           );
    mGroupListMenu->addAction( a );
  }

  if ( tags.count() > 0 )
  {
    mGroupListMenu->addSeparator();
  }
  a = new QAction( tr( "Create New Tag…" ), mGroupListMenu );
  connect( a, &QAction::triggered, this, [ = ]( bool ) { tagSelectedSymbols( true ); }
         );
  mGroupListMenu->addAction( a );

  mGroupMenu->popup( globalPos );
}

void QgsStyleManagerDialog::addFavoriteSelectedSymbols()
{
  QgsStyle::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
  if ( currentItemType() > 3 )
  {
    QgsDebugMsg( QStringLiteral( "unknown entity type" ) );
    return;
  }

  const QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : indexes )
  {
    mStyle->addFavorite( type, index.data().toString() );
  }
  populateList();
}

void QgsStyleManagerDialog::removeFavoriteSelectedSymbols()
{
  QgsStyle::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
  if ( currentItemType() > 3 )
  {
    QgsDebugMsg( QStringLiteral( "unknown entity type" ) );
    return;
  }

  const QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : indexes )
  {
    mStyle->removeFavorite( type, index.data().toString() );
  }
  populateList();
}

void QgsStyleManagerDialog::tagSelectedSymbols( bool newTag )
{
  QAction *selectedItem = qobject_cast<QAction *>( sender() );
  if ( selectedItem )
  {
    QgsStyle::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
    if ( currentItemType() > 3 )
    {
      QgsDebugMsg( QStringLiteral( "unknown entity type" ) );
      return;
    }

    QString tag;
    if ( newTag )
    {
      int id = addTag();
      if ( id == 0 )
      {
        return;
      }

      tag = mStyle->tag( id );
    }
    else
    {
      tag = selectedItem->data().toString();
    }

    const QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
    for ( const QModelIndex &index : indexes )
    {
      mStyle->tagSymbol( type, index.data().toString(), QStringList( tag ) );
    }
    populateList();

    QgsDebugMsg( QStringLiteral( "Selected Action: %1" ).arg( selectedItem->text() ) );
  }
}

void QgsStyleManagerDialog::detagSelectedSymbols()
{
  QAction *selectedItem = qobject_cast<QAction *>( sender() );

  if ( selectedItem )
  {
    QgsStyle::StyleEntity type = ( currentItemType() < 3 ) ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity;
    if ( currentItemType() > 3 )
    {
      QgsDebugMsg( QStringLiteral( "unknown entity type" ) );
      return;
    }
    const QModelIndexList indexes = listItems->selectionModel()->selectedIndexes();
    for ( const QModelIndex &index : indexes )
    {
      mStyle->detagSymbol( type, index.data().toString() );
    }
    populateList();

    QgsDebugMsg( QStringLiteral( "Selected Action: %1" ).arg( selectedItem->text() ) );
  }
}

void QgsStyleManagerDialog::editSmartgroupAction()
{
  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel *>( groupTree->model() );

  // determine whether it is a valid group
  QModelIndex present = groupTree->currentIndex();
  if ( present.parent().data( Qt::UserRole + 1 ) != "smartgroups" )
  {
    QMessageBox::critical( this, tr( "Edit Smart Group" ),
                           tr( "You have not selected a Smart Group. Kindly select a Smart Group to edit." ) );
    return;
  }
  QStandardItem *item = treeModel->itemFromIndex( present );

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
    QMessageBox::critical( this, tr( "Edit Smart Group" ),
                           tr( "There was some error while editing the smart group." ) );
    return;
  }
  item->setText( dlg.smartgroupName() );
  item->setData( id );

  groupChanged( present );
}

void QgsStyleManagerDialog::onClose()
{
  reject();
}

void QgsStyleManagerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/style_library.html#the-style-manager" ) );
}
