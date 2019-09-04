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
#include "qgsbookmarkmodel.h"
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

  mBookmarkModel = new QgsBookmarkManagerProxyModel( QgsApplication::bookmarkManager(), QgsProject::instance()->bookmarkManager(), this );

  lstBookmarks->setModel( mBookmarkModel );
  lstBookmarks->setItemDelegate( new QgsDoubleSpinBoxBookmarksDelegate( this ) );
  lstBookmarks->setSortingEnabled( true );
  lstBookmarks->sortByColumn( 0, Qt::AscendingOrder );

  QgsSettings settings;
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

  QModelIndex newIdx = mBookmarkModel->index( QgsApplication::bookmarkManager()->bookmarks().count() - 1, 0 );
  // Edit new bookmark title
  lstBookmarks->scrollTo( newIdx );
  lstBookmarks->setCurrentIndex( newIdx );
  lstBookmarks->edit( newIdx );
}

void QgsBookmarks::deleteClicked()
{
  QItemSelection selection = lstBookmarks->selectionModel()->selection();
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
    mBookmarkModel->removeRow( row );
  }
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
}

QMap<QString, QModelIndex> QgsBookmarks::getIndexMap()
{
  QMap<QString, QModelIndex> map;
  int rowCount = mBookmarkModel->rowCount();

  for ( int i = 0; i < rowCount; ++i )
  {
    QModelIndex idx = mBookmarkModel->index( i, QgsBookmarkManagerModel::ColumnName ); //Name col
    if ( idx.isValid() )
    {
      QString name = idx.data( Qt::DisplayRole ).toString();
      QString project = idx.sibling( idx.row(), QgsBookmarkManagerModel::ColumnGroup ).data().toString();
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

//
// QgsDoubleSpinBoxBookmarksDelegate
//

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
