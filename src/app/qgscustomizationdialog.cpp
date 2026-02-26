/***************************************************************************
    qgscustomizationdialog.cpp
    ---------------------
    begin                : 2025/12/16
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscustomizationdialog.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscustomization.h"
#include "qgsfileutils.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgssettings.h"

#include <QAbstractItemModel>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QString>
#include <QToolButton>
#include <QWidgetAction>
#include <qnamespace.h>

#include "moc_qgscustomizationdialog.cpp"

using namespace Qt::StringLiterals;

#ifdef QGIS_DEBUG
#include <QAbstractItemModelTester>
#endif

constexpr int TOOLBAR_ROW = 4;

const QgsSettingsEntryString *QgsCustomizationDialog::sSettingLastSaveDir = new QgsSettingsEntryString( u"last-save-directory"_s, sTreeCustomization, QDir::homePath(), u"Last directory used when saving a customization XML file"_s );

#define ACTIONPATHS_MIMEDATA_NAME "application/qgis.customization.actionpaths"

QgsCustomizationDialog::QgsCustomizationModel::QgsCustomizationModel( QgisApp *qgisApp, Mode mode, QObject *parent )
  : QAbstractItemModel( parent )
  , mMode( mode )
  , mQgisApp( qgisApp )
{
  init();
}

QVariant QgsCustomizationDialog::QgsCustomizationModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || ( role != Qt::DisplayRole && role != Qt::DecorationRole && role != Qt::CheckStateRole ) )
    return {};

  QgsCustomization::QgsItem *item = static_cast<QgsCustomization::QgsItem *>( index.internalPointer() );
  switch ( role )
  {
    case Qt::DecorationRole:
      return item && index.column() == 0 ? item->icon() : QVariant();

    case Qt::DisplayRole:
      if ( item )
      {
        if ( parent( index ).isValid() )
          return index.column() == 0 ? QVariant( item->name() ) : QVariant( item->title() );
        // root items, we display only title
        else
          return index.column() == 0 ? QVariant( item->title() ) : QVariant();
      }
      else
        return QVariant {};

    case Qt::CheckStateRole:
      return mMode == Mode::ItemVisibility && item && index.parent().isValid() && index.column() == 0 ? ( item->isVisible() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked ) : QVariant();

    default:
      break;
  }

  return QVariant();
}

bool QgsCustomizationDialog::QgsCustomizationModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || ( role != Qt::ItemDataRole::CheckStateRole && role != Qt::ItemDataRole::EditRole ) )
    return false;

  QgsCustomization::QgsItem *item = static_cast<QgsCustomization::QgsItem *>( index.internalPointer() );
  if ( !item )
    return false;

  switch ( role )
  {
    case Qt::ItemDataRole::CheckStateRole:
      item->setVisible( value.value<Qt::CheckState>() == Qt::CheckState::Checked );
      emit dataChanged( index, index, QList<int>() << Qt::CheckStateRole );
      return true;
      break;

    case Qt::ItemDataRole::EditRole:
      if ( index.column() == 1 )
      {
        item->setTitle( value.toString() );
        emit dataChanged( index, index, QList<int>() << Qt::EditRole );
        return true;
      }
      break;

    default:
      break;
  }

  return false;
}

Qt::ItemFlags QgsCustomizationDialog::QgsCustomizationModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  QgsCustomization::QgsItem *item = index.parent().isValid() ? static_cast<QgsCustomization::QgsItem *>( index.internalPointer() ) : nullptr;
  switch ( mMode )
  {
    case Mode::ActionSelector:
      if ( item && item->hasCapability( QgsCustomization::QgsItem::ItemCapability::Drag ) )
        flags |= Qt::ItemIsDragEnabled;
      break;

    case Mode::ItemVisibility:
      // only root item (which have an invalid parent) are checkable
      if ( index.parent().isValid() && index.column() == 0 )
        flags |= Qt::ItemIsUserCheckable;

      if ( item && item->hasCapability( QgsCustomization::QgsItem::ItemCapability::Rename ) && index.column() == 1 )
        flags |= Qt::ItemIsEditable;

      flags |= Qt::ItemIsDropEnabled;
  }
  return flags;
}

QVariant QgsCustomizationDialog::QgsCustomizationModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  return role == Qt::DisplayRole && orientation == Qt::Horizontal ? ( section == 0 ? tr( "Object name" ) : tr( "Label" ) )
                                                                  : QVariant {};
}

QModelIndex QgsCustomizationDialog::QgsCustomizationModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return {};

  // first level, categories title ("Menus" for instance)
  if ( !parent.isValid() )
  {
    return createIndex( row, column, row >= 0 && row < mRootItems.count() ? mRootItems.at( row ) : nullptr );
  }

  QgsCustomization::QgsItem *item = static_cast<QgsCustomization::QgsItem *>( parent.internalPointer() );
  return createIndex( row, column, item->getChild( row ) );
}

QModelIndex QgsCustomizationDialog::QgsCustomizationModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return {};

  QgsCustomization::QgsItem *item = static_cast<QgsCustomization::QgsItem *>( index.internalPointer() );
  QgsCustomization::QgsItem *parentItem = item ? item->parent() : nullptr;
  if ( !item || !parentItem )
    return {};

  QgsCustomization::QgsItem *grandParentItem = parentItem->parent();
  if ( grandParentItem )
  {
    return createIndex( grandParentItem->indexOf( parentItem ), 0, parentItem );
  }

  QList<QgsCustomization::QgsItem *>::const_iterator it = std::find( mRootItems.cbegin(), mRootItems.cend(), parentItem );
  if ( it != mRootItems.cend() )
    return createIndex( static_cast<int>( std::distance( mRootItems.cbegin(), it ) ), 0, *it );

  Q_ASSERT( false );

  return {}; // should never happen
}

int QgsCustomizationDialog::QgsCustomizationModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.column() > 0 )
    return 0;

  // first level, categories title ("Menus" for instance)
  if ( !parent.isValid() )
  {
    return static_cast<int>( mRootItems.count() );
  }

  // second level, categories content (menus item for instance)
  QgsCustomization::QgsItem *item = static_cast<QgsCustomization::QgsItem *>( parent.internalPointer() );
  return item ? static_cast<int>( item->childrenCount() ) : 0;
}

int QgsCustomizationDialog::QgsCustomizationModel::columnCount( const QModelIndex & ) const
{
  return 2;
}

void QgsCustomizationDialog::QgsCustomizationModel::init()
{
  mCustomization = std::make_unique<QgsCustomization>( *mQgisApp->customization() );
  mRootItems.clear();
  switch ( mMode )
  {
    case Mode::ActionSelector:
      mRootItems << mCustomization->menusItem()
                 << mCustomization->toolBarsItem();
      break;

    case Mode::ItemVisibility:
      // If you change this, don't forget to update TOOLBAR_COLUMN value
      mRootItems << mCustomization->browserElementsItem()
                 << mCustomization->docksItem()
                 << mCustomization->menusItem()
                 << mCustomization->statusBarWidgetsItem()
                 << mCustomization->toolBarsItem();
      break;
  }
}

void QgsCustomizationDialog::QgsCustomizationModel::reset()
{
  // restore application state
  beginResetModel();
  init();
  endResetModel();
}

void QgsCustomizationDialog::QgsCustomizationModel::apply()
{
  mQgisApp->setCustomization( std::make_unique<QgsCustomization>( *mCustomization ) );

  const QString error = mCustomization->write();
  if ( !error.isEmpty() )
  {
    QMessageBox::warning( mQgisApp, tr( "Error writing customization XML file" ), error );
  }
}

QString QgsCustomizationDialog::QgsCustomizationModel::readFile( const QString &filePath )
{
  beginResetModel();
  QString error = mCustomization->readFile( filePath );
  endResetModel();

  return error;
}

const std::unique_ptr<QgsCustomization> &QgsCustomizationDialog::QgsCustomizationModel::customization() const
{
  return mCustomization;
}

QModelIndex QgsCustomizationDialog::QgsCustomizationModel::addUserItem( const QModelIndex &parent )
{
  QgsCustomization::QgsItem *item = parent.isValid() ? static_cast<QgsCustomization::QgsItem *>( parent.internalPointer() ) : nullptr;

  if ( !item || ( !item->hasCapability( QgsCustomization::QgsItem::ItemCapability::AddUserMenuChild ) && !item->hasCapability( QgsCustomization::QgsItem::ItemCapability::AddUserToolBarChild ) ) )
    return {};

  const int nbChildren = static_cast<int>( item->childItemList().size() );
  beginInsertRows( parent, nbChildren, nbChildren );

  if ( item->hasCapability( QgsCustomization::QgsItem::ItemCapability::AddUserMenuChild ) )
  {
    const QString name = mCustomization->uniqueMenuName();
    item->addChild( std::make_unique<QgsCustomization::QgsUserMenuItem>( name, name, item ) );
  }
  else if ( item->hasCapability( QgsCustomization::QgsItem::ItemCapability::AddUserToolBarChild ) )
  {
    const QString name = mCustomization->uniqueToolBarName();
    item->addChild( std::make_unique<QgsCustomization::QgsUserToolBarItem>( name, name, item ) );
  }
  else
  {
    QgsDebugError( "Cannot add child to this item" );
    return {};
  }

  endInsertRows();

  return index( nbChildren, 0, parent );
}

void QgsCustomizationDialog::QgsCustomizationModel::deleteUserItems( const QModelIndexList &indexes )
{
  QHash<QgsCustomization::QgsItem *, QModelIndex> toDelete;
  bool allAcceptDelete = false;
  for ( const QModelIndex &index : indexes )
  {
    QgsCustomization::QgsItem *item = index.isValid() ? static_cast<QgsCustomization::QgsItem *>( index.internalPointer() ) : nullptr;
    allAcceptDelete = item && item->hasCapability( QgsCustomization::QgsItem::ItemCapability::Delete );
    if ( !allAcceptDelete )
      break;

    toDelete.insert( item, index );
  }

  // Shall not happen, but better to check anyway
  if ( !allAcceptDelete )
    return;

  // We need to remove all items whose parent is already in deleted list
  std::function<bool( QgsCustomization::QgsItem * )> alreadyDeleted = [&alreadyDeleted, &toDelete]( QgsCustomization::QgsItem *item ) {
    return item && ( toDelete.contains( item->parent() ) || alreadyDeleted( item->parent() ) );
  };

  QHash<QgsCustomization::QgsItem *, QModelIndex>::iterator it = toDelete.begin();
  while ( it != toDelete.end() )
  {
    if ( alreadyDeleted( it.key() ) )
    {
      it = toDelete.erase( it );
    }
    else
    {
      ++it;
    }
  }

  for ( QHash<QgsCustomization::QgsItem *, QModelIndex>::const_iterator it = toDelete.cbegin(); it != toDelete.cend(); ++it )
  {
    QgsCustomization::QgsItem *item = it.key();
    const int index = static_cast<int>( item->parent() ? item->parent()->indexOf( item ) : -1 );
    if ( index < 0 )
    {
      QgsDebugError( "Impossible to find item among parent's children" );
      continue;
    }

    beginRemoveRows( parent( it.value() ), index, index );
    item->parent()->deleteChild( index );
    endRemoveRows();
  }
}

QMimeData *QgsCustomizationDialog::QgsCustomizationModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();


  QStringList strActionPaths;
  QSet<int> rows;
  for ( const QModelIndex &index : indexes )
  {
    // there could be several indexes for each column representing the same row
    if ( rows.contains( index.row() ) )
      continue;

    rows << index.row();

    QgsCustomization::QgsItem *item = index.isValid() ? static_cast<QgsCustomization::QgsItem *>( index.internalPointer() ) : nullptr;
    if ( QgsCustomization::QgsActionItem *action = dynamic_cast<QgsCustomization::QgsActionItem *>( item ) )
      strActionPaths << action->path();
  }

  QByteArray actionPaths;
  QDataStream dataStreamWrite( &actionPaths, QIODevice::WriteOnly );
  dataStreamWrite << strActionPaths;

  mimeData->setData( QStringLiteral( ACTIONPATHS_MIMEDATA_NAME ), actionPaths );

  return mimeData;
}

bool QgsCustomizationDialog::QgsCustomizationModel::canDropMimeData( const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex & ) const
{
  // QgsCustomization::Item *item = parent.isValid() ? static_cast<QgsCustomization::Item *>( parent.internalPointer() ) : nullptr;
  return ( action == Qt::DropAction::LinkAction || action == Qt::DropAction::MoveAction || action == Qt::DropAction::CopyAction )
         // TODO Qt issue https://qt-project.atlassian.net/browse/QTBUG-76418?focusedCommentId=465643
         // canDropMimeData() doesn't work if the result value differs from one index to another, specially
         // when we start with a cannot-drop-item after we start dragging
         // Try to see if we can workaround thin in dragEnterEvent
         // uncomment the following lines when fixed
         /* && item && item->hasCapability( QgsCustomization::Item::ItemCapability::UserMenuChild ) */
         && data && data->hasFormat( QStringLiteral( ACTIONPATHS_MIMEDATA_NAME ) );
}

bool QgsCustomizationDialog::QgsCustomizationModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int, const QModelIndex &parent )
{
  if ( action == Qt::IgnoreAction )
    return true;

  if ( row == -1 )
    row = rowCount( parent ); // if dropped directly onto group item, insert at last position

  QgsCustomization::QgsItem *item = parent.isValid() ? static_cast<QgsCustomization::QgsItem *>( parent.internalPointer() ) : nullptr;
  if ( !item || !item->hasCapability( QgsCustomization::QgsItem::ItemCapability::AddActionRefChild ) || !data || !data->hasFormat( QStringLiteral( ACTIONPATHS_MIMEDATA_NAME ) ) )
    return false;

  QDataStream dataStreamRead( data->data( QStringLiteral( ACTIONPATHS_MIMEDATA_NAME ) ) );
  QStringList actionPaths;
  dataStreamRead >> actionPaths;

  QList<QPair<QgsCustomization::QgsActionItem *, QString>> actions; // name, path
  for ( QString actionPath : actionPaths )
  {
    QgsCustomization::QgsActionItem *action = mCustomization->getItem<QgsCustomization::QgsActionItem>( actionPath );
    if ( !action )
    {
      QgsDebugError( u"Invalid action path '%1'"_s.arg( actionPath ) );
      continue;
    }

    actions << QPair<QgsCustomization::QgsActionItem *, QString> { action, actionPath };
  }

  if ( actions.isEmpty() )
    return false;

  beginInsertRows( parent, row, row + static_cast<int>( actions.count() ) - 1 );
  for ( QPair<QgsCustomization::QgsActionItem *, QString> actionAndPath : actions )
  {
    QgsCustomization::QgsActionItem *action = actionAndPath.first;
    auto actionRef = std::make_unique<QgsCustomization::QgsActionRefItem>( mCustomization->uniqueActionName( action->name() ), action->title(), actionAndPath.second, item );
    actionRef->setVisible( action->isVisible() );
    item->insertChild( row, std::move( actionRef ) );
  }

  endInsertRows();

  return true;
}

////////////////

QgsCustomizationDialog::QgsCustomizationDialog( QgisApp *qgisApp )
#ifdef Q_OS_MACOS
  : QMainWindow( qgisApp, Qt::WindowSystemMenuHint ) // Modeless dialog with close button only
#else
  : QMainWindow( qgisApp )
#endif
  , mQgisApp( qgisApp )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( actionSave, &QAction::triggered, this, &QgsCustomizationDialog::onSaveFile );
  connect( actionLoad, &QAction::triggered, this, &QgsCustomizationDialog::onLoadFile );
  connect( actionExpandAll, &QAction::triggered, this, &QgsCustomizationDialog::onExpandAll );
  connect( actionCollapseAll, &QAction::triggered, this, &QgsCustomizationDialog::onCollapseAll );
  connect( actionSelectAll, &QAction::triggered, this, &QgsCustomizationDialog::onSelectAll );
  connect( mCustomizationEnabledCheckBox, &QCheckBox::toggled, this, &QgsCustomizationDialog::enableCustomization );
  connect( actionCatch, &QAction::triggered, this, &QgsCustomizationDialog::onActionCatchToggled );
  connect( mAddAction, &QAction::triggered, this, &QgsCustomizationDialog::addUserItem );
  connect( mDeleteAction, &QAction::triggered, this, &QgsCustomizationDialog::deleteSelectedItems );
  connect( mListActionsAction, &QAction::triggered, this, &QgsCustomizationDialog::updateSplitterSizes );

  connect( buttonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::ok );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::apply );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::cancel );
  connect( buttonBox->button( QDialogButtonBox::Reset ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::reset );
  connect( buttonBox->button( QDialogButtonBox::Help ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::showHelp );

  {
    mActionsModel = new QgsCustomizationModel( mQgisApp, QgsCustomizationModel::Mode::ActionSelector, this );
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel( this );
    proxyModel->sort( 0 );
    proxyModel->setFilterKeyColumn( 0 );
    proxyModel->setFilterCaseSensitivity( Qt::CaseSensitivity::CaseInsensitive );
    proxyModel->setSortCaseSensitivity( Qt::CaseSensitivity::CaseInsensitive );

    proxyModel->setRecursiveFilteringEnabled( true );
    proxyModel->setSourceModel( mActionsModel );
    mActionsTreeView->setModel( proxyModel );
    mActionsTreeView->header()->resizeSection( 0, 250 );
    connect( mFilterActionsLe, &QgsFilterLineEdit::valueChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString );
  }

  {
    mItemsVisibilityModel = new QgsCustomizationModel( mQgisApp, QgsCustomizationModel::Mode::ItemVisibility, this );
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel( this );
    proxyModel->setFilterKeyColumn( 0 );
    proxyModel->setFilterCaseSensitivity( Qt::CaseSensitivity::CaseInsensitive );
    proxyModel->setSortCaseSensitivity( Qt::CaseSensitivity::CaseInsensitive );

#ifdef DEBUG_MODEL
    new QAbstractItemModelTester( mItemsVisibilityModel, QAbstractItemModelTester::FailureReportingMode::Fatal, this );
#endif

    proxyModel->setRecursiveFilteringEnabled( true );
    proxyModel->setSourceModel( mItemsVisibilityModel );
    mTreeView->setModel( proxyModel );
    mTreeView->setAutoScroll( true );
    mTreeView->resizeColumnToContents( 0 );
    mTreeView->header()->resizeSection( 0, 250 );
    mTreeView->setContextMenuPolicy( Qt::ContextMenuPolicy::ActionsContextMenu );
    mTreeView->addActions( QList<QAction *>() << mDeleteAction << mAddAction );
    connect( mFilterLe, &QgsFilterLineEdit::valueChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString );
    connect( mTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsCustomizationDialog::currentItemChanged );
    connect( mTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsCustomizationDialog::selectedItemsChanged );
    connect( mItemsVisibilityModel, &QAbstractItemModel::rowsInserted, this, &QgsCustomizationDialog::onRowsInserted );
  }

  mActionsTreeView->setEnabled( false );
  mFilterActionsLe->setEnabled( false );
  mTreeView->setEnabled( false );
  toolBar->setEnabled( false );
  mFilterLe->setEnabled( false );
  mCustomizationEnabledCheckBox->setChecked( customization()->isEnabled() );
  updateSplitterSizes();
  currentItemChanged();
  selectedItemsChanged();
}

void QgsCustomizationDialog::reset()
{
  mItemsVisibilityModel->reset();
}

void QgsCustomizationDialog::ok()
{
  apply();
  hide();
}

void QgsCustomizationDialog::apply()
{
  mItemsVisibilityModel->apply();
}

void QgsCustomizationDialog::cancel()
{
  reset();
  hide();
}

void QgsCustomizationDialog::onSaveFile( bool )
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Choose a customization XML file" ), sSettingLastSaveDir->value(), tr( "Customization files (*.xml)" ) );
  fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, QStringList() << u"xml"_s );

  if ( fileName.isEmpty() )
    return;

  const QString error = customization()->writeFile( fileName );
  if ( !error.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error writing customization XML file" ), error );
    return;
  }

  QFileInfo fileInfo( fileName );
  sSettingLastSaveDir->setValue( fileInfo.absoluteDir().absolutePath() );
}

void QgsCustomizationDialog::onLoadFile( bool )
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Choose a customization XML file" ), sSettingLastSaveDir->value(), tr( "Customization files (*.xml)" ) );
  if ( fileName.isEmpty() )
    return;

  QString error = mItemsVisibilityModel->readFile( fileName );
  if ( !error.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error loading customization XML file" ), error );
    return;
  }

  QFileInfo fileInfo( fileName );
  sSettingLastSaveDir->setValue( fileInfo.absoluteDir().absolutePath() );
}

void QgsCustomizationDialog::onExpandAll( bool )
{
  mTreeView->expandAll();
}

void QgsCustomizationDialog::onCollapseAll( bool )
{
  mTreeView->collapseAll();
}

void QgsCustomizationDialog::onSelectAll( bool )
{
  // change state recursively

  std::function<void( const QModelIndex & )> selectAll = [this, &selectAll]( const QModelIndex &root ) {
    if ( mItemsVisibilityModel->data( root, Qt::ItemDataRole::DisplayRole ).value<Qt::CheckState>() == Qt::CheckState::Unchecked )
    {
      mItemsVisibilityModel->setData( root, Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole );
    }

    for ( int i = 0; i < mItemsVisibilityModel->rowCount( root ); i++ )
    {
      selectAll( mItemsVisibilityModel->index( i, 0, root ) );
    }
  };

  selectAll( QModelIndex() );
}

void QgsCustomizationDialog::enableCustomization( bool checked )
{
  mActionsTreeView->setEnabled( checked );
  mTreeView->setEnabled( checked );
  toolBar->setEnabled( checked );
  mFilterLe->setEnabled( checked );
  mFilterActionsLe->setEnabled( checked );

  if ( checked != customization()->isEnabled() )
    customization()->setEnabled( checked );
}

void QgsCustomizationDialog::showHelp()
{
  QgsHelp::openHelp( u"introduction/qgis_configuration.html#sec-customization"_s );
}

void QgsCustomizationDialog::currentItemChanged()
{
  const QModelIndex index = treeViewModel()->mapToSource( mTreeView->currentIndex() );
  QgsCustomization::QgsItem *item = index.isValid() ? static_cast<QgsCustomization::QgsItem *>( index.internalPointer() ) : nullptr;

  const bool isEnabled = item && ( item->hasCapability( QgsCustomization::QgsItem::ItemCapability::AddUserMenuChild ) || item->hasCapability( QgsCustomization::QgsItem::ItemCapability::AddUserToolBarChild ) );
  mAddAction->setEnabled( isEnabled );

  QString tooltip = tr( "Add a user defined menu or toolbar" );
  if ( !isEnabled )
    tooltip += "<br/><br/>" + tr( "Current item doesn't accept user menu or toolbar" );

  mAddAction->setToolTip( tooltip );
}

void QgsCustomizationDialog::selectedItemsChanged()
{
  bool allAcceptDelete = false;
  for ( const QModelIndex &index : mTreeView->selectionModel()->selectedIndexes() )
  {
    const QModelIndex sourceIndex = treeViewModel()->mapToSource( index );
    QgsCustomization::QgsItem *item = sourceIndex.isValid() ? static_cast<QgsCustomization::QgsItem *>( sourceIndex.internalPointer() ) : nullptr;
    allAcceptDelete = item && item->hasCapability( QgsCustomization::QgsItem::ItemCapability::Delete );
    if ( !allAcceptDelete )
      break;
  }

  mDeleteAction->setEnabled( allAcceptDelete );

  QString tooltip = tr( "Delete selected items" );
  if ( !allAcceptDelete )
    tooltip += "<br/><br/>" + tr( "Currently selected item are not all deletable" );

  mDeleteAction->setToolTip( tooltip );
}

void QgsCustomizationDialog::onRowsInserted( const QModelIndex &parent, int first, int )
{
  const QModelIndex newIndex = treeViewModel()->mapFromSource( mItemsVisibilityModel->index( first, 0, parent ) );
  mTreeView->setCurrentIndex( newIndex );
}

void QgsCustomizationDialog::addUserItem()
{
  const QModelIndex parentIndex = treeViewModel()->mapToSource( mTreeView->selectionModel()->currentIndex() );
  ( void ) mItemsVisibilityModel->addUserItem( parentIndex );
}

void QgsCustomizationDialog::deleteSelectedItems()
{
  QModelIndexList sourceIndexes;
  for ( const QModelIndex &index : mTreeView->selectionModel()->selectedIndexes() )
  {
    sourceIndexes << treeViewModel()->mapToSource( index );
  }

  mItemsVisibilityModel->deleteUserItems( sourceIndexes );
}

void QgsCustomizationDialog::updateSplitterSizes()
{
  // the splitter size values are arbitrary here, the only important thing is to set 0
  // when the tool button is not checked
  mSplitter->setSizes( QList<int>() << 200 << ( mListActionsAction->isChecked() ? 200 : 0 ) );
}

void QgsCustomizationDialog::onActionCatchToggled( bool triggered )
{
  if ( triggered )
    connect( QgsApplication::instance(), &QgsApplication::preNotify, this, &QgsCustomizationDialog::preNotify );
  else
    disconnect( QgsApplication::instance(), &QgsApplication::preNotify, this, &QgsCustomizationDialog::preNotify );
}

QAction *QgsCustomizationDialog::findAction( QToolButton *toolbutton )
{
  if ( !toolbutton->parent() )
    return toolbutton->defaultAction();

  // We need to find the QAction that was returned from the call of "QToolBar::addWidget".
  // This is a defaultAction in most cases. But when QToolButton is composed of multiple actions,
  // (e.g. "Select Features by ..." button) we need to go through the parent widget to search for the
  // parent action name.
  const QList<QWidgetAction *> tbWidgetActions = toolbutton->parent()->findChildren<QWidgetAction *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QWidgetAction *act : tbWidgetActions )
  {
    QWidget *widget = act->defaultWidget();
    if ( widget == toolbutton )
      return act;
  }

  return toolbutton->defaultAction();
}

bool QgsCustomizationDialog::selectWidget( QWidget *widget )
{
  if ( !actionCatch->isChecked() )
    return false;

  QString widgetName = widget->objectName();
  if ( QToolButton *toolbutton = qobject_cast<QToolButton *>( widget ) )
  {
    QAction *action = findAction( toolbutton );
    if ( !action )
      return false;
    widgetName = action->objectName();
  }

  QModelIndexList items = mItemsVisibilityModel->match(
    mItemsVisibilityModel->index( TOOLBAR_ROW, 0 ),
    Qt::DisplayRole,
    widgetName,
    2,
    Qt::MatchRecursive
  );

  if ( items.isEmpty() )
    return false;

  const QModelIndex currentIndex = treeViewModel()->mapFromSource( items.first() );
  mTreeView->selectionModel()->setCurrentIndex( currentIndex, QItemSelectionModel::SelectCurrent );

  raise();

  return true;
}

void QgsCustomizationDialog::preNotify( QObject *receiver, QEvent *event, bool *done )
{
  if ( !actionCatch->isChecked() || !customization() )
    return;

  QWidget *widget = qobject_cast<QWidget *>( receiver );
  if ( !widget )
    return;

  // try to select a widget on mouse click
  if ( event->type() == QEvent::MouseButtonPress )
  {
    *done = selectWidget( widget );
  }
}

QgsCustomization *QgsCustomizationDialog::customization() const
{
  return mItemsVisibilityModel->customization().get();
}

QSortFilterProxyModel *QgsCustomizationDialog::treeViewModel() const
{
  return static_cast<QSortFilterProxyModel *>( mTreeView->model() );
}
