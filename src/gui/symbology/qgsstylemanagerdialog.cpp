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
#include "qgsmessagebar.h"
#include "qgstextformatwidget.h"
#include "qgslabelinggui.h"
#include "qgslegendpatchshapewidget.h"
#include "qgsabstract3dsymbol.h"
#include "qgs3dsymbolregistry.h"
#include "qgs3dsymbolwidget.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsiconutils.h"

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QMenu>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
#include <QShortcut>

#include "qgsapplication.h"
#include "qgslogger.h"

//
// QgsCheckableStyleModel
//

///@cond PRIVATE
QgsCheckableStyleModel::QgsCheckableStyleModel( QgsStyleModel *sourceModel, QObject *parent, bool readOnly )
  : QgsStyleProxyModel( sourceModel, parent )
  , mStyle( sourceModel->style() )
  , mReadOnly( readOnly )
{

}

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

#include "qgsgui.h"

QString QgsStyleManagerDialog::sPreviousTag;

QgsStyleManagerDialog::QgsStyleManagerDialog( QgsStyle *style, QWidget *parent, Qt::WindowFlags flags, bool readOnly )
  : QDialog( parent, flags )
  , mStyle( style )
  , mReadOnly( readOnly )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  connect( tabItemType, &QTabWidget::currentChanged, this, &QgsStyleManagerDialog::tabItemType_currentChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsStyleManagerDialog::showHelp );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsStyleManagerDialog::onClose );

  QPushButton *downloadButton = buttonBox->addButton( tr( "Browse Online Styles" ), QDialogButtonBox::ResetRole );
  downloadButton->setToolTip( tr( "Download new styles from the online QGIS style repository" ) );
  downloadButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFindReplace.svg" ) ) );
  connect( downloadButton, &QPushButton::clicked, this, [ = ]
  {
    QDesktopServices::openUrl( QUrl( QStringLiteral( "https://plugins.qgis.org/styles" ) ) );
  } );

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  mVerticalLayout->insertWidget( 0,  mMessageBar );

#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  QgsSettings settings;

  mSplitter->setSizes( QList<int>() << 170 << 540 );
  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/StyleV2Manager/splitter" ) ).toByteArray() );

  tabItemType->setDocumentMode( true );
  searchBox->setShowSearchIcon( true );
  searchBox->setPlaceholderText( tr( "Filter symbols…" ) );

  connect( this, &QDialog::finished, this, &QgsStyleManagerDialog::onFinished );
  connect( listItems, &QAbstractItemView::doubleClicked, this, &QgsStyleManagerDialog::editItem );
  connect( btnEditItem, &QPushButton::clicked, this, [ = ]( bool ) { editItem(); }
         );
  connect( actnEditItem, &QAction::triggered, this, [ = ]( bool ) { editItem(); }
         );

  if ( !mReadOnly )
  {
    connect( btnAddItem, &QPushButton::clicked, this, [ = ]( bool )
    {
      // only show add item if the btn doesn't have a menu -- otherwise it should show the menu instead!
      if ( !btnAddItem->menu() )
      { addItem(); }
    }
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

  mActionCopyItem = new QAction( tr( "Copy Item" ), this );
  connect( mActionCopyItem, &QAction::triggered, this, &QgsStyleManagerDialog::copyItem );
  mActionPasteItem = new QAction( tr( "Paste Item…" ), this );
  connect( mActionPasteItem, &QAction::triggered, this, &QgsStyleManagerDialog::pasteItem );

  QShortcut *copyShortcut = new QShortcut( QKeySequence( QKeySequence::StandardKey::Copy ), this );
  connect( copyShortcut, &QShortcut::activated, this, &QgsStyleManagerDialog::copyItem );
  QShortcut *pasteShortcut = new QShortcut( QKeySequence( QKeySequence::StandardKey::Paste ), this );
  connect( pasteShortcut, &QShortcut::activated, this, &QgsStyleManagerDialog::pasteItem );
  QShortcut *removeShortcut = new QShortcut( QKeySequence( QKeySequence::StandardKey::Delete ), this );
  connect( removeShortcut, &QShortcut::activated, this, &QgsStyleManagerDialog::removeItem );
  QShortcut *editShortcut = new QShortcut( QKeySequence( Qt::Key_Return ), this );
  connect( editShortcut, &QShortcut::activated, this, &QgsStyleManagerDialog::editItem );

  shareMenu->addSeparator();
  shareMenu->addAction( actnExportAsPNG );
  shareMenu->addAction( actnExportAsSVG );

  connect( actnExportAsPNG, &QAction::triggered, this, &QgsStyleManagerDialog::exportItemsPNG );
  connect( actnExportAsSVG, &QAction::triggered, this, &QgsStyleManagerDialog::exportItemsSVG );
  connect( exportAction, &QAction::triggered, this, &QgsStyleManagerDialog::exportItems );
  btnShare->setMenu( shareMenu );

  listItems->setTextElideMode( Qt::TextElideMode::ElideRight );
  double treeIconSize = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 2;
  mSymbolTreeView->setIconSize( QSize( static_cast< int >( treeIconSize ), static_cast< int >( treeIconSize ) ) );

  mModel = mStyle == QgsStyle::defaultStyle() ? new QgsCheckableStyleModel( QgsApplication::defaultStyleModel(), this, mReadOnly )
           : new QgsCheckableStyleModel( mStyle, this, mReadOnly );
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

  const QModelIndexList prevIndex = groupTree->model()->match( groupTree->model()->index( 0, 0 ), Qt::UserRole + 1, sPreviousTag, 1, Qt::MatchFixedString | Qt::MatchCaseSensitive | Qt::MatchRecursive );
  groupTree->setCurrentIndex( !prevIndex.empty() ? prevIndex.at( 0 ) : groupTree->model()->index( 0, 0 ) );

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
    mMenuBtnAddItemAll = new QMenu( this );
    mMenuBtnAddItemColorRamp = new QMenu( this );
    mMenuBtnAddItemLabelSettings = new QMenu( this );
    mMenuBtnAddItemLegendPatchShape = new QMenu( this );
    mMenuBtnAddItemSymbol3D = new QMenu( this );

    QAction *item = new QAction( QgsIconUtils::iconPoint(), tr( "Marker…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addSymbol( static_cast< int >( Qgis::SymbolType::Marker ) ); } );
    mMenuBtnAddItemAll->addAction( item );
    item = new QAction( QgsIconUtils::iconLine(), tr( "Line…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addSymbol( static_cast< int >( Qgis::SymbolType::Line ) ); } );
    mMenuBtnAddItemAll->addAction( item );
    item = new QAction( QgsIconUtils::iconPolygon(), tr( "Fill…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addSymbol( static_cast< int >( Qgis::SymbolType::Fill ) ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemAll->addSeparator();

    const QList< QPair< QString, QString > > rampTypes = QgsColorRamp::rampTypes();
    for ( const QPair< QString, QString > &rampType : rampTypes )
    {
      item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/color.svg" ) ), tr( "%1…" ).arg( rampType.second ), this );
      connect( item, &QAction::triggered, this, [ = ]( bool ) { addColorRamp( rampType.first ); } );
      mMenuBtnAddItemAll->addAction( item );
      mMenuBtnAddItemColorRamp->addAction( item );
    }
    mMenuBtnAddItemAll->addSeparator();
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mIconFieldText.svg" ) ), tr( "Text Format…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addTextFormat(); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemAll->addSeparator();
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Point Label Settings…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addLabelSettings( QgsWkbTypes::PointGeometry ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemLabelSettings->addAction( item );
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Line Label Settings…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) {  addLabelSettings( QgsWkbTypes::LineGeometry ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemLabelSettings->addAction( item );
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Polygon Label Settings…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) {  addLabelSettings( QgsWkbTypes::PolygonGeometry ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemLabelSettings->addAction( item );

    mMenuBtnAddItemAll->addSeparator();
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "legend.svg" ) ), tr( "Marker Legend Patch Shape…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addLegendPatchShape( Qgis::SymbolType::Marker ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemLegendPatchShape->addAction( item );
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "legend.svg" ) ), tr( "Line Legend Patch Shape…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) {  addLegendPatchShape( Qgis::SymbolType::Line ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemLegendPatchShape->addAction( item );
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "legend.svg" ) ), tr( "Fill Legend Patch Shape…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) {  addLegendPatchShape( Qgis::SymbolType::Fill ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemLegendPatchShape->addAction( item );

    mMenuBtnAddItemAll->addSeparator();
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "3d.svg" ) ), tr( "3D Point Symbol…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) { addSymbol3D( QStringLiteral( "point" ) ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemSymbol3D->addAction( item );
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "3d.svg" ) ), tr( "3D Line Symbol…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) {  addSymbol3D( QStringLiteral( "line" ) ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemSymbol3D->addAction( item );
    item = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "3d.svg" ) ), tr( "3D Polygon Symbol…" ), this );
    connect( item, &QAction::triggered, this, [ = ]( bool ) {  addSymbol3D( QStringLiteral( "polygon" ) ); } );
    mMenuBtnAddItemAll->addAction( item );
    mMenuBtnAddItemSymbol3D->addAction( item );
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
    mGroupMenu->addAction( mActionCopyItem );
    mGroupMenu->addAction( mActionPasteItem );
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

    mGroupMenu->addAction( mActionCopyItem );
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

  const int thumbnailSize = settings.value( QStringLiteral( "Windows/StyleV2Manager/thumbnailSize" ), 0, QgsSettings::Gui ).toInt();
  mSliderIconSize->setValue( thumbnailSize );
  connect( mSliderIconSize, &QSlider::valueChanged, this, &QgsStyleManagerDialog::setThumbnailSize );
  setThumbnailSize( thumbnailSize );

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
  settings.setValue( QStringLiteral( "Windows/StyleV2Manager/splitter" ), mSplitter->saveState() );
}

void QgsStyleManagerDialog::populateTypes()
{
}

void QgsStyleManagerDialog::tabItemType_currentChanged( int )
{
  // when in Color Ramp tab, add menu to add item button and hide "Export symbols as PNG/SVG"
  const bool isSymbol = currentItemType() != 3 && currentItemType() != 4 && currentItemType() != 5 && currentItemType() != 6 && currentItemType() != 7;
  const bool isColorRamp = currentItemType() == 3;
  const bool isTextFormat = currentItemType() == 4;
  const bool isLabelSettings = currentItemType() == 5;
  const bool isLegendPatchShape = currentItemType() == 6;
  const bool isSymbol3D = currentItemType() == 7;
  searchBox->setPlaceholderText( isSymbol ? tr( "Filter symbols…" ) :
                                 isColorRamp ? tr( "Filter color ramps…" ) :
                                 isTextFormat ? tr( "Filter text symbols…" ) :
                                 isLabelSettings ? tr( "Filter label settings…" ) :
                                 isLegendPatchShape ? tr( "Filter legend patch shapes…" ) : tr( "Filter 3D symbols…" ) );

  if ( !mReadOnly && isColorRamp ) // color ramp tab
  {
    btnAddItem->setMenu( mMenuBtnAddItemColorRamp );
  }
  else if ( !mReadOnly && isLegendPatchShape ) // legend patch shape tab
  {
    btnAddItem->setMenu( mMenuBtnAddItemLegendPatchShape );
  }
  else if ( !mReadOnly && isSymbol3D ) // legend patch shape tab
  {
    btnAddItem->setMenu( mMenuBtnAddItemSymbol3D );
  }
  else if ( !mReadOnly && isLabelSettings ) // label settings tab
  {
    btnAddItem->setMenu( mMenuBtnAddItemLabelSettings );
  }
  else if ( !mReadOnly && !isSymbol && !isColorRamp ) // text format tab
  {
    btnAddItem->setMenu( nullptr );
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

  mModel->setEntityFilter( isSymbol ? QgsStyle::SymbolEntity : ( isColorRamp ? QgsStyle::ColorrampEntity : isTextFormat ? QgsStyle::TextFormatEntity : isLabelSettings ? QgsStyle::LabelSettingsEntity : isLegendPatchShape ? QgsStyle::LegendPatchShapeEntity : QgsStyle::Symbol3DEntity ) );
  mModel->setEntityFilterEnabled( !allTypesSelected() );
  mModel->setSymbolTypeFilterEnabled( isSymbol && !allTypesSelected() );
  if ( isSymbol && !allTypesSelected() )
    mModel->setSymbolType( static_cast< Qgis::SymbolType >( currentItemType() ) );

  populateList();
}

void QgsStyleManagerDialog::copyItemsToDefault()
{
  const QList< ItemDetails > items = selectedItems();
  if ( !items.empty() )
  {
    bool ok = false;
    QStringList options;
    if ( !mBaseName.isEmpty() )
      options.append( mBaseName );

    QStringList defaultTags = QgsStyle::defaultStyle()->tags();
    defaultTags.sort( Qt::CaseInsensitive );
    options.append( defaultTags );
    const QString tags = QInputDialog::getItem( this, tr( "Import Items" ),
                         tr( "Additional tags to add (comma separated)" ), options, mBaseName.isEmpty() ? -1 : 0, true, &ok );
    if ( !ok )
      return;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QStringList parts = tags.split( ',', QString::SkipEmptyParts );
#else
    const QStringList parts = tags.split( ',', Qt::SkipEmptyParts );
#endif
    QStringList additionalTags;
    additionalTags.reserve( parts.count() );
    for ( const QString &tag : parts )
      additionalTags << tag.trimmed();

    auto cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
    const int count = copyItems( items, mStyle, QgsStyle::defaultStyle(), this, cursorOverride, true, additionalTags, false, false );
    cursorOverride.reset();
    if ( count > 0 )
    {
      mMessageBar->pushSuccess( tr( "Import Items" ), count > 1 ? tr( "Successfully imported %n item(s).", nullptr, count ) : tr( "Successfully imported item." ) );
    }
  }
}

void QgsStyleManagerDialog::copyItem()
{
  const QList< ItemDetails > items = selectedItems();
  if ( items.empty() )
    return;

  ItemDetails details = items.at( 0 );
  switch ( details.entityType )
  {
    case QgsStyle::SymbolEntity:
    {
      std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( details.name ) );
      if ( !symbol )
        return;
      QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::symbolToMimeData( symbol.get() ) );
      break;
    }

    case QgsStyle::TextFormatEntity:
    {
      const QgsTextFormat format( mStyle->textFormat( details.name ) );
      QApplication::clipboard()->setMimeData( format.toMimeData() );
      break;
    }

    case QgsStyle::LabelSettingsEntity:
    {
      const QgsTextFormat format( mStyle->labelSettings( details.name ).format() );
      QApplication::clipboard()->setMimeData( format.toMimeData() );
      break;
    }

    case QgsStyle::ColorrampEntity:
    case QgsStyle::LegendPatchShapeEntity:
    case QgsStyle::Symbol3DEntity:
    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
      return;

  }
}

void QgsStyleManagerDialog::pasteItem()
{
  const QString defaultTag = groupTree->currentIndex().isValid() ? groupTree->currentIndex().data( GroupModelRoles::TagName ).toString() : QString();
  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( tempSymbol )
  {
    QgsStyleSaveDialog saveDlg( this );
    saveDlg.setWindowTitle( tr( "Paste Symbol" ) );
    saveDlg.setDefaultTags( defaultTag );
    if ( !saveDlg.exec() || saveDlg.name().isEmpty() )
      return;

    if ( mStyle->symbolNames().contains( saveDlg.name() ) )
    {
      int res = QMessageBox::warning( this, tr( "Paste Symbol" ),
                                      tr( "A symbol with the name '%1' already exists. Overwrite?" )
                                      .arg( saveDlg.name() ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
      {
        return;
      }
      mStyle->removeSymbol( saveDlg.name() );
    }

    QStringList symbolTags = saveDlg.tags().split( ',' );
    QgsSymbol *newSymbol = tempSymbol.get();
    mStyle->addSymbol( saveDlg.name(), tempSymbol.release() );
    // make sure the symbol is stored
    mStyle->saveSymbol( saveDlg.name(), newSymbol, saveDlg.isFavorite(), symbolTags );
    return;
  }

  bool ok = false;
  const QgsTextFormat format = QgsTextFormat::fromMimeData( QApplication::clipboard()->mimeData(), &ok );
  if ( ok )
  {
    QgsStyleSaveDialog saveDlg( this, QgsStyle::TextFormatEntity );
    saveDlg.setDefaultTags( defaultTag );
    saveDlg.setWindowTitle( tr( "Paste Text Format" ) );
    if ( !saveDlg.exec() || saveDlg.name().isEmpty() )
      return;

    if ( mStyle->textFormatNames().contains( saveDlg.name() ) )
    {
      int res = QMessageBox::warning( this, tr( "Paste Text Format" ),
                                      tr( "A format with the name '%1' already exists. Overwrite?" )
                                      .arg( saveDlg.name() ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
      {
        return;
      }
      mStyle->removeTextFormat( saveDlg.name() );
    }

    QStringList symbolTags = saveDlg.tags().split( ',' );
    mStyle->addTextFormat( saveDlg.name(), format );
    // make sure the foprmatis stored
    mStyle->saveTextFormat( saveDlg.name(), format, saveDlg.isFavorite(), symbolTags );
    return;
  }
}

void QgsStyleManagerDialog::setThumbnailSize( int value )
{
  // value ranges from 0-10
  const double iconSize = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * ( value * 2.5 + 10 );
  // set a grid size which allows sufficient vertical spacing to fit reasonably sized entity names
  const double spacing = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * ( value * 2.2 + 14 );
  const double verticalSpacing = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 7
                                 + iconSize * 0.8;
  listItems->setIconSize( QSize( static_cast< int >( iconSize ), static_cast< int >( iconSize * 0.9 ) ) );
  listItems->setGridSize( QSize( static_cast< int >( spacing ), static_cast< int >( verticalSpacing ) ) );
  mModel->addDesiredIconSize( listItems->iconSize() );

  QgsSettings().setValue( QStringLiteral( "Windows/StyleV2Manager/thumbnailSize" ), value, QgsSettings::Gui );
}

int QgsStyleManagerDialog::selectedItemType()
{
  QModelIndex index = listItems->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return 0;

  const QgsStyle::StyleEntity entity = static_cast< QgsStyle::StyleEntity >( mModel->data( index, QgsStyleModel::TypeRole ).toInt() );
  if ( entity == QgsStyle::ColorrampEntity )
    return 3;
  else if ( entity == QgsStyle::TextFormatEntity )
    return 4;
  else if ( entity == QgsStyle::LabelSettingsEntity )
    return 5;
  else if ( entity == QgsStyle::LegendPatchShapeEntity )
    return 6;
  else if ( entity == QgsStyle::Symbol3DEntity )
    return 7;

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
      details.symbolType = static_cast< Qgis::SymbolType >( mModel->data( index, QgsStyleModel::SymbolTypeRole ).toInt() );
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
  const QStringList favoriteTextFormats = src->symbolsOfFavorite( QgsStyle::TextFormatEntity );
  const QStringList favoriteLabelSettings = src->symbolsOfFavorite( QgsStyle::LabelSettingsEntity );
  const QStringList favoriteLegendPatchShapes = src->symbolsOfFavorite( QgsStyle::LegendPatchShapeEntity );
  const QStringList favorite3dSymbols = src->symbolsOfFavorite( QgsStyle::Symbol3DEntity );

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
          cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
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
          cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
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

      case QgsStyle::TextFormatEntity:
      {
        const QgsTextFormat format( src->textFormat( details.name ) );

        const bool hasDuplicateName = dst->textFormatNames().contains( details.name );
        bool overwriteThis = false;
        if ( isImport )
          addItemToFavorites = favoriteTextFormats.contains( details.name );

        if ( hasDuplicateName && prompt )
        {
          cursorOverride.reset();
          int res = QMessageBox::warning( parentWidget, isImport ? tr( "Import Text Format" ) : tr( "Export Text Format" ),
                                          tr( "A text format with the name “%1” already exists.\nOverwrite?" )
                                          .arg( details.name ),
                                          QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
          cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
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
          dst->addTextFormat( details.name, format );
          dst->saveTextFormat( details.name, format, addItemToFavorites, symbolTags );
          count++;
        }
        break;
      }

      case QgsStyle::LabelSettingsEntity:
      {
        const QgsPalLayerSettings settings( src->labelSettings( details.name ) );

        const bool hasDuplicateName = dst->labelSettingsNames().contains( details.name );
        bool overwriteThis = false;
        if ( isImport )
          addItemToFavorites = favoriteLabelSettings.contains( details.name );

        if ( hasDuplicateName && prompt )
        {
          cursorOverride.reset();
          int res = QMessageBox::warning( parentWidget, isImport ? tr( "Import Label Settings" ) : tr( "Export Label Settings" ),
                                          tr( "Label settings with the name “%1” already exist.\nOverwrite?" )
                                          .arg( details.name ),
                                          QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
          cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
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
          dst->addLabelSettings( details.name, settings );
          dst->saveLabelSettings( details.name, settings, addItemToFavorites, symbolTags );
          count++;
        }
        break;
      }

      case QgsStyle::LegendPatchShapeEntity:
      {
        const QgsLegendPatchShape shape( src->legendPatchShape( details.name ) );

        const bool hasDuplicateName = dst->legendPatchShapeNames().contains( details.name );
        bool overwriteThis = false;
        if ( isImport )
          addItemToFavorites = favoriteLegendPatchShapes.contains( details.name );

        if ( hasDuplicateName && prompt )
        {
          cursorOverride.reset();
          int res = QMessageBox::warning( parentWidget, isImport ? tr( "Import Legend Patch Shape" ) : tr( "Export Legend Patch Shape" ),
                                          tr( "Legend patch shape with the name “%1” already exist.\nOverwrite?" )
                                          .arg( details.name ),
                                          QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
          cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
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
          dst->addLegendPatchShape( details.name, shape );
          dst->saveLegendPatchShape( details.name, shape, addItemToFavorites, symbolTags );
          count++;
        }
        break;
      }

      case QgsStyle::Symbol3DEntity:
      {
        std::unique_ptr< QgsAbstract3DSymbol > symbol( src->symbol3D( details.name ) );
        if ( !symbol )
          continue;

        const bool hasDuplicateName = dst->symbol3DNames().contains( details.name );
        bool overwriteThis = false;
        if ( isImport )
          addItemToFavorites = favorite3dSymbols.contains( details.name );

        if ( hasDuplicateName && prompt )
        {
          cursorOverride.reset();
          int res = QMessageBox::warning( parentWidget, isImport ? tr( "Import 3D Symbol" ) : tr( "Export 3D Symbol" ),
                                          tr( "A 3D symbol with the name “%1” already exists.\nOverwrite?" )
                                          .arg( details.name ),
                                          QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
          cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
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
          QgsAbstract3DSymbol *newSymbol = symbol.get();
          dst->addSymbol3D( details.name, symbol.release() );
          dst->saveSymbol3D( details.name, newSymbol, addItemToFavorites, symbolTags );
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

bool QgsStyleManagerDialog::addTextFormat()
{
  QgsTextFormat format;
  QgsTextFormatDialog formatDlg( format, nullptr, this );
  formatDlg.setWindowTitle( tr( "New Text Format" ) );
  if ( !formatDlg.exec() )
    return false;
  format = formatDlg.format();

  QgsStyleSaveDialog saveDlg( this, QgsStyle::TextFormatEntity );
  const QString defaultTag = groupTree->currentIndex().isValid() ? groupTree->currentIndex().data( GroupModelRoles::TagName ).toString() : QString();
  saveDlg.setDefaultTags( defaultTag );
  if ( !saveDlg.exec() )
    return false;
  QString name = saveDlg.name();

  // request valid/unique name
  bool nameInvalid = true;
  while ( nameInvalid )
  {
    // validate name
    if ( name.isEmpty() )
    {
      QMessageBox::warning( this, tr( "Save Text Format" ),
                            tr( "Cannot save text format without name. Enter a name." ) );
    }
    else if ( mStyle->textFormatNames().contains( name ) )
    {
      int res = QMessageBox::warning( this, tr( "Save Text Format" ),
                                      tr( "Text format with name '%1' already exists. Overwrite?" )
                                      .arg( name ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res == QMessageBox::Yes )
      {
        mStyle->removeTextFormat( name );
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
      name = QInputDialog::getText( this, tr( "Text Format Name" ),
                                    tr( "Please enter a name for new text format:" ),
                                    QLineEdit::Normal, name, &ok );
      if ( !ok )
      {
        return false;
      }
    }
  }

  QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new format to style and re-populate the list
  mStyle->addTextFormat( name, format );
  mStyle->saveTextFormat( name, format, saveDlg.isFavorite(), symbolTags );

  mModified = true;
  return true;
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
      return static_cast< int >( Qgis::SymbolType::Marker );
    case 2:
      return static_cast< int >( Qgis::SymbolType::Line );
    case 3:
      return static_cast< int >( Qgis::SymbolType::Fill );
    case 4:
      return 3;
    case 5:
      return 4;
    case 6:
      return 5;
    case 7:
      return 6;
    case 8:
      return 7;
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
  else if ( currentItemType() == 4 )
  {
    changed = addTextFormat();
  }
  else if ( currentItemType() == 5 )
  {
    // actually never hit, because we present a submenu when adding label settings
    // changed = addLabelSettings();
  }
  else if ( currentItemType() == 6 )
  {
    // actually never hit, because we present a submenu when adding legend patches
    // changed = addLegendPatchShape();
  }
  else if ( currentItemType() == 7 )
  {
    // actually never hit, because we present a submenu when adding 3d symbols
    // changed = addSymbol3D();
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
  QString dialogTitle;
  switch ( symbolType == -1 ? currentItemType() : symbolType )
  {
    case static_cast< int >( Qgis::SymbolType::Marker ):
      symbol = new QgsMarkerSymbol();
      name = tr( "new marker" );
      dialogTitle = tr( "New Marker Symbol" );
      break;
    case static_cast< int>( Qgis::SymbolType::Line ):
      symbol = new QgsLineSymbol();
      name = tr( "new line" );
      dialogTitle = tr( "New Line Symbol" );
      break;
    case static_cast< int >( Qgis::SymbolType::Fill ):
      symbol = new QgsFillSymbol();
      name = tr( "new fill symbol" );
      dialogTitle = tr( "New Fill Symbol" );
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
  dlg.setWindowTitle( dialogTitle );
  if ( dlg.exec() == 0 )
  {
    delete symbol;
    return false;
  }

  QgsStyleSaveDialog saveDlg( this );
  const QString defaultTag = groupTree->currentIndex().isValid() ? groupTree->currentIndex().data( GroupModelRoles::TagName ).toString() : QString();
  saveDlg.setDefaultTags( defaultTag );
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


QString QgsStyleManagerDialog::addColorRampStatic( QWidget *parent, QgsStyle *style, const QString &type )
{
  QString rampType = type;

  if ( rampType.isEmpty() )
  {
    // let the user choose the color ramp type if rampType is not given
    bool ok = true;
    const QList< QPair< QString, QString > > rampTypes = QgsColorRamp::rampTypes();
    QStringList rampTypeNames;
    rampTypeNames.reserve( rampTypes.size() );
    for ( const QPair< QString, QString > &type : rampTypes )
      rampTypeNames << type.second;
    const QString selectedRampTypeName = QInputDialog::getItem( parent, tr( "Color Ramp Type" ),
                                         tr( "Please select color ramp type:" ), rampTypeNames, 0, false, &ok );
    if ( !ok || selectedRampTypeName.isEmpty() )
      return QString();

    rampType = rampTypes.value( rampTypeNames.indexOf( selectedRampTypeName ) ).first;
  }

  QString name = tr( "new ramp" );

  std::unique_ptr< QgsColorRamp  > ramp;
  if ( rampType == QgsGradientColorRamp::typeString() )
  {
    QgsGradientColorRampDialog dlg( QgsGradientColorRamp(), parent );
    dlg.setWindowTitle( tr( "New Gradient Color Ramp" ) );
    if ( !dlg.exec() )
    {
      return QString();
    }
    ramp.reset( dlg.ramp().clone() );
    name = tr( "new gradient ramp" );
  }
  else if ( rampType == QgsLimitedRandomColorRamp::typeString() )
  {
    QgsLimitedRandomColorRampDialog dlg( QgsLimitedRandomColorRamp(), parent );
    dlg.setWindowTitle( tr( "New Random Color Ramp" ) );
    if ( !dlg.exec() )
    {
      return QString();
    }
    ramp.reset( dlg.ramp().clone() );
    name = tr( "new random ramp" );
  }
  else if ( rampType == QgsColorBrewerColorRamp::typeString() )
  {
    QgsColorBrewerColorRampDialog dlg( QgsColorBrewerColorRamp(), parent );
    dlg.setWindowTitle( tr( "New ColorBrewer Ramp" ) );
    if ( !dlg.exec() )
    {
      return QString();
    }
    ramp.reset( dlg.ramp().clone() );
    name = dlg.ramp().schemeName() + QString::number( dlg.ramp().colors() );
  }
  else if ( rampType == QgsPresetSchemeColorRamp::typeString() )
  {
    QgsPresetColorRampDialog dlg( QgsPresetSchemeColorRamp(), parent );
    dlg.setWindowTitle( tr( "New Preset Color Ramp" ) );
    if ( !dlg.exec() )
    {
      return QString();
    }
    ramp.reset( dlg.ramp().clone() );
    name = tr( "new preset ramp" );
  }
  else if ( rampType == QgsCptCityColorRamp::typeString() )
  {
    QgsCptCityColorRampDialog dlg( QgsCptCityColorRamp( QString(), QString() ), parent );
    dlg.setWindowTitle( tr( "New cpt-city Color Ramp" ) );
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

bool QgsStyleManagerDialog::addColorRamp( const QString &type )
{
  // pass the action text, which is the color ramp type
  QString rampName = addColorRampStatic( this, mStyle, type );
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
  else if ( selectedItemType() == 4 )
  {
    editTextFormat();
  }
  else if ( selectedItemType() == 5 )
  {
    editLabelSettings();
  }
  else if ( selectedItemType() == 6 )
  {
    editLegendPatchShape();
  }
  else if ( selectedItemType() == 7 )
  {
    editSymbol3D();
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
  dlg.setWindowTitle( symbolName );
  if ( mReadOnly )
    dlg.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

  if ( !dlg.exec() )
    return false;

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

  if ( ramp->type() == QgsGradientColorRamp::typeString() )
  {
    QgsGradientColorRamp *gradRamp = static_cast<QgsGradientColorRamp *>( ramp.get() );
    QgsGradientColorRampDialog dlg( *gradRamp, this );
    dlg.setWindowTitle( name );
    if ( mReadOnly )
      dlg.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

    if ( !dlg.exec() )
    {
      return false;
    }
    ramp.reset( dlg.ramp().clone() );
  }
  else if ( ramp->type() == QgsLimitedRandomColorRamp::typeString() )
  {
    QgsLimitedRandomColorRamp *randRamp = static_cast<QgsLimitedRandomColorRamp *>( ramp.get() );
    QgsLimitedRandomColorRampDialog dlg( *randRamp, this );
    dlg.setWindowTitle( name );
    if ( mReadOnly )
      dlg.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

    if ( !dlg.exec() )
    {
      return false;
    }
    ramp.reset( dlg.ramp().clone() );
  }
  else if ( ramp->type() == QgsColorBrewerColorRamp::typeString() )
  {
    QgsColorBrewerColorRamp *brewerRamp = static_cast<QgsColorBrewerColorRamp *>( ramp.get() );
    QgsColorBrewerColorRampDialog dlg( *brewerRamp, this );
    dlg.setWindowTitle( name );
    if ( mReadOnly )
      dlg.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

    if ( !dlg.exec() )
    {
      return false;
    }
    ramp.reset( dlg.ramp().clone() );
  }
  else if ( ramp->type() == QgsPresetSchemeColorRamp::typeString() )
  {
    QgsPresetSchemeColorRamp *presetRamp = static_cast<QgsPresetSchemeColorRamp *>( ramp.get() );
    QgsPresetColorRampDialog dlg( *presetRamp, this );
    dlg.setWindowTitle( name );
    if ( mReadOnly )
      dlg.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

    if ( !dlg.exec() )
    {
      return false;
    }
    ramp.reset( dlg.ramp().clone() );
  }
  else if ( ramp->type() == QgsCptCityColorRamp::typeString() )
  {
    QgsCptCityColorRamp *cptCityRamp = static_cast<QgsCptCityColorRamp *>( ramp.get() );
    QgsCptCityColorRampDialog dlg( *cptCityRamp, this );
    dlg.setWindowTitle( name );
    if ( mReadOnly )
      dlg.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

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

bool QgsStyleManagerDialog::editTextFormat()
{
  const QString formatName = currentItemName();
  if ( formatName.isEmpty() )
    return false;

  QgsTextFormat format = mStyle->textFormat( formatName );

  // let the user edit the format and update list when done
  QgsTextFormatDialog dlg( format, nullptr, this );
  dlg.setWindowTitle( formatName );
  if ( mReadOnly )
    dlg.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

  if ( !dlg.exec() )
    return false;

  // by adding format to style with the same name the old effectively gets overwritten
  mStyle->addTextFormat( formatName, dlg.format(), true );
  mModified = true;
  return true;
}

bool QgsStyleManagerDialog::addLabelSettings( QgsWkbTypes::GeometryType type )
{
  QgsPalLayerSettings settings;
  QgsLabelSettingsDialog settingsDlg( settings, nullptr, nullptr, this, type );
  settingsDlg.setWindowTitle( tr( "New Label Settings" ) );
  if ( mReadOnly )
    settingsDlg.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

  if ( !settingsDlg.exec() )
    return false;

  settings = settingsDlg.settings();
  settings.layerType = type;

  QgsStyleSaveDialog saveDlg( this, QgsStyle::LabelSettingsEntity );
  const QString defaultTag = groupTree->currentIndex().isValid() ? groupTree->currentIndex().data( GroupModelRoles::TagName ).toString() : QString();
  saveDlg.setDefaultTags( defaultTag );
  if ( !saveDlg.exec() )
    return false;
  QString name = saveDlg.name();

  // request valid/unique name
  bool nameInvalid = true;
  while ( nameInvalid )
  {
    // validate name
    if ( name.isEmpty() )
    {
      QMessageBox::warning( this, tr( "Save Label Settings" ),
                            tr( "Cannot save label settings without a name. Enter a name." ) );
    }
    else if ( mStyle->labelSettingsNames().contains( name ) )
    {
      int res = QMessageBox::warning( this, tr( "Save Label Settings" ),
                                      tr( "Label settings with the name '%1' already exist. Overwrite?" )
                                      .arg( name ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res == QMessageBox::Yes )
      {
        mStyle->removeLabelSettings( name );
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
      name = QInputDialog::getText( this, tr( "Label Settings Name" ),
                                    tr( "Please enter a name for the new label settings:" ),
                                    QLineEdit::Normal, name, &ok );
      if ( !ok )
      {
        return false;
      }
    }
  }

  QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new format to style and re-populate the list
  mStyle->addLabelSettings( name, settings );
  mStyle->saveLabelSettings( name, settings, saveDlg.isFavorite(), symbolTags );

  mModified = true;
  return true;
}

bool QgsStyleManagerDialog::editLabelSettings()
{
  const QString formatName = currentItemName();
  if ( formatName.isEmpty() )
    return false;

  QgsPalLayerSettings settings = mStyle->labelSettings( formatName );
  QgsWkbTypes::GeometryType geomType = settings.layerType;

  // let the user edit the settings and update list when done
  QgsLabelSettingsDialog dlg( settings, nullptr, nullptr, this, geomType );
  dlg.setWindowTitle( formatName );
  if ( !dlg.exec() )
    return false;

  settings = dlg.settings();
  settings.layerType = geomType;

  // by adding format to style with the same name the old effectively gets overwritten
  mStyle->addLabelSettings( formatName, settings, true );
  mModified = true;
  return true;
}

bool QgsStyleManagerDialog::addLegendPatchShape( Qgis::SymbolType type )
{
  QgsLegendPatchShape shape = mStyle->defaultPatch( type, QSizeF( 10, 5 ) );
  QgsLegendPatchShapeDialog dialog( shape, this );
  dialog.setWindowTitle( tr( "New Legend Patch Shape" ) );
  if ( mReadOnly )
    dialog.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

  if ( !dialog.exec() )
    return false;

  shape = dialog.shape();

  QgsStyleSaveDialog saveDlg( this, QgsStyle::LegendPatchShapeEntity );
  const QString defaultTag = groupTree->currentIndex().isValid() ? groupTree->currentIndex().data( GroupModelRoles::TagName ).toString() : QString();
  saveDlg.setDefaultTags( defaultTag );
  if ( !saveDlg.exec() )
    return false;
  QString name = saveDlg.name();

  // request valid/unique name
  bool nameInvalid = true;
  while ( nameInvalid )
  {
    // validate name
    if ( name.isEmpty() )
    {
      QMessageBox::warning( this, tr( "Save Legend Patch Shape" ),
                            tr( "Cannot save legend patch shapes without a name. Enter a name." ) );
    }
    else if ( mStyle->legendPatchShapeNames().contains( name ) )
    {
      int res = QMessageBox::warning( this, tr( "Save Legend Patch Shape" ),
                                      tr( "A legend patch shape with the name '%1' already exists. Overwrite?" )
                                      .arg( name ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res == QMessageBox::Yes )
      {
        mStyle->removeEntityByName( QgsStyle::LegendPatchShapeEntity, name );
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
      name = QInputDialog::getText( this, tr( "Legend Patch Shape Name" ),
                                    tr( "Please enter a name for the new legend patch shape:" ),
                                    QLineEdit::Normal, name, &ok );
      if ( !ok )
      {
        return false;
      }
    }
  }

  QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new shape to style and re-populate the list
  mStyle->addLegendPatchShape( name, shape );
  mStyle->saveLegendPatchShape( name, shape, saveDlg.isFavorite(), symbolTags );

  mModified = true;
  return true;
}

bool QgsStyleManagerDialog::editLegendPatchShape()
{
  const QString shapeName = currentItemName();
  if ( shapeName.isEmpty() )
    return false;

  QgsLegendPatchShape shape = mStyle->legendPatchShape( shapeName );
  if ( shape.isNull() )
    return false;

  // let the user edit the shape and update list when done
  QgsLegendPatchShapeDialog dlg( shape, this );
  dlg.setWindowTitle( shapeName );
  if ( !dlg.exec() )
    return false;

  shape = dlg.shape();

  // by adding shape to style with the same name the old effectively gets overwritten
  mStyle->addLegendPatchShape( shapeName, shape, true );
  mModified = true;
  return true;
}

bool QgsStyleManagerDialog::addSymbol3D( const QString &type )
{
  std::unique_ptr< QgsAbstract3DSymbol > symbol( QgsApplication::symbol3DRegistry()->createSymbol( type ) );
  if ( !symbol )
    return false;

  Qgs3DSymbolDialog dialog( symbol.get(), this );
  dialog.setWindowTitle( tr( "New 3D Symbol" ) );
  if ( mReadOnly )
    dialog.buttonBox()->button( QDialogButtonBox::Ok )->setEnabled( false );

  if ( !dialog.exec() )
    return false;

  symbol.reset( dialog.symbol() );
  if ( !symbol )
    return false;

  QgsStyleSaveDialog saveDlg( this, QgsStyle::Symbol3DEntity );
  const QString defaultTag = groupTree->currentIndex().isValid() ? groupTree->currentIndex().data( GroupModelRoles::TagName ).toString() : QString();
  saveDlg.setDefaultTags( defaultTag );
  if ( !saveDlg.exec() )
    return false;
  QString name = saveDlg.name();

  // request valid/unique name
  bool nameInvalid = true;
  while ( nameInvalid )
  {
    // validate name
    if ( name.isEmpty() )
    {
      QMessageBox::warning( this, tr( "Save 3D Symbol" ),
                            tr( "Cannot save 3D symbols without a name. Enter a name." ) );
    }
    else if ( mStyle->symbol3DNames().contains( name ) )
    {
      int res = QMessageBox::warning( this, tr( "Save 3D Symbol" ),
                                      tr( "A 3D symbol with the name '%1' already exists. Overwrite?" )
                                      .arg( name ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res == QMessageBox::Yes )
      {
        mStyle->removeEntityByName( QgsStyle::Symbol3DEntity, name );
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
      name = QInputDialog::getText( this, tr( "3D Symbol Name" ),
                                    tr( "Please enter a name for the new 3D symbol:" ),
                                    QLineEdit::Normal, name, &ok );
      if ( !ok )
      {
        return false;
      }
    }
  }

  QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new shape to style and re-populate the list
  QgsAbstract3DSymbol *newSymbol = symbol.get();
  mStyle->addSymbol3D( name, symbol.release() );
  mStyle->saveSymbol3D( name, newSymbol, saveDlg.isFavorite(), symbolTags );

  mModified = true;
  return true;
}

bool QgsStyleManagerDialog::editSymbol3D()
{
  const QString symbolName = currentItemName();
  if ( symbolName.isEmpty() )
    return false;

  std::unique_ptr< QgsAbstract3DSymbol > symbol( mStyle->symbol3D( symbolName ) );
  if ( !symbol )
    return false;

  // let the user edit the symbol and update list when done
  Qgs3DSymbolDialog dlg( symbol.get(), this );
  dlg.setWindowTitle( symbolName );
  if ( !dlg.exec() )
    return false;

  symbol.reset( dlg.symbol() );
  if ( !symbol )
    return false;

  // by adding symbol to style with the same name the old effectively gets overwritten
  mStyle->addSymbol3D( symbolName, symbol.release(), true );
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
    else if ( currentItemType() == 3 )
    {
      if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Color Ramp" ),
           QString( tr( "Do you really want to remove %n ramp(s)?", nullptr, items.count() ) ),
           QMessageBox::Yes,
           QMessageBox::No ) )
        return;
    }
    else if ( currentItemType() == 4 )
    {
      if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Text Formats" ),
           QString( tr( "Do you really want to remove %n text format(s)?", nullptr, items.count() ) ),
           QMessageBox::Yes,
           QMessageBox::No ) )
        return;
    }
    else if ( currentItemType() == 5 )
    {
      if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Label Settings" ),
           QString( tr( "Do you really want to remove %n label setting(s)?", nullptr, items.count() ) ),
           QMessageBox::Yes,
           QMessageBox::No ) )
        return;
    }
    else if ( currentItemType() == 6 )
    {
      if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove Legend Patch Shapes" ),
           QString( tr( "Do you really want to remove %n legend patch shape(s)?", nullptr, items.count() ) ),
           QMessageBox::Yes,
           QMessageBox::No ) )
        return;
    }
    else if ( currentItemType() == 7 )
    {
      if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Remove 3D Symbols" ),
           QString( tr( "Do you really want to remove %n 3D symbol(s)?", nullptr, items.count() ) ),
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

    mStyle->removeEntityByName( details.entityType, details.name );
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
  for ( const QString &tag : std::as_const( tags ) )
  {
    QStandardItem *item = new QStandardItem( tag );
    item->setData( mStyle->tagId( tag ) );
    item->setData( tag, GroupModelRoles::TagName );
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
  sPreviousTag = category;

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
    mMessageBar->pushCritical( tr( "Add Tag" ), tr( "The tag “%1” already exists." ).arg( itemName ) );
    return 0;
  }

  // block the auto-repopulation of groups when the style emits groupsModified
  // instead, we manually update the model items for better state retention
  mBlockGroupUpdates++;
  id = mStyle->addTag( itemName );
  mBlockGroupUpdates--;

  if ( !id )
  {
    mMessageBar->pushCritical( tr( "Add Tag" ),  tr( "New tag could not be created — There was a problem with the symbol database." ) );
    return 0;
  }

  QStandardItem *parentItem = model->itemFromIndex( index );
  QStandardItem *childItem = new QStandardItem( itemName );
  childItem->setData( id );
  childItem->setData( itemName, GroupModelRoles::TagName );
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
    // should never appear -- blocked by GUI
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
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
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
  mActionCopyItem->setEnabled( enable );
  mActionPasteItem->setEnabled( enable );
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

  const QModelIndexList indices = listItems->selectionModel()->selectedRows();

  if ( !mReadOnly )
  {
    const QStringList currentTags = indices.count() == 1 ? indices.at( 0 ).data( QgsStyleModel::TagRole ).toStringList() : QStringList();
    QAction *a = nullptr;
    QStringList tags = mStyle->tags();
    tags.sort();
    for ( const QString &tag : std::as_const( tags ) )
    {
      a = new QAction( tag, mGroupListMenu );
      a->setData( tag );
      if ( indices.count() == 1 )
      {
        a->setCheckable( true );
        a->setChecked( currentTags.contains( tag ) );
      }
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

  const QList< ItemDetails > items = selectedItems();
  mActionCopyItem->setEnabled( !items.isEmpty() && ( items.at( 0 ).entityType != QgsStyle::ColorrampEntity ) );

  bool enablePaste = false;
  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( tempSymbol )
    enablePaste = true;
  else
  {
    ( void )QgsTextFormat::fromMimeData( QApplication::clipboard()->mimeData(), &enablePaste );
  }
  mActionPasteItem->setEnabled( enablePaste );

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
    // should never appear - blocked by GUI logic
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
    mMessageBar->pushCritical( tr( "Edit Smart Group" ), tr( "There was an error while editing the smart group." ) );
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
  QgsHelp::openHelp( QStringLiteral( "style_library/style_manager.html" ) );
}

