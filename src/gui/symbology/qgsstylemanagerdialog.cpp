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
#include "qgsstylemodel.h"

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

//
// QgsCheckableStyleModel
//

///@cond PRIVATE
QgsCheckableStyleModel::QgsCheckableStyleModel( QgsStyle *style, QObject *parent, bool readOnly )
  : QgsStyleProxyModel( style, parent )
  , mStyle( style )
  , mReadOnly( readOnly )
{

}

void QgsCheckableStyleModel::setCheckable( bool checkable )
{
  if ( checkable == mCheckable )
    return;

  mCheckable = checkable;
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ), QVector< int >() << Qt::CheckStateRole );
}

void QgsCheckableStyleModel::setCheckTag( const QString &tag )
{
  if ( tag == mCheckTag )
    return;

  mCheckTag = tag;
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ), QVector< int >() << Qt::CheckStateRole );
}

Qt::ItemFlags QgsCheckableStyleModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags f = QgsStyleProxyModel::flags( index );
  if ( !mReadOnly && mCheckable && index.column() == 0 )
    f |= Qt::ItemIsUserCheckable;

  if ( mReadOnly )
    f &= ~Qt::ItemIsEditable;

  return f;
}

QVariant QgsCheckableStyleModel::data( const QModelIndex &index, int role ) const
{
  switch ( role )
  {
    case Qt::FontRole:
    {
      // drop font size to get reasonable amount of item name shown
      QFont f = QgsStyleProxyModel::data( index, role ).value< QFont >();
      f.setPointSize( 9 );
      return f;
    }

    case Qt::CheckStateRole:
    {
      if ( !mCheckable || index.column() != 0 )
        return QVariant();

      const QStringList tags = data( index, QgsStyleModel::TagRole ).toStringList();
      return tags.contains( mCheckTag ) ? Qt::Checked : Qt::Unchecked;
    }

    default:
      break;

  }
  return QgsStyleProxyModel::data( index, role );
}

bool QgsCheckableStyleModel::setData( const QModelIndex &i, const QVariant &value, int role )
{
  if ( i.row() < 0 || i.row() >= rowCount( QModelIndex() ) ||
       ( role != Qt::EditRole && role != Qt::CheckStateRole ) )
    return false;

  if ( mReadOnly )
    return false;

  if ( role == Qt::CheckStateRole )
  {
    if ( !mCheckable || mCheckTag.isEmpty() )
      return false;

    const QString name = data( index( i.row(), QgsStyleModel::Name ), Qt::DisplayRole ).toString();
    const QgsStyle::StyleEntity entity = static_cast< QgsStyle::StyleEntity >( data( i, QgsStyleModel::TypeRole ).toInt() );

    if ( value.toInt() == Qt::Checked )
      return mStyle->tagSymbol( entity, name, QStringList() << mCheckTag );
    else
      return mStyle->detagSymbol( entity, name, QStringList() << mCheckTag );
  }
  return QgsStyleProxyModel::setData( i, value, role );
}
///@endcond

//
// QgsStyleManagerDialog
//

QgsStyleManagerDialog::QgsStyleManagerDialog( QgsStyle *style, QWidget *parent, Qt::WindowFlags flags, bool readOnly )
  : QDialog( parent, flags )
  , mStyle( style )
  , mReadOnly( readOnly )
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

  if ( !mReadOnly )
  {
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
  }
  else
  {
    btnAddTag->setEnabled( false );
    btnAddSmartgroup->setEnabled( false );
  }

  QMenu *shareMenu = new QMenu( tr( "Share Menu" ), this );
  QAction *exportAction = new QAction( tr( "Export Item(s)…" ), this );
  exportAction->setIcon( QIcon( QgsApplication::iconPath( "mActionFileSave.svg" ) ) );
  shareMenu->addAction( exportAction );
  if ( !mReadOnly )
  {
    QAction *importAction = new QAction( tr( "Import Item(s)…" ), this );
    importAction->setIcon( QIcon( QgsApplication::iconPath( "mActionFileOpen.svg" ) ) );
    shareMenu->addAction( importAction );
    connect( importAction, &QAction::triggered, this, &QgsStyleManagerDialog::importItems );
  }
  if ( mStyle != QgsStyle::defaultStyle() )
  {
    mActionCopyToDefault = new QAction( tr( "Copy Selection to Default Style…" ), this );
    shareMenu->addAction( mActionCopyToDefault );
    connect( mActionCopyToDefault, &QAction::triggered, this, &QgsStyleManagerDialog::copyItemsToDefault );
    connect( mCopyToDefaultButton, &QPushButton::clicked, this, &QgsStyleManagerDialog::copyItemsToDefault );
  }
  else
  {
    mCopyToDefaultButton->hide();
  }

  shareMenu->addSeparator();
  shareMenu->addAction( actnExportAsPNG );
  shareMenu->addAction( actnExportAsSVG );

  connect( actnExportAsPNG, &QAction::triggered, this, &QgsStyleManagerDialog::exportItemsPNG );
  connect( actnExportAsSVG, &QAction::triggered, this, &QgsStyleManagerDialog::exportItemsSVG );
  connect( exportAction, &QAction::triggered, this, &QgsStyleManagerDialog::exportItems );
  btnShare->setMenu( shareMenu );

  double iconSize = Qgis::UI_SCALE_FACTOR * fontMetrics().width( 'X' ) * 10;
  listItems->setIconSize( QSize( static_cast< int >( iconSize ), static_cast< int >( iconSize * 0.9 ) ) );  // ~100, 90 on low dpi
  double treeIconSize = Qgis::UI_SCALE_FACTOR * fontMetrics().width( 'X' ) * 2;
  mSymbolTreeView->setIconSize( QSize( static_cast< int >( treeIconSize ), static_cast< int >( treeIconSize ) ) );

  mModel = new QgsCheckableStyleModel( mStyle, this, mReadOnly );
  mModel->addDesiredIconSize( listItems->iconSize() );
  mModel->addDesiredIconSize( mSymbolTreeView->iconSize() );
  listItems->setModel( mModel );
  mSymbolTreeView->setModel( mModel );

  listItems->setSelectionBehavior( QAbstractItemView::SelectRows );
  listItems->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mSymbolTreeView->setSelectionModel( listItems->selectionModel() );
  mSymbolTreeView->setSelectionMode( listItems->selectionMode() );

  connect( listItems->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsStyleManagerDialog::symbolSelected );
  connect( listItems->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &QgsStyleManagerDialog::selectedSymbolsChanged );

  QStandardItemModel *groupModel = new QStandardItemModel( groupTree );
  groupTree->setModel( groupModel );
  groupTree->setHeaderHidden( true );
  populateGroups();
  groupTree->setCurrentIndex( groupTree->model()->index( 0, 0 ) );

  connect( groupTree->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsStyleManagerDialog::groupChanged );
  if ( !mReadOnly )
  {
    connect( groupModel, &QStandardItemModel::itemChanged,
             this, &QgsStyleManagerDialog::groupRenamed );
  }

  if ( !mReadOnly )
  {
    QMenu *groupMenu = new QMenu( tr( "Group Actions" ), this );
    connect( actnTagSymbols, &QAction::triggered, this, &QgsStyleManagerDialog::tagSymbolsAction );
    groupMenu->addAction( actnTagSymbols );
    connect( actnFinishTagging, &QAction::triggered, this, &QgsStyleManagerDialog::tagSymbolsAction );
    actnFinishTagging->setVisible( false );
    groupMenu->addAction( actnFinishTagging );
    groupMenu->addAction( actnEditSmartGroup );
    btnManageGroups->setMenu( groupMenu );
  }
  else
  {
    btnManageGroups->setEnabled( false );
  }

  connect( searchBox, &QLineEdit::textChanged, this, &QgsStyleManagerDialog::filterSymbols );

  // Context menu for groupTree
  groupTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( groupTree, &QWidget::customContextMenuRequested,
           this, &QgsStyleManagerDialog::grouptreeContextMenu );

  // Context menu for listItems
  listItems->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( listItems, &QWidget::customContextMenuRequested,
           this, &QgsStyleManagerDialog::listitemsContextMenu );
  mSymbolTreeView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mSymbolTreeView, &QWidget::customContextMenuRequested,
           this, &QgsStyleManagerDialog::listitemsContextMenu );

  if ( !mReadOnly )
  {
    // Menu for the "Add item" toolbutton when in colorramp mode
    QStringList rampTypes;
    rampTypes << tr( "Gradient" ) << tr( "Color presets" ) << tr( "Random" ) << tr( "Catalog: cpt-city" );
    rampTypes << tr( "Catalog: ColorBrewer" );

    mMenuBtnAddItemAll = new QMenu( this );
    mMenuBtnAddItemColorRamp = new QMenu( this );

    QAction *item = new QAction( tr( "Marker" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addSymbol( QgsSymbol::Marker ); } );
    mMenuBtnAddItemAll->addAction( item );
    item = new QAction( tr( "Line" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addSymbol( QgsSymbol::Line ); } );
    mMenuBtnAddItemAll->addAction( item );
    item = new QAction( tr( "Fill" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addSymbol( QgsSymbol::Fill ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemAll->addSeparator();
    for ( const QString &rampType : qgis::as_const( rampTypes ) )
    {
      item = new QAction( rampType, this );
      connect( item, &QAction::triggered, this, [ = ]( bool ) { addColorRamp( item ); } );
      mMenuBtnAddItemAll->addAction( item );
      mMenuBtnAddItemColorRamp->addAction( new QAction( rampType, this ) );
    }

    connect( mMenuBtnAddItemColorRamp, &QMenu::triggered,
             this, static_cast<bool ( QgsStyleManagerDialog::* )( QAction * )>( &QgsStyleManagerDialog::addColorRamp ) );
  }

  // Context menu for symbols/colorramps. The menu entries for every group are created when displaying the menu.
  mGroupMenu = new QMenu( this );
  mGroupListMenu = new QMenu( mGroupMenu );
  mGroupListMenu->setTitle( tr( "Add to Tag" ) );
  mGroupListMenu->setEnabled( false );
  if ( !mReadOnly )
  {
    connect( actnAddFavorite, &QAction::triggered, this, &QgsStyleManagerDialog::addFavoriteSelectedSymbols );
    mGroupMenu->addAction( actnAddFavorite );
    connect( actnRemoveFavorite, &QAction::triggered, this, &QgsStyleManagerDialog::removeFavoriteSelectedSymbols );
    mGroupMenu->addAction( actnRemoveFavorite );
    mGroupMenu->addSeparator()->setParent( this );
    mGroupMenu->addMenu( mGroupListMenu );
    actnDetag->setData( 0 );
    connect( actnDetag, &QAction::triggered, this, &QgsStyleManagerDialog::detagSelectedSymbols );
    mGroupMenu->addAction( actnDetag );
    mGroupMenu->addSeparator()->setParent( this );
    mGroupMenu->addAction( actnRemoveItem );
    mGroupMenu->addAction( actnEditItem );
    mGroupMenu->addSeparator()->setParent( this );
  }
  else
  {
    btnAddItem->setVisible( false );
    btnRemoveItem->setVisible( false );
    btnEditItem->setVisible( false );
    btnAddSmartgroup->setVisible( false );
    btnAddTag->setVisible( false );
    btnManageGroups->setVisible( false );
  }
  if ( mActionCopyToDefault )
  {
    mGroupMenu->addAction( mActionCopyToDefault );
  }
  mGroupMenu->addAction( actnExportAsPNG );
  mGroupMenu->addAction( actnExportAsSVG );

  // Context menu for the group tree
  mGroupTreeContextMenu = new QMenu( this );
  if ( !mReadOnly )
  {
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
  }

  tabItemType_currentChanged( 0 );

  connect( mStyle, &QgsStyle::symbolSaved, this, &QgsStyleManagerDialog::populateList );
  connect( mStyle, &QgsStyle::groupsModified, this, &QgsStyleManagerDialog::populateGroups );

  connect( mButtonIconView, &QToolButton::toggled, this, [ = ]( bool active )
  {
    if ( active )
    {
      mSymbolViewStackedWidget->setCurrentIndex( 0 );
      // note -- we have to save state here and not in destructor, as new symbol list widgets are created before the previous ones are destroyed
      QgsSettings().setValue( QStringLiteral( "Windows/StyleV2Manager/lastIconView" ), 0, QgsSettings::Gui );
    }
  } );
  connect( mButtonListView, &QToolButton::toggled, this, [ = ]( bool active )
  {
    if ( active )
    {
      QgsSettings().setValue( QStringLiteral( "Windows/StyleV2Manager/lastIconView" ), 1, QgsSettings::Gui );
      mSymbolViewStackedWidget->setCurrentIndex( 1 );
    }
  } );
  // restore previous view
  const int currentView = settings.value( QStringLiteral( "Windows/StyleV2Manager/lastIconView" ), 0, QgsSettings::Gui ).toInt();
  if ( currentView == 0 )
    mButtonIconView->setChecked( true );
  else
    mButtonListView->setChecked( true );

  mSymbolTreeView->header()->restoreState( settings.value( QStringLiteral( "Windows/StyleV2Manager/treeState" ), QByteArray(), QgsSettings::Gui ).toByteArray() );
  connect( mSymbolTreeView->header(), &QHeaderView::sectionResized, this, [this]
  {
    // note -- we have to save state here and not in destructor, as new symbol list widgets are created before the previous ones are destroyed
    QgsSettings().setValue( QStringLiteral( "Windows/StyleV2Manager/treeState" ), mSymbolTreeView->header()->saveState(), QgsSettings::Gui );
  } );

  // set initial disabled state for actions requiring a selection
  selectedSymbolsChanged( QItemSelection(), QItemSelection() );
}

void QgsStyleManagerDialog::onFinished()
{
  if ( mModified && !mReadOnly )
  {
    mStyle->save();
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/StyleV2Manager/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/StyleV2Manager/splitter" ), mSplitter->saveState() );
}

void QgsStyleManagerDialog::populateTypes()
{
}

void QgsStyleManagerDialog::tabItemType_currentChanged( int )
{
  // when in Color Ramp tab, add menu to add item button and hide "Export symbols as PNG/SVG"
  const bool isSymbol = currentItemType() != 3;
  searchBox->setPlaceholderText( isSymbol ? tr( "Filter symbols…" ) : tr( "Filter color ramps…" ) );

  if ( !mReadOnly && !isSymbol ) // color ramp tab
  {
    btnAddItem->setMenu( mMenuBtnAddItemColorRamp );
  }
  else if ( !mReadOnly && tabItemType->currentIndex() == 0 ) // all symbols tab
  {
    btnAddItem->setMenu( mMenuBtnAddItemAll );
  }
  else
  {
    btnAddItem->setMenu( nullptr );
  }

  actnExportAsPNG->setVisible( isSymbol );
  actnExportAsSVG->setVisible( isSymbol );

  mModel->setEntityFilter( isSymbol  ? QgsStyle::SymbolEntity : QgsStyle::ColorrampEntity );
  mModel->setEntityFilterEnabled( !allTypesSelected() );
  mModel->setSymbolTypeFilterEnabled( isSymbol && !allTypesSelected() );
  if ( isSymbol && !allTypesSelected() )
    mModel->setSymbolType( static_cast< QgsSymbol::SymbolType >( currentItemType() ) );

  populateList();
}

void QgsStyleManagerDialog::copyItemsToDefault()
{
  const QList< ItemDetails > items = selectedItems();
  if ( !items.empty() )
  {
    bool ok = false;
    const QString tags = QInputDialog::getText( this, tr( "Import Items" ),
                         tr( "Additional tags to add (comma separated)" ), QLineEdit::Normal,
                         mBaseName, &ok );
    if ( !ok )
      return;

    const QStringList parts = tags.split( ',', QString::SkipEmptyParts );
    QStringList additionalTags;
    additionalTags.reserve( parts.count() );
    for ( const QString &tag : parts )
      additionalTags << tag.trimmed();

    auto cursorOverride = qgis::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
    const int count = copyItems( items, mStyle, QgsStyle::defaultStyle(), this, cursorOverride, true, additionalTags, false, false );
    cursorOverride.reset();
    if ( count > 0 )
    {
      QMessageBox::information( this, tr( "Import Items" ),
                                count > 1 ? tr( "Successfully imported %1 items." ).arg( count )
                                : tr( "Successfully imported item." ) );
    }
  }
}

int QgsStyleManagerDialog::selectedItemType()
{
  QModelIndex index = listItems->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return 0;

  const QgsStyle::StyleEntity entity = static_cast< QgsStyle::StyleEntity >( mModel->data( index, QgsStyleModel::TypeRole ).toInt() );
  if ( entity == QgsStyle::ColorrampEntity )
    return 3;

  return  mModel->data( index, QgsStyleModel::SymbolTypeRole ).toInt();
}

bool QgsStyleManagerDialog::allTypesSelected() const
{
  return tabItemType->currentIndex() == 0;
}

QList< QgsStyleManagerDialog::ItemDetails > QgsStyleManagerDialog::selectedItems()
{
  QList<QgsStyleManagerDialog::ItemDetails > res;
  QModelIndexList indices = listItems->selectionModel()->selectedRows();
  for ( const QModelIndex &index : indices )
  {
    if ( !index.isValid() )
      continue;

    ItemDetails details;
    details.entityType = static_cast< QgsStyle::StyleEntity >( mModel->data( index, QgsStyleModel::TypeRole ).toInt() );
    if ( details.entityType == QgsStyle::SymbolEntity )
      details.symbolType = static_cast< QgsSymbol::SymbolType >( mModel->data( index, QgsStyleModel::SymbolTypeRole ).toInt() );
    details.name = mModel->data( mModel->index( index.row(), QgsStyleModel::Name, index.parent() ), Qt::DisplayRole ).toString();

    res << details;
  }
  return res;
}

int QgsStyleManagerDialog::copyItems( const QList<QgsStyleManagerDialog::ItemDetails> &items, QgsStyle *src, QgsStyle *dst, QWidget *parentWidget,
                                      std::unique_ptr< QgsTemporaryCursorOverride > &cursorOverride, bool isImport, const QStringList &importTags, bool addToFavorites, bool ignoreSourceTags )
{
  bool prompt = true;
  bool overwriteAll = true;
  int count = 0;

  const QStringList favoriteSymbols = src->symbolsOfFavorite( QgsStyle::SymbolEntity );
  const QStringList favoriteColorramps = src->symbolsOfFavorite( QgsStyle::ColorrampEntity );

  for ( auto &details : items )
  {
    QStringList symbolTags;
    if ( !ignoreSourceTags )
    {
      symbolTags = src->tagsOfSymbol( details.entityType, details.name );
    }

    bool addItemToFavorites = false;
    if ( isImport )
    {
      symbolTags << importTags;
      addItemToFavorites = addToFavorites;
    }

    switch ( details.entityType )
    {
      case QgsStyle::SymbolEntity:
      {
        std::unique_ptr< QgsSymbol > symbol( src->symbol( details.name ) );
        if ( !symbol )
          continue;

        const bool hasDuplicateName = dst->symbolNames().contains( details.name );
        bool overwriteThis = false;
        if ( isImport )
          addItemToFavorites = favoriteSymbols.contains( details.name );

        if ( hasDuplicateName && prompt )
        {
          cursorOverride.reset();
          int res = QMessageBox::warning( parentWidget, isImport ? tr( "Import Symbol" ) : tr( "Export Symbol" ),
                                          tr( "A symbol with the name “%1” already exists.\nOverwrite?" )
                                          .arg( details.name ),
                                          QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
          cursorOverride = qgis::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
          switch ( res )
          {
            case QMessageBox::Cancel:
              return count;

            case QMessageBox::No:
              continue;

            case QMessageBox::Yes:
              overwriteThis = true;
              break;

            case QMessageBox::YesToAll:
              prompt = false;
              overwriteAll = true;
              break;

            case QMessageBox::NoToAll:
              prompt = false;
              overwriteAll = false;
              break;
          }
        }

        if ( !hasDuplicateName || overwriteAll || overwriteThis )
        {
          QgsSymbol *newSymbol = symbol.get();
          dst->addSymbol( details.name, symbol.release() );
          dst->saveSymbol( details.name, newSymbol, addItemToFavorites, symbolTags );
          count++;
        }
        break;
      }

      case QgsStyle::ColorrampEntity:
      {
        std::unique_ptr< QgsColorRamp > ramp( src->colorRamp( details.name ) );
        if ( !ramp )
          continue;

        const bool hasDuplicateName = dst->colorRampNames().contains( details.name );
        bool overwriteThis = false;
        if ( isImport )
          addItemToFavorites = favoriteColorramps.contains( details.name );

        if ( hasDuplicateName && prompt )
        {
          cursorOverride.reset();
          int res = QMessageBox::warning( parentWidget, isImport ? tr( "Import Color Ramp" ) : tr( "Export Color Ramp" ),
                                          tr( "A color ramp with the name “%1” already exists.\nOverwrite?" )
                                          .arg( details.name ),
                                          QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
          cursorOverride = qgis::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
          switch ( res )
          {
            case QMessageBox::Cancel:
              return count;

            case QMessageBox::No:
              continue;

            case QMessageBox::Yes:
              overwriteThis = true;
              break;

            case QMessageBox::YesToAll:
              prompt = false;
              overwriteAll = true;
              break;

            case QMessageBox::NoToAll:
              prompt = false;
              overwriteAll = false;
              break;
          }
        }

        if ( !hasDuplicateName || overwriteAll || overwriteThis )
        {
          QgsColorRamp *newRamp = ramp.get();
          dst->addColorRamp( details.name, ramp.release() );
          dst->saveColorRamp( details.name, newRamp, addItemToFavorites, symbolTags );
          count++;
        }
        break;
      }

      case QgsStyle::TagEntity:
      case QgsStyle::SmartgroupEntity:
        break;

    }
  }
  return count;
}

void QgsStyleManagerDialog::populateList()
{
  groupChanged( groupTree->selectionModel()->currentIndex() );
}

void QgsStyleManagerDialog::populateSymbols( const QStringList &, bool )
{
}

void QgsStyleManagerDialog::populateColorRamps( const QStringList &, bool )
{
}

int QgsStyleManagerDialog::currentItemType()
{
  switch ( tabItemType->currentIndex() )
  {
    case 1:
      return QgsSymbol::Marker;
    case 2:
      return QgsSymbol::Line;
    case 3:
      return QgsSymbol::Fill;
    case 4:
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

  return mModel->data( mModel->index( index.row(), QgsStyleModel::Name, index.parent() ), Qt::DisplayRole ).toString();
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
  }
}

bool QgsStyleManagerDialog::addSymbol( int symbolType )
{
  // create new symbol with current type
  QgsSymbol *symbol = nullptr;
  QString name = tr( "new symbol" );
  switch ( symbolType == -1 ? currentItemType() : symbolType )
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

void QgsStyleManagerDialog::setFavoritesGroupVisible( bool show )
{
  mFavoritesGroupVisible = show;
  populateGroups();
}

void QgsStyleManagerDialog::setSmartGroupsVisible( bool show )
{
  mSmartGroupVisible = show;
  populateGroups();
}

void QgsStyleManagerDialog::setBaseStyleName( const QString &name )
{
  mBaseName = name;
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
  if ( selectedItemType() < 3 )
  {
    editSymbol();
  }
  else if ( selectedItemType() == 3 )
  {
    editColorRamp();
  }
  else
  {
    Q_ASSERT( false && "not implemented" );
  }
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
  const QList< ItemDetails > items = selectedItems();

  if ( allTypesSelected() )
  {
    if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Items" ),
         QString( tr( "Do you really want to remove %n item(s)?", nullptr, items.count() ) ),
         QMessageBox::Yes,
         QMessageBox::No ) )
      return;
  }
  else
  {
    if ( currentItemType() < 3 )
    {
      if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Symbol" ),
           QString( tr( "Do you really want to remove %n symbol(s)?", nullptr, items.count() ) ),
           QMessageBox::Yes,
           QMessageBox::No ) )
        return;
    }
    else
    {
      if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Color Ramp" ),
           QString( tr( "Do you really want to remove %n ramp(s)?", nullptr, items.count() ) ),
           QMessageBox::Yes,
           QMessageBox::No ) )
        return;
    }
  }

  QgsTemporaryCursorOverride override( Qt::WaitCursor );

  for ( const ItemDetails &details : items )
  {
    if ( details.name.isEmpty() )
      continue;

    if ( details.entityType == QgsStyle::SymbolEntity )
      mStyle->removeSymbol( details.name );
    else if ( details.entityType == QgsStyle::ColorrampEntity )
      mStyle->removeColorRamp( details.name );
  }

  mModified = true;
}

bool QgsStyleManagerDialog::removeSymbol()
{
  return false;
}

bool QgsStyleManagerDialog::removeColorRamp()
{
  return false;
}

void QgsStyleManagerDialog::itemChanged( QStandardItem * )
{
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

  const QList< ItemDetails > items = selectedItems();
  for ( const ItemDetails &details : items )
  {
    if ( details.entityType != QgsStyle::SymbolEntity )
      continue;

    QString path = dir + '/' + details.name + '.' + format;
    std::unique_ptr< QgsSymbol > sym( mStyle->symbol( details.name ) );
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
  if ( mBlockGroupUpdates )
    return;

  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( groupTree->model() );
  model->clear();

  if ( mFavoritesGroupVisible )
  {
    QStandardItem *favoriteSymbols = new QStandardItem( tr( "Favorites" ) );
    favoriteSymbols->setData( "favorite" );
    favoriteSymbols->setEditable( false );
    setBold( favoriteSymbols );
    model->appendRow( favoriteSymbols );
  }

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
    item->setEditable( !mReadOnly );
    taggroup->appendRow( item );
  }
  taggroup->setText( tr( "Tags" ) );//set title later
  setBold( taggroup );
  model->appendRow( taggroup );

  if ( mSmartGroupVisible )
  {
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
      item->setEditable( !mReadOnly );
      smart->appendRow( item );
      ++i;
    }
    model->appendRow( smart );
  }

  // expand things in the group tree
  int rows = model->rowCount( model->indexFromItem( model->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    groupTree->setExpanded( model->indexFromItem( model->item( i ) ), true );
  }
}

void QgsStyleManagerDialog::groupChanged( const QModelIndex &index )
{
  QStringList groupSymbols;

  const QString category = index.data( Qt::UserRole + 1 ).toString();
  if ( mGroupingMode )
  {
    mModel->setTagId( -1 );
    mModel->setSmartGroupId( -1 );
    mModel->setFavoritesOnly( false );
    mModel->setCheckTag( index.data( Qt::DisplayRole ).toString() );
  }
  else if ( category == QLatin1String( "all" ) || category == QLatin1String( "tags" ) || category == QLatin1String( "smartgroups" ) )
  {
    enableGroupInputs( false );
    if ( category == QLatin1String( "tags" ) )
    {
      actnAddTag->setEnabled( !mReadOnly );
      actnAddSmartgroup->setEnabled( false );
    }
    else if ( category == QLatin1String( "smartgroups" ) )
    {
      actnAddTag->setEnabled( false );
      actnAddSmartgroup->setEnabled( !mReadOnly );
    }

    mModel->setTagId( -1 );
    mModel->setSmartGroupId( -1 );
    mModel->setFavoritesOnly( false );
  }
  else if ( category == QLatin1String( "favorite" ) )
  {
    enableGroupInputs( false );
    mModel->setTagId( -1 );
    mModel->setSmartGroupId( -1 );
    mModel->setFavoritesOnly( true );
  }
  else if ( index.parent().data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    actnRemoveGroup->setEnabled( !mReadOnly );
    btnManageGroups->setEnabled( !mReadOnly );
    const int groupId = index.data( Qt::UserRole + 1 ).toInt();
    mModel->setTagId( -1 );
    mModel->setSmartGroupId( groupId );
    mModel->setFavoritesOnly( false );
  }
  else // tags
  {
    enableGroupInputs( true );
    int tagId = index.data( Qt::UserRole + 1 ).toInt();
    mModel->setTagId( tagId );
    mModel->setSmartGroupId( -1 );
    mModel->setFavoritesOnly( false );
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
      actnEditSmartGroup->setVisible( !mGroupingMode && !mReadOnly );
    }
    else if ( index.parent().data( Qt::UserRole + 1 ).toString() == QLatin1String( "tags" ) )
    {
      actnAddTag->setVisible( !mGroupingMode && !mReadOnly );
      actnTagSymbols->setVisible( !mGroupingMode && !mReadOnly );
      actnFinishTagging->setVisible( mGroupingMode && !mReadOnly );
    }
    actnRemoveGroup->setVisible( !mReadOnly );
  }
  else if ( index.data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    actnAddSmartgroup->setVisible( !mGroupingMode && !mReadOnly );
  }
  else if ( index.data( Qt::UserRole + 1 ) == "tags" )
  {
    actnAddTag->setVisible( !mGroupingMode && !mReadOnly );
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

  // block the auto-repopulation of groups when the style emits groupsModified
  // instead, we manually update the model items for better state retention
  mBlockGroupUpdates++;
  id = mStyle->addTag( itemName );
  mBlockGroupUpdates--;

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

  // block the auto-repopulation of groups when the style emits groupsModified
  // instead, we manually update the model items for better state retention
  mBlockGroupUpdates++;
  id = mStyle->addSmartgroup( dlg.smartgroupName(), dlg.conditionOperator(), dlg.conditionMap() );
  mBlockGroupUpdates--;

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

  // block the auto-repopulation of groups when the style emits groupsModified
  // instead, we manually update the model items for better state retention
  mBlockGroupUpdates++;

  if ( parentItem->data( Qt::UserRole + 1 ).toString() == QLatin1String( "smartgroups" ) )
  {
    mStyle->remove( QgsStyle::SmartgroupEntity, index.data( Qt::UserRole + 1 ).toInt() );
  }
  else
  {
    mStyle->remove( QgsStyle::TagEntity, index.data( Qt::UserRole + 1 ).toInt() );
  }

  mBlockGroupUpdates--;
  parentItem->removeRow( index.row() );
}

void QgsStyleManagerDialog::groupRenamed( QStandardItem *item )
{
  QgsDebugMsg( QStringLiteral( "Symbol group edited: data=%1 text=%2" ).arg( item->data( Qt::UserRole + 1 ).toString(), item->text() ) );
  int id = item->data( Qt::UserRole + 1 ).toInt();
  QString name = item->text();
  mBlockGroupUpdates++;
  if ( item->parent()->data( Qt::UserRole + 1 ) == "smartgroups" )
  {
    mStyle->rename( QgsStyle::SmartgroupEntity, id, name );
  }
  else
  {
    mStyle->rename( QgsStyle::TagEntity, id, name );
  }
  mBlockGroupUpdates--;
}

void QgsStyleManagerDialog::tagSymbolsAction()
{
  QStandardItemModel *treeModel = qobject_cast<QStandardItemModel *>( groupTree->model() );

  if ( mGroupingMode )
  {
    mGroupingMode = false;
    mModel->setCheckable( false );
    actnTagSymbols->setVisible( true );
    actnFinishTagging->setVisible( false );
    // disconnect slot which handles regrouping

    // disable all items except groups in groupTree
    enableItemsForGroupingMode( true );
    groupChanged( groupTree->currentIndex() );

    // Finally: Reconnect all Symbol editing functionalities
    connect( treeModel, &QStandardItemModel::itemChanged,
             this, &QgsStyleManagerDialog::groupRenamed );

    // Reset the selection mode
    listItems->setSelectionMode( QAbstractItemView::ExtendedSelection );
    mSymbolTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
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

    mGroupingMode = true;
    // Change visibility of actions
    actnTagSymbols->setVisible( false );
    actnFinishTagging->setVisible( true );
    // Remove all Symbol editing functionalities
    disconnect( treeModel, &QStandardItemModel::itemChanged,
                this, &QgsStyleManagerDialog::groupRenamed );

    // disable all items except groups in groupTree
    enableItemsForGroupingMode( false );
    groupChanged( groupTree->currentIndex() );
    btnManageGroups->setEnabled( true );

    mModel->setCheckable( true );

    // No selection should be possible
    listItems->setSelectionMode( QAbstractItemView::NoSelection );
    mSymbolTreeView->setSelectionMode( QAbstractItemView::NoSelection );
  }
}

void QgsStyleManagerDialog::regrouped( QStandardItem * )
{
}

void QgsStyleManagerDialog::setSymbolsChecked( const QStringList & )
{
}

void QgsStyleManagerDialog::filterSymbols( const QString &qword )
{
  mModel->setFilterString( qword );
}

void QgsStyleManagerDialog::symbolSelected( const QModelIndex &index )
{
  actnEditItem->setEnabled( index.isValid() && !mGroupingMode && !mReadOnly );
}

void QgsStyleManagerDialog::selectedSymbolsChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  bool nothingSelected = listItems->selectionModel()->selectedIndexes().empty();
  actnRemoveItem->setDisabled( nothingSelected || mReadOnly );
  actnAddFavorite->setDisabled( nothingSelected || mReadOnly );
  actnRemoveFavorite->setDisabled( nothingSelected || mReadOnly );
  mGroupListMenu->setDisabled( nothingSelected || mReadOnly );
  actnDetag->setDisabled( nothingSelected || mReadOnly );
  actnExportAsPNG->setDisabled( nothingSelected );
  actnExportAsSVG->setDisabled( nothingSelected );
  if ( mActionCopyToDefault )
    mActionCopyToDefault->setDisabled( nothingSelected );
  mCopyToDefaultButton->setDisabled( nothingSelected );
  actnEditItem->setDisabled( nothingSelected || mReadOnly );
}

void QgsStyleManagerDialog::enableSymbolInputs( bool enable )
{
  groupTree->setEnabled( enable );
  btnAddTag->setEnabled( enable && !mReadOnly );
  btnAddSmartgroup->setEnabled( enable && !mReadOnly );
  actnAddTag->setEnabled( enable && !mReadOnly );
  actnAddSmartgroup->setEnabled( enable && !mReadOnly );
  actnRemoveGroup->setEnabled( enable && !mReadOnly );
  btnManageGroups->setEnabled( !mReadOnly && ( enable || mGroupingMode ) ); // always enabled in grouping mode, as it is the only way to leave grouping mode
  searchBox->setEnabled( enable );
}

void QgsStyleManagerDialog::enableGroupInputs( bool enable )
{
  actnRemoveGroup->setEnabled( enable && !mReadOnly );
  btnManageGroups->setEnabled( !mReadOnly && ( enable || mGroupingMode ) ); // always enabled in grouping mode, as it is the only way to leave grouping mode
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
    QWidget *w = symbolBtnsLayout->itemAt( i )->widget();
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
  if ( index.isValid() && !mGroupingMode )
    mGroupTreeContextMenu->popup( globalPos );
}

void QgsStyleManagerDialog::listitemsContextMenu( QPoint point )
{
  QPoint globalPos = mSymbolViewStackedWidget->currentIndex() == 0
                     ? listItems->viewport()->mapToGlobal( point )
                     : mSymbolTreeView->viewport()->mapToGlobal( point );

  // Clear all actions and create new actions for every group
  mGroupListMenu->clear();

  if ( !mReadOnly )
  {
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
  }

  mGroupMenu->popup( globalPos );
}

void QgsStyleManagerDialog::addFavoriteSelectedSymbols()
{
  const QList< ItemDetails > items = selectedItems();
  for ( const ItemDetails &details : items )
  {
    mStyle->addFavorite( details.entityType, details.name );
  }
}

void QgsStyleManagerDialog::removeFavoriteSelectedSymbols()
{
  const QList< ItemDetails > items = selectedItems();
  for ( const ItemDetails &details : items )
  {
    mStyle->removeFavorite( details.entityType, details.name );
  }
}

void QgsStyleManagerDialog::tagSelectedSymbols( bool newTag )
{
  QAction *selectedItem = qobject_cast<QAction *>( sender() );
  if ( selectedItem )
  {
    const QList< ItemDetails > items = selectedItems();
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

    for ( const ItemDetails &details : items )
    {
      mStyle->tagSymbol( details.entityType, details.name, QStringList( tag ) );
    }
  }
}

void QgsStyleManagerDialog::detagSelectedSymbols()
{
  QAction *selectedItem = qobject_cast<QAction *>( sender() );

  if ( selectedItem )
  {
    const QList< ItemDetails > items = selectedItems();
    for ( const ItemDetails &details : items )
    {
      mStyle->detagSymbol( details.entityType, details.name );
    }
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

  mBlockGroupUpdates++;
  mStyle->remove( QgsStyle::SmartgroupEntity, item->data().toInt() );
  int id = mStyle->addSmartgroup( dlg.smartgroupName(), dlg.conditionOperator(), dlg.conditionMap() );
  mBlockGroupUpdates--;
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

