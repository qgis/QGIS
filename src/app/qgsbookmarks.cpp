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
#include "moc_qgsbookmarks.cpp"
#include "qgsbookmarkeditordialog.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsbookmarkmanager.h"
#include "qgsbookmarkmodel.h"
#include "qgsmessagebar.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QModelIndex>
#include <QDoubleSpinBox>
#include <QAbstractTableModel>
#include <QToolButton>
#include <QUrl>

const int QgsDoubleSpinBoxBookmarksDelegate::DEFAULT_DECIMAL_PLACES = 6;

QgsBookmarks::QgsBookmarks( QWidget *parent )
  : QgsDockWidget( parent )

{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( lstBookmarks, &QTreeView::doubleClicked, this, &QgsBookmarks::lstBookmarks_doubleClicked );
  lstBookmarks->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( lstBookmarks, &QTreeView::customContextMenuRequested, this, &QgsBookmarks::lstBookmarks_customContextMenuRequested );

  bookmarksDockContents->layout()->setContentsMargins( 0, 0, 0, 0 );
  static_cast<QGridLayout *>( bookmarksDockContents->layout() )->setVerticalSpacing( 0 );

  QToolButton *btnImpExp = new QToolButton;
  btnImpExp->setAutoRaise( true );
  btnImpExp->setToolTip( tr( "Import/Export Bookmarks" ) );
  btnImpExp->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharing.svg" ) ) );
  btnImpExp->setPopupMode( QToolButton::InstantPopup );

  QMenu *share = new QMenu( this );
  share->addAction( actionExport );
  share->addAction( actionImport );
  btnImpExp->setMenu( share );

  connect( actionAdd, &QAction::triggered, this, &QgsBookmarks::addClicked );
  connect( actionDelete, &QAction::triggered, this, &QgsBookmarks::deleteClicked );
  connect( actionZoomTo, &QAction::triggered, this, &QgsBookmarks::zoomToBookmark );
  connect( actionExport, &QAction::triggered, this, &QgsBookmarks::exportToXml );
  connect( actionImport, &QAction::triggered, this, &QgsBookmarks::importFromXml );

  mBookmarkToolbar->addWidget( btnImpExp );

  mBookmarkModel = new QgsBookmarkManagerProxyModel( QgsApplication::bookmarkManager(), QgsProject::instance()->bookmarkManager(), this );

  lstBookmarks->setModel( mBookmarkModel );
  lstBookmarks->setItemDelegate( new QgsDoubleSpinBoxBookmarksDelegate( this ) );
  lstBookmarks->setItemDelegateForColumn( QgsBookmarkManagerModel::ColumnRotation, new QgsDoubleSpinBoxBookmarksDelegate( this, 1 ) );
  lstBookmarks->setSortingEnabled( true );
  lstBookmarks->sortByColumn( 0, Qt::AscendingOrder );

  const QgsSettings settings;
  lstBookmarks->header()->restoreState( settings.value( QStringLiteral( "Windows/Bookmarks/headerstateV2" ) ).toByteArray() );
}

QgsBookmarks::~QgsBookmarks()
{
  saveWindowLocation();
}

void QgsBookmarks::saveWindowLocation()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/Bookmarks/headerstateV2" ), lstBookmarks->header()->saveState() );
}

void QgsBookmarks::addClicked()
{
  QgsMapCanvas *canvas = QgisApp::instance()->mapCanvas();
  Q_ASSERT( canvas );

  QgsBookmark bookmark;
  bookmark.setName( tr( "New bookmark" ) );
  bookmark.setExtent( QgsReferencedRectangle( canvas->extent(), canvas->mapSettings().destinationCrs() ) );
  bookmark.setRotation( canvas->rotation() );
  QgsBookmarkEditorDialog *dlg = new QgsBookmarkEditorDialog( bookmark, false, this, canvas );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}

void QgsBookmarks::deleteClicked()
{
  const QItemSelection selection = lstBookmarks->selectionModel()->selection();
  std::vector<int> rows;
  for ( const auto &selectedIdx : selection.indexes() )
  {
    if ( selectedIdx.column() == 1 )
      rows.push_back( selectedIdx.row() );
  }

  if ( rows.size() == 0 )
    return;

  // make sure the user really wants to delete these bookmarks
  if ( QMessageBox::No == QMessageBox::question( this, tr( "Delete Bookmarks" ), tr( "Are you sure you want to delete %n bookmark(s)?", "number of rows", rows.size() ), QMessageBox::Yes | QMessageBox::No ) )
    return;

  // Remove in reverse order to keep the merged model indexes
  std::sort( rows.begin(), rows.end(), std::greater<int>() );

  for ( const auto &row : rows )
  {
    mBookmarkModel->removeRow( row );
  }
}

void QgsBookmarks::lstBookmarks_doubleClicked( const QModelIndex &index )
{
  Q_UNUSED( index )
  zoomToBookmark();
}

void QgsBookmarks::lstBookmarks_customContextMenuRequested( QPoint pos )
{
  // Get index of item under mouse
  QModelIndex index = lstBookmarks->indexAt( pos );
  if ( !index.isValid() )
  {
    // No bookmark under mouse, display generic menu
    QMenu menu;
    menu.addAction( actionAdd );
    menu.addSeparator();
    menu.addAction( actionExport );
    menu.addAction( actionImport );
    menu.exec( lstBookmarks->mapToGlobal( pos ) );
    return;
  }

  // Create the context menu
  QMenu menu;

  // Add zoom and delete actions
  menu.addAction( actionZoomTo );
  menu.addAction( actionDelete );

  // Get the bookmark
  const QString id = lstBookmarks->model()->data( index, static_cast<int>( QgsBookmarkManagerModel::CustomRole::Id ) ).toString();
  QgsBookmark bookmark = QgsApplication::bookmarkManager()->bookmarkById( id );
  bool inProject = false;
  if ( bookmark.id().isEmpty() )
  {
    inProject = true;
    bookmark = QgsProject::instance()->bookmarkManager()->bookmarkById( id );
  }

  // Add an edit action (similar to the one in QgsBookmarksItemGuiProvider)
  QAction *actionEdit = new QAction( tr( "Edit Spatial Bookmark…" ), &menu );
  connect( actionEdit, &QAction::triggered, this, [bookmark, inProject] {
    QgsBookmarkEditorDialog *dlg = new QgsBookmarkEditorDialog( bookmark, inProject, QgisApp::instance(), QgisApp::instance()->mapCanvas() );
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->show();
  } );
  menu.addAction( actionEdit );
  menu.exec( lstBookmarks->viewport()->mapToGlobal( pos ) );
}

void QgsBookmarks::zoomToBookmark()
{
  const QModelIndex index = lstBookmarks->currentIndex();
  if ( !index.isValid() )
    return;
  zoomToBookmarkIndex( index );
}

void QgsBookmarks::zoomToBookmarkIndex( const QModelIndex &index )
{
  const QgsReferencedRectangle rect = index.data( static_cast<int>( QgsBookmarkManagerModel::CustomRole::Extent ) ).value<QgsReferencedRectangle>();
  try
  {
    if ( QgisApp::instance()->mapCanvas()->setReferencedExtent( rect ) )
    {
      QgisApp::instance()->mapCanvas()->setRotation( index.data( static_cast<int>( QgsBookmarkManagerModel::CustomRole::Rotation ) ).toDouble() );
      QgisApp::instance()->mapCanvas()->refresh();
    }
    else
    {
      QgisApp::instance()->messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Bookmark extent is empty" ) );
    }
  }
  catch ( QgsCsException & )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Could not reproject bookmark extent to project CRS." ) );
  }
}

void QgsBookmarks::importFromXml()
{
  const QgsSettings settings;

  const QString lastUsedDir = settings.value( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QDir::homePath() ).toString();
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Import Bookmarks" ), lastUsedDir, tr( "XML files (*.xml *.XML)" ) );
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
}

QMap<QString, QModelIndex> QgsBookmarks::getIndexMap()
{
  QMap<QString, QModelIndex> map;
  const int rowCount = mBookmarkModel->rowCount();

  for ( int i = 0; i < rowCount; ++i )
  {
    const QModelIndex idx = mBookmarkModel->index( i, QgsBookmarkManagerModel::ColumnName ); //Name col
    if ( idx.isValid() )
    {
      QString name = idx.data( Qt::DisplayRole ).toString();
      const QString project = idx.sibling( idx.row(), QgsBookmarkManagerModel::ColumnGroup ).data().toString();
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

  const QString lastUsedDir = settings.value( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Export Bookmarks" ), lastUsedDir, tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never omitted the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".xml" ), Qt::CaseInsensitive ) )
  {
    fileName += QLatin1String( ".xml" );
  }

  if ( !QgsBookmarkManager::exportToFile( fileName, QList<const QgsBookmarkManager *>() << QgsApplication::bookmarkManager() << QgsProject::instance()->bookmarkManager() ) )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Export Bookmarks" ), tr( "Error exporting bookmark file" ) );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushSuccess( tr( "Export Bookmarks" ), tr( "Successfully exported bookmarks to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
  }

  settings.setValue( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QFileInfo( fileName ).path() );
}

//
// QgsDoubleSpinBoxBookmarksDelegate
//

QgsDoubleSpinBoxBookmarksDelegate::QgsDoubleSpinBoxBookmarksDelegate( QObject *parent, int decimals )
  : QStyledItemDelegate( parent ), mDecimals( decimals == -1 ? QgsDoubleSpinBoxBookmarksDelegate::DEFAULT_DECIMAL_PLACES : decimals )
{
}

QString QgsDoubleSpinBoxBookmarksDelegate::displayText( const QVariant &value, const QLocale &locale ) const
{
  if ( value.userType() == QMetaType::Type::Double )
  {
    return locale.toString( value.toDouble(), 'f', mDecimals );
  }
  return QStyledItemDelegate::displayText( value, locale );
}

QWidget *QgsDoubleSpinBoxBookmarksDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QWidget *widget = QStyledItemDelegate::createEditor( parent, option, index );
  QDoubleSpinBox *spinbox = qobject_cast<QDoubleSpinBox *>( widget );
  if ( spinbox )
  {
    if ( index.column() == QgsBookmarkManagerModel::ColumnRotation )
    {
      spinbox->setRange( -360, 360 );
    }
    spinbox->setDecimals( mDecimals );
  }
  return widget;
}
