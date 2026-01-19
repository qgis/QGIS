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
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QToolButton>
#include <QWidgetAction>
#include <qnamespace.h>

#include "moc_qgscustomizationdialog.cpp"

#ifdef QGIS_DEBUG
#include <QAbstractItemModelTester>
#endif

constexpr int TOOLBAR_COLUMN = 4;

const QgsSettingsEntryString *QgsCustomizationDialog::sSettingLastSaveDir = new QgsSettingsEntryString( u"last-save-directory"_s, sTreeCustomization, QDir::homePath(), u"Last directory used when saving a customization XML file"_s );

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
  if ( !index.isValid() || role != Qt::CheckStateRole )
    return false;

  QgsCustomization::QgsItem *item = static_cast<QgsCustomization::QgsItem *>( index.internalPointer() );
  if ( item )
  {
    item->setVisible( value.value<Qt::CheckState>() == Qt::CheckState::Checked );
    emit dataChanged( index, index, QList<int>() << Qt::CheckStateRole );
    return true;
  }

  return false;
}

Qt::ItemFlags QgsCustomizationDialog::QgsCustomizationModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if ( mMode == Mode::ItemVisibility && index.parent().isValid() && index.column() == 0 )
    flags |= Qt::ItemIsUserCheckable;

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
  // NOLINTBEGIN(bugprone-branch-clone)
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
  // NOLINTEND(bugprone-branch-clone)
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

  connect( buttonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::ok );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::apply );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::cancel );
  connect( buttonBox->button( QDialogButtonBox::Reset ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::reset );
  connect( buttonBox->button( QDialogButtonBox::Help ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::showHelp );

  mItemsVisibilityModel = new QgsCustomizationModel( mQgisApp, QgsCustomizationModel::Mode::ItemVisibility, this );
  QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel( this );
  proxyModel->sort( 0 );
  proxyModel->setFilterKeyColumn( 0 );
  proxyModel->setFilterCaseSensitivity( Qt::CaseSensitivity::CaseInsensitive );
  proxyModel->setSortCaseSensitivity( Qt::CaseSensitivity::CaseInsensitive );

#ifdef QGIS_DEBUG
  new QAbstractItemModelTester( mItemsVisibilityModel, QAbstractItemModelTester::FailureReportingMode::Fatal, this );
#endif

  proxyModel->setRecursiveFilteringEnabled( true );
  proxyModel->setSourceModel( mItemsVisibilityModel );
  mTreeView->setModel( proxyModel );
  mTreeView->resizeColumnToContents( 0 );
  mTreeView->header()->resizeSection( 0, 250 );
  connect( mFilterLe, &QgsFilterLineEdit::valueChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString );

  mTreeView->setEnabled( false );
  toolBar->setEnabled( false );
  mFilterLe->setEnabled( false );
  mCustomizationEnabledCheckBox->setChecked( customization()->isEnabled() );
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
  mTreeView->setEnabled( checked );
  toolBar->setEnabled( checked );
  mFilterLe->setEnabled( checked );

  if ( checked != customization()->isEnabled() )
    customization()->setEnabled( checked );
}

void QgsCustomizationDialog::showHelp()
{
  QgsHelp::openHelp( u"introduction/qgis_configuration.html#sec-customization"_s );
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
    mItemsVisibilityModel->index( TOOLBAR_COLUMN, 0 ),
    Qt::DisplayRole,
    widgetName,
    2,
    Qt::MatchRecursive
  );

  if ( items.isEmpty() )
    return false;

  const QModelIndex currentIndex = static_cast<QSortFilterProxyModel *>( mTreeView->model() )->mapFromSource( items.first() );
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

const std::unique_ptr<QgsCustomization> &QgsCustomizationDialog::customization() const
{
  return mItemsVisibilityModel->customization();
}
