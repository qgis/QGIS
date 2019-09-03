/***************************************************************************
               QgsBookmarks.cpp  - Spatial Bookmarks
                             -------------------
    begin                : 2005-04-23
    copyright            : (C) 2005 Gary Sherman
    email                : sherman at mrcc dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsbookmarks.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsbookmarkmanager.h"
#include "qgsmessagebar.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QModelIndex>
#include <QDoubleSpinBox>
#include <QAbstractTableModel>
#include <QToolButton>


const int QgsDoubleSpinBoxBookmarksDelegate::DECIMAL_PLACES = 6;

QgsBookmarks::QgsBookmarks( QWidget *parent )
  : QgsDockWidget( parent )

{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( lstBookmarks, &QTreeView::doubleClicked, this, &QgsBookmarks::lstBookmarks_doubleClicked );

  bookmarksDockContents->layout()->setMargin( 0 );
  bookmarksDockContents->layout()->setContentsMargins( 0, 0, 0, 0 );
  static_cast< QGridLayout * >( bookmarksDockContents->layout() )->setVerticalSpacing( 0 );

  QToolButton *btnImpExp = new QToolButton;
  btnImpExp->setAutoRaise( true );
  btnImpExp->setToolTip( tr( "Import/Export Bookmarks" ) );
  btnImpExp->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharing.svg" ) ) );
  btnImpExp->setPopupMode( QToolButton::InstantPopup );

  QMenu *share = new QMenu( this );
  QAction *btnExport = share->addAction( tr( "&Export" ) );
  QAction *btnImport = share->addAction( tr( "&Import" ) );
  btnExport->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharingExport.svg" ) ) );
  btnImport->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharingImport.svg" ) ) );
  connect( btnExport, &QAction::triggered, this, &QgsBookmarks::exportToXml );
  connect( btnImport, &QAction::triggered, this, &QgsBookmarks::importFromXml );
  btnImpExp->setMenu( share );

  connect( actionAdd, &QAction::triggered, this, &QgsBookmarks::addClicked );
  connect( actionDelete, &QAction::triggered, this, &QgsBookmarks::deleteClicked );
  connect( actionZoomTo, &QAction::triggered, this, &QgsBookmarks::zoomToBookmark );

  mBookmarkToolbar->addWidget( btnImpExp );

  mQgisModel = new QgsBookmarkManagerModel( QgsApplication::bookmarkManager(), this );
  mProjectModel = new QgsBookmarkManagerModel( QgsProject::instance()->bookmarkManager(), this );
  mMergedModel = new QgsMergedBookmarksTableModel( *mQgisModel, *mProjectModel, lstBookmarks, this );

  mProxyModel = new QgsBookmarksProxyModel( );
  mProxyModel->setSourceModel( mMergedModel );
  mProxyModel->setSortCaseSensitivity( Qt::CaseInsensitive );

  lstBookmarks->setModel( mProxyModel );
  lstBookmarks->setItemDelegate( new QgsDoubleSpinBoxBookmarksDelegate( this ) );
  lstBookmarks->setSortingEnabled( true );
  lstBookmarks->sortByColumn( 1, Qt::AscendingOrder );

  connect( mMergedModel, &QgsMergedBookmarksTableModel::selectItem, this, [ = ]( const QModelIndex & index )
  {
    QModelIndex proxyIndex( mProxyModel->mapFromSource( index ) );
    lstBookmarks->scrollTo( proxyIndex );
    lstBookmarks->setCurrentIndex( proxyIndex );
  } );

  connect( mMergedModel, &QgsMergedBookmarksTableModel::layoutChanged, mProxyModel, &QgsBookmarksProxyModel::_resetModel );

  QgsSettings settings;
  lstBookmarks->header()->restoreState( settings.value( QStringLiteral( "Windows/Bookmarks/headerstate" ) ).toByteArray() );
}

QgsBookmarks::~QgsBookmarks()
{
  delete mQgisModel;
  delete mProxyModel;
  saveWindowLocation();
}

void QgsBookmarks::saveWindowLocation()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/Bookmarks/headerstate" ), lstBookmarks->header()->saveState() );
}

void QgsBookmarks::addClicked()
{
  Q_ASSERT( mMergedModel );
  Q_ASSERT( mQgisModel );

  QgsMapCanvas *canvas = QgisApp::instance()->mapCanvas();
  Q_ASSERT( canvas );

  QString projStr;
  if ( QgsProject::instance() )
  {
    if ( !QgsProject::instance()->title().isEmpty() )
    {
      projStr = QgsProject::instance()->title();
    }
    else if ( !QgsProject::instance()->fileName().isEmpty() )
    {
      QFileInfo fi( QgsProject::instance()->fileName() );
      projStr = fi.exists() ? fi.fileName() : QString();
    }
  }

  QgsBookmark b;
  b.setName( tr( "New bookmark" ) );
  b.setGroup( projStr );
  b.setExtent( QgsReferencedRectangle( canvas->extent(), canvas->mapSettings().destinationCrs() ) );
  QgsApplication::bookmarkManager()->addBookmark( b );

  QModelIndex newIdx = mProxyModel->mapFromSource( mMergedModel->index( mQgisModel->rowCount() - 1, 0 ) );
  // Edit new bookmark title
  lstBookmarks->scrollTo( newIdx );
  lstBookmarks->setCurrentIndex( newIdx );
  lstBookmarks->edit( newIdx );
}

void QgsBookmarks::deleteClicked()
{
  QItemSelection selection( mProxyModel->mapSelectionToSource( lstBookmarks->selectionModel()->selection() ) );
  std::vector<int> rows;
  for ( const auto &selectedIdx : selection.indexes() )
  {
    if ( selectedIdx.column() == 1 )
      rows.push_back( selectedIdx.row() );
  }

  if ( rows.size() == 0 )
    return;

  // make sure the user really wants to delete these bookmarks
  if ( QMessageBox::No == QMessageBox::question( this, tr( "Delete Bookmarks" ),
       tr( "Are you sure you want to delete %n bookmark(s)?", "number of rows", rows.size() ),
       QMessageBox::Yes | QMessageBox::No ) )
    return;

  // Remove in reverse order to keep the merged model indexes
  std::sort( rows.begin(), rows.end(), std::greater<int>() );

  for ( const auto &row : rows )
  {
    mMergedModel->removeRow( row );
  }
  mProxyModel->_resetModel();
}

void QgsBookmarks::lstBookmarks_doubleClicked( const QModelIndex &index )
{
  Q_UNUSED( index )
  zoomToBookmark();
}

void QgsBookmarks::zoomToBookmark()
{
  QModelIndex index = lstBookmarks->currentIndex();
  if ( !index.isValid() )
    return;
  zoomToBookmarkIndex( index );
}

void  QgsBookmarks::zoomToBookmarkIndex( const QModelIndex &index )
{
  double xmin = index.sibling( index.row(), 3 ).data().toDouble();
  double ymin = index.sibling( index.row(), 4 ).data().toDouble();
  double xmax = index.sibling( index.row(), 5 ).data().toDouble();
  double ymax = index.sibling( index.row(), 6 ).data().toDouble();
  QString authid = index.sibling( index.row(), 7 ).data().toString();

  QgsRectangle rect = QgsRectangle( xmin, ymin, xmax, ymax );

  // backwards compatibility, older version had -1 in the srid column
  if ( ! authid.isEmpty( ) &&
       authid != QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs().authid() )
  {
    QgsCoordinateTransform ct( QgsCoordinateReferenceSystem::fromOgcWmsCrs( authid ),
                               QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs(), QgsProject::instance() );
    rect = ct.transform( rect );
    if ( rect.isEmpty() )
    {
      QMessageBox::warning( this, tr( "Empty Extent" ), tr( "Reprojected extent is empty." ) );
      return;
    }
  }

  // set the extent to the bookmark and refresh
  QgisApp::instance()->setExtent( rect );
  QgisApp::instance()->mapCanvas()->refresh();
}

void QgsBookmarks::importFromXml()
{
  QgsSettings settings;

  QString lastUsedDir = settings.value( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Import Bookmarks" ), lastUsedDir,
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  if ( !QgsApplication::bookmarkManager()->importFromFile( fileName ) )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Import Bookmarks" ), tr( "Error importing bookmark file" ) );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushSuccess( tr( "Import Bookmarks" ), tr( "Bookmarks imported successfully" ) );
  }

  mProxyModel->_resetModel();
}

QMap<QString, QModelIndex> QgsBookmarks::getIndexMap()
{
  QMap<QString, QModelIndex> map;
  int rowCount = mMergedModel->rowCount();

  for ( int i = 0; i < rowCount; ++i )
  {
    QModelIndex idx = mMergedModel->index( i, 1 ); //Name col
    if ( idx.isValid() )
    {
      QString name = idx.data( Qt::DisplayRole ).toString();
      QString project = idx.sibling( idx.row(), 2 ).data().toString();
      if ( !project.isEmpty() )
      {
        name = name + " (" + project + ")";
      }
      map.insert( name, idx ); //Duplicate name/project pairs are overwritten by subsequent bookmarks
    }
  }

  return map;

}

void QgsBookmarks::exportToXml()
{
  QgsSettings settings;

  QString lastUsedDir = settings.value( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Export Bookmarks" ), lastUsedDir,
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never omitted the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".xml" ), Qt::CaseInsensitive ) )
  {
    fileName += QLatin1String( ".xml" );
  }

  if ( !QgsBookmarkManager::exportToFile( fileName, QList< const QgsBookmarkManager * >() << QgsApplication::bookmarkManager()
                                          << QgsProject::instance()->bookmarkManager() ) )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Export Bookmarks" ), tr( "Error exporting bookmark file" ) );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushSuccess( tr( "Export Bookmarks" ), tr( "Successfully exported bookmarks to <a href=\"%1\">%2</a>" )
        .arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
  }

  settings.setValue( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QFileInfo( fileName ).path() );
}

QgsBookmarkManagerModel::QgsBookmarkManagerModel( QgsBookmarkManager *manager, QObject *parent )
  : QAbstractTableModel( parent )
  , mManager( manager )
{
  connect(
    mManager, &QgsBookmarkManager::bookmarkAdded,
    this, &QgsBookmarkManagerModel::bookmarkAdded );
  connect(
    mManager, &QgsBookmarkManager::bookmarkAboutToBeAdded,
    this, &QgsBookmarkManagerModel::bookmarkAboutToBeAdded );
  connect(
    mManager, &QgsBookmarkManager::bookmarkRemoved,
    this, &QgsBookmarkManagerModel::bookmarkRemoved );
  connect(
    mManager, &QgsBookmarkManager::bookmarkAboutToBeRemoved,
    this, &QgsBookmarkManagerModel::bookmarkAboutToBeRemoved );
  connect(
    mManager, &QgsBookmarkManager::bookmarkChanged,
    this, &QgsBookmarkManagerModel::bookmarkChanged );
}

int QgsBookmarkManagerModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mManager->bookmarks().count();
}

int QgsBookmarkManagerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 7;
}

QVariant QgsBookmarkManagerModel::data( const QModelIndex &index, int role ) const
{
  if ( role != Qt::DisplayRole && role != Qt::EditRole )
  {
    return QVariant();
  }

  QgsBookmark b = mManager->bookmarks().at( index.row() );
  switch ( index.column() )
  {
    case 0:
      return b.name();
    case 1:
      return b.group();
    case 2:
      return b.extent().xMinimum();
    case 3:
      return b.extent().yMinimum();
    case 4:
      return b.extent().xMaximum();
    case 5:
      return b.extent().yMaximum();
    case 6:
      return b.extent().crs().authid();
    default:
      return QVariant();
  }
}

bool QgsBookmarkManagerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( role )
  Q_ASSERT( role == Qt::EditRole );

  QgsBookmark b = mManager->bookmarks().at( index.row() );
  QgsReferencedRectangle extent = b.extent();
  switch ( index.column() )
  {
    case 0:
      b.setName( value.toString() );
      break;
    case 1:
      b.setGroup( value.toString() );
      break;
    case 2:
    {
      bool ok = false;
      extent.setXMinimum( value.toDouble( &ok ) );
      if ( !ok )
        return false;
      break;
    }
    case 3:
    {
      bool ok = false;
      extent.setYMinimum( value.toDouble( &ok ) );
      if ( !ok )
        return false;
      break;
    }
    case 4:
    {
      bool ok = false;
      extent.setXMaximum( value.toDouble( &ok ) );
      if ( !ok )
        return false;
      break;
    }
    case 5:
    {
      bool ok = false;
      extent.setYMaximum( value.toDouble( &ok ) );
      if ( !ok )
        return false;
      break;
    }
    case 6:
    {
      QgsCoordinateReferenceSystem crs;
      if ( !crs.createFromString( value.toString() ) )
        return false;
      extent.setCrs( crs );
      break;
    }
    default:
      return false;
  }

  b.setExtent( extent );
  return mManager->updateBookmark( b );
}

bool QgsBookmarkManagerModel::insertRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent )
  Q_UNUSED( row )
  // append
  int oldCount = mManager->bookmarks().count();
  beginInsertRows( parent, oldCount, oldCount + count );
  bool result = true;
  for ( int i = 0; i < count; ++i )
  {
    bool res = false;
    mBlocked = true;
    mManager->addBookmark( QgsBookmark(), &res );
    mBlocked = false;
    result &= res;
  }
  endInsertRows();
  return result;
}

bool QgsBookmarkManagerModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent )
  beginRemoveRows( parent, row, row + count );

  QList< QgsBookmark > bookmarks = mManager->bookmarks();
  for ( int r = row + count - 1; r >= row; --r )
  {
    mManager->removeBookmark( bookmarks.at( r ).id() );
  }
  endRemoveRows();
  return true;
}

void QgsBookmarkManagerModel::bookmarkAboutToBeAdded( const QString & )
{
  if ( mBlocked )
    return;

  beginInsertRows( QModelIndex(), mManager->bookmarks().count(), mManager->bookmarks().count() );
}

void QgsBookmarkManagerModel::bookmarkAdded( const QString & )
{
  if ( mBlocked )
    return;

  endInsertRows();
}

void QgsBookmarkManagerModel::bookmarkAboutToBeRemoved( const QString &id )
{
  if ( mBlocked )
    return;

  QList< QgsBookmark > bookmarks = mManager->bookmarks();
  bool found = false;
  int i = 0;
  for ( i = 0; i < bookmarks.count(); ++i )
  {
    if ( bookmarks.at( i ).id() == id )
    {
      found = true;
      break;
    }
  }
  if ( !found )
    return;

  beginRemoveRows( QModelIndex(), i, i );
}

void QgsBookmarkManagerModel::bookmarkRemoved( const QString & )
{
  if ( mBlocked )
    return;

  endRemoveRows();
}

void QgsBookmarkManagerModel::bookmarkChanged( const QString &id )
{
  if ( mBlocked )
    return;

  QList< QgsBookmark > bookmarks = mManager->bookmarks();
  bool found = false;
  int i = 0;
  for ( i = 0; i < bookmarks.count(); ++i )
  {
    if ( bookmarks.at( i ).id() == id )
    {
      found = true;
      break;
    }
  }
  if ( !found )
    return;

  emit dataChanged( index( i, 0 ), index( i, columnCount() - 1 ) );
}


QgsMergedBookmarksTableModel::QgsMergedBookmarksTableModel( QAbstractTableModel &qgisTableModel, QAbstractTableModel &projectTableModel, QTreeView *treeView, QObject *parent )
  : QAbstractTableModel( parent )
  , mQgisTableModel( qgisTableModel )
  , mProjectTableModel( projectTableModel )
  , mTreeView( treeView )
{

  connect(
    &mQgisTableModel, &QAbstractTableModel::layoutChanged,
    this, &QgsMergedBookmarksTableModel::allLayoutChanged );

  connect(
    &mQgisTableModel, &QAbstractTableModel::rowsInserted,
    this, &QgsMergedBookmarksTableModel::allLayoutChanged );

  connect(
    &mQgisTableModel, &QAbstractTableModel::rowsRemoved,
    this, &QgsMergedBookmarksTableModel::allLayoutChanged );

  connect(
    &projectTableModel, &QAbstractTableModel::layoutChanged,
    this, &QgsMergedBookmarksTableModel::allLayoutChanged );

  connect(
    &projectTableModel, &QAbstractTableModel::rowsInserted,
    this, &QgsMergedBookmarksTableModel::allLayoutChanged );

  connect(
    &projectTableModel, &QAbstractTableModel::rowsRemoved,
    this, &QgsMergedBookmarksTableModel::allLayoutChanged );

}

int QgsMergedBookmarksTableModel::rowCount( const QModelIndex &parent ) const
{
  return mQgisTableModel.rowCount( parent ) + mProjectTableModel.rowCount( parent );
}

int QgsMergedBookmarksTableModel::columnCount( const QModelIndex &parent ) const
{
  return mQgisTableModel.columnCount( parent ) + 1;
}

QVariant QgsMergedBookmarksTableModel::data( const QModelIndex &index, int role ) const
{
  QVariant value;
  // Is it checkbox column?
  if ( index.column() == mQgisTableModel.columnCount() && role == Qt::CheckStateRole )
  {
    value = index.row() < mQgisTableModel.rowCount() ? Qt::Unchecked : Qt::Checked;
  }
  else
  {
    // Is it a SQLite stored entry ?
    if ( index.row() < mQgisTableModel.rowCount() )
    {
      value = mQgisTableModel.data( index, role );
    }
    else // ... it is a project stored bookmark
    {
      value = mProjectTableModel.data( this->index( index.row() - mQgisTableModel.rowCount(), index.column() ), role );
    }
  }
  return value;
}

bool QgsMergedBookmarksTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  bool result = false;
  // last column triggers a move from QGIS to project bookmark
  if ( index.column() == mQgisTableModel.columnCount() )
  {
    if ( index.row() < mQgisTableModel.rowCount() )
    {
      // Move from application storage to project
      const QString id = QgsApplication::bookmarkManager()->bookmarks().at( index.row() ).id();
      QgsApplication::bookmarkManager()->moveBookmark( id, QgsProject::instance()->bookmarkManager() );
      emit selectItem( this->index( rowCount() - 1, 1 ) );
    }
    else
    {
      //Move from project to application storage
      const QString id = QgsProject::instance()->bookmarkManager()->bookmarks().at( index.row() - mQgisTableModel.rowCount() ).id();
      QgsProject::instance()->bookmarkManager()->moveBookmark( id, QgsApplication::bookmarkManager() );
      emit selectItem( this->index( mQgisTableModel.rowCount() - 1, 1 ) );
    }
    result = true;
  }
  else
  {
    if ( index.row() < mQgisTableModel.rowCount() )
    {
      result = mQgisTableModel.setData( index, value, role );
    }
    else
    {
      result = mProjectTableModel.setData( this->index( index.row() - mQgisTableModel.rowCount(), index.column() ), value, role );
    }
  }
  if ( result )
    emit dataChanged( index, index );
  return result;
}

Qt::ItemFlags QgsMergedBookmarksTableModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if ( index.column() == mQgisTableModel.columnCount() )
  {
    if ( !projectAvailable() )
    {
      return Qt::ItemIsSelectable;
    }
    flags |= Qt::ItemIsUserCheckable;
  }
  else
  {
    // Skip projection: not editable!
    if ( index.column() != mQgisTableModel.columnCount() - 1 )
      flags |= Qt::ItemIsEditable;
  }
  return flags;
}

bool QgsMergedBookmarksTableModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_ASSERT( count == 1 );
  bool result;
  if ( row < mQgisTableModel.rowCount() )
  {
    result = mQgisTableModel.removeRows( row, count, parent );
  }
  else
  {
    result = mProjectTableModel.removeRows( row - mQgisTableModel.rowCount(), count, parent );
  }
  allLayoutChanged();
  return result;
}

QVariant QgsMergedBookmarksTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case 0:
        return tr( "Name" );
      case 1:
        return tr( "Group" );
      case 2:
        return tr( "xMin" );
      case 3:
        return tr( "yMin" );
      case 4:
        return tr( "xMax" );
      case 5:
        return tr( "yMax" );
      case 6:
        return tr( "CRS" );
      case 7:
        return tr( "In Project" );
    }
  }
  return mQgisTableModel.headerData( section, orientation, role );
}

QAbstractTableModel *QgsMergedBookmarksTableModel::qgisModel()
{
  return &mQgisTableModel;
}

bool QgsMergedBookmarksTableModel::projectAvailable() const
{
  return ! QgsProject::instance()->fileName().isEmpty();
}

QgsBookmarksProxyModel::QgsBookmarksProxyModel( QObject *parent ):
  QSortFilterProxyModel( parent )
{

}

QVariant QgsBookmarksProxyModel::headerData( int section, Qt::Orientation orientation, int role ) const
{

  return sourceModel()->headerData( section, orientation, role );
}

QgsDoubleSpinBoxBookmarksDelegate::QgsDoubleSpinBoxBookmarksDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{

}

QString QgsDoubleSpinBoxBookmarksDelegate::displayText( const QVariant &value, const QLocale &locale ) const
{
  if ( value.userType() == QVariant::Double )
  {
    return locale.toString( value.toDouble(), 'f', QgsDoubleSpinBoxBookmarksDelegate::DECIMAL_PLACES );
  }
  else
  {
    return QStyledItemDelegate::displayText( value, locale );
  }
}

QWidget *QgsDoubleSpinBoxBookmarksDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QWidget *widget = QStyledItemDelegate::createEditor( parent, option, index );
  QDoubleSpinBox *spinbox = qobject_cast<QDoubleSpinBox *>( widget );
  if ( spinbox )
    spinbox->setDecimals( QgsDoubleSpinBoxBookmarksDelegate::DECIMAL_PLACES );
  return widget;
}
