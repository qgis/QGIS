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
#include "qgscontexthelp.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgscrscache.h"

#include "qgslogger.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QModelIndex>
#include <QAbstractTableModel>


QgsBookmarks::QgsBookmarks( QWidget *parent )
    : QgsDockWidget( parent )
{
  setupUi( this );
  restorePosition();

  bookmarksDockContents->layout()->setMargin( 0 );
  bookmarksDockContents->layout()->setContentsMargins( 0, 0, 0, 0 );
  static_cast< QGridLayout* >( bookmarksDockContents->layout() )->setVerticalSpacing( 0 );

  QToolButton* btnImpExp = new QToolButton;
  btnImpExp->setAutoRaise( true );
  btnImpExp->setToolTip( tr( "Import/Export Bookmarks" ) );
  btnImpExp->setIcon( QgsApplication::getThemeIcon( "/mActionSharing.svg" ) );
  btnImpExp->setPopupMode( QToolButton::InstantPopup );

  QMenu *share = new QMenu( this );
  QAction *btnExport = share->addAction( tr( "&Export" ) );
  QAction *btnImport = share->addAction( tr( "&Import" ) );
  btnExport->setIcon( QgsApplication::getThemeIcon( "/mActionSharingExport.svg" ) );
  btnImport->setIcon( QgsApplication::getThemeIcon( "/mActionSharingImport.svg" ) );
  connect( btnExport, SIGNAL( triggered() ), this, SLOT( exportToXML() ) );
  connect( btnImport, SIGNAL( triggered() ), this, SLOT( importFromXML() ) );
  btnImpExp->setMenu( share );

  connect( actionAdd, SIGNAL( triggered() ), this, SLOT( addClicked() ) );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteClicked() ) );
  connect( actionZoomTo, SIGNAL( triggered() ), this, SLOT( zoomToBookmark() ) );

  mBookmarkToolbar->addWidget( btnImpExp );

  // open the database
  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "bookmarks" );
  db.setDatabaseName( QgsApplication::qgisUserDbFilePath() );
  if ( !db.open() )
  {
    QMessageBox::warning( this, tr( "Error" ),
                          tr( "Unable to open bookmarks database.\nDatabase: %1\nDriver: %2\nDatabase: %3" )
                          .arg( QgsApplication::qgisUserDbFilePath(),
                                db.lastError().driverText(),
                                db.lastError().databaseText() )
                        );
    deleteLater();
    return;
  }

  mQgisModel = new QSqlTableModel( this, db );
  mQgisModel->setTable( "tbl_bookmarks" );
  mQgisModel->setSort( 0, Qt::AscendingOrder );
  mQgisModel->select();
  mQgisModel->setEditStrategy( QSqlTableModel::OnFieldChange );

  // set better headers then column names from table
  mQgisModel->setHeaderData( 0, Qt::Horizontal, tr( "ID" ) );
  mQgisModel->setHeaderData( 1, Qt::Horizontal, tr( "Name" ) );
  mQgisModel->setHeaderData( 2, Qt::Horizontal, tr( "Project" ) );
  mQgisModel->setHeaderData( 3, Qt::Horizontal, tr( "xMin" ) );
  mQgisModel->setHeaderData( 4, Qt::Horizontal, tr( "yMin" ) );
  mQgisModel->setHeaderData( 5, Qt::Horizontal, tr( "xMax" ) );
  mQgisModel->setHeaderData( 6, Qt::Horizontal, tr( "yMax" ) );
  mQgisModel->setHeaderData( 7, Qt::Horizontal, tr( "SRID" ) );

  mProjectModel = new QgsProjectBookmarksTableModel();
  mModel.reset( new QgsMergedBookmarksTableModel( *mQgisModel, *mProjectModel, lstBookmarks ) );

  lstBookmarks->setModel( mModel.data() );

  QSettings settings;
  lstBookmarks->header()->restoreState( settings.value( "/Windows/Bookmarks/headerstate" ).toByteArray() );

#ifndef QGISDEBUG
  lstBookmarks->setColumnHidden( 0, true );
#endif
}

QgsBookmarks::~QgsBookmarks()
{
  delete mQgisModel;
  delete mProjectModel;
  QSqlDatabase::removeDatabase( "bookmarks" );
  saveWindowLocation();
}

void QgsBookmarks::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/Bookmarks/geometry" ).toByteArray() );
}

void QgsBookmarks::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/Windows/Bookmarks/geometry", saveGeometry() );
  settings.setValue( "/Windows/Bookmarks/headerstate", lstBookmarks->header()->saveState() );
}

void QgsBookmarks::addClicked()
{
  Q_ASSERT( mModel );
  Q_ASSERT( mQgisModel );

  QgsMapCanvas *canvas = QgisApp::instance()->mapCanvas();
  Q_ASSERT( canvas );

  QSqlQuery query( "INSERT INTO tbl_bookmarks(bookmark_id,name,project_name,xmin,ymin,xmax,ymax,projection_srid)"
                   "  VALUES (NULL,:name,:project_name,:xmin,:xmax,:ymin,:ymax,:projection_srid)",
                   mQgisModel->database() );

  QString projStr( "" );
  if ( QgsProject::instance() )
  {
    if ( !QgsProject::instance()->title().isEmpty() )
    {
      projStr = QgsProject::instance()->title();
    }
    else if ( !QgsProject::instance()->fileName().isEmpty() )
    {
      QFileInfo fi( QgsProject::instance()->fileName() );
      projStr = fi.exists() ? fi.fileName() : "";
    }
  }

  query.bindValue( ":name", tr( "New bookmark" ) );
  query.bindValue( ":project_name", projStr );
  query.bindValue( ":xmin", canvas->extent().xMinimum() );
  query.bindValue( ":ymin", canvas->extent().yMinimum() );
  query.bindValue( ":xmax", canvas->extent().xMaximum() );
  query.bindValue( ":ymax", canvas->extent().yMaximum() );
  query.bindValue( ":projection_srid", QVariant::fromValue( canvas->mapSettings().destinationCrs().srsid() ) );
  if ( query.exec() )
  {
    mQgisModel->setSort( 0, Qt::AscendingOrder );
    mQgisModel->select();
    lstBookmarks->scrollTo( mModel->index( mQgisModel->rowCount() - 1, 1 ) );
    lstBookmarks->setCurrentIndex( mModel->index( mQgisModel->rowCount() - 1, 1 ) );
    lstBookmarks->edit( mModel->index( mQgisModel->rowCount() - 1, 1 ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Unable to create the bookmark.\nDriver:%1\nDatabase:%2" )
                          .arg( query.lastError().driverText(),
                                query.lastError().databaseText() ) );
  }
}

void QgsBookmarks::deleteClicked()
{
  QList<int> rows;
  Q_FOREACH ( const QModelIndex &idx, lstBookmarks->selectionModel()->selectedIndexes() )
  {
    if ( idx.column() == 1 )
    {
      rows << idx.row();
    }
  }

  if ( rows.isEmpty() )
    return;

  // make sure the user really wants to delete these bookmarks
  if ( QMessageBox::Cancel == QMessageBox::information( this, tr( "Really Delete?" ),
       tr( "Are you sure you want to delete %n bookmark(s)?", "number of rows", rows.size() ),
       QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  int i = 0;
  Q_FOREACH ( int row, rows )
  {
    mModel->removeRow( row - i );
    i++;
  }
}

void QgsBookmarks::on_lstBookmarks_doubleClicked( const QModelIndex & index )
{
  Q_UNUSED( index );
  zoomToBookmark();
}

void QgsBookmarks::zoomToBookmark()
{
  QModelIndex index = lstBookmarks->currentIndex();
  if ( !index.isValid() )
    return;

  double xmin = index.sibling( index.row(), 3 ).data().toDouble();
  double ymin = index.sibling( index.row(), 4 ).data().toDouble();
  double xmax = index.sibling( index.row(), 5 ).data().toDouble();
  double ymax = index.sibling( index.row(), 6 ).data().toDouble();
  int srid = index.sibling( index.row(), 7 ).data().toInt();

  QgsRectangle rect = QgsRectangle( xmin, ymin, xmax, ymax );

  // backwards compatibility, older version had -1 in the srid column
  if ( srid > 0 &&
       srid != QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs().srsid() )
  {
    QgsCoordinateTransform ct( QgsCRSCache::instance()->crsBySrsId( srid ),
                               QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs() );
    rect = ct.transform( rect );
    if ( rect.isEmpty() )
    {
      QMessageBox::warning( this, tr( "Empty extent" ), tr( "Reprojected extent is empty." ) );
      return;
    }
  }

  // set the extent to the bookmark and refresh
  QgisApp::instance()->setExtent( rect );
  QgisApp::instance()->mapCanvas()->refresh();
}

void QgsBookmarks::importFromXML()
{
  QSettings settings;

  QString lastUsedDir = settings.value( "/Windows/Bookmarks/LastUsedDirectory", QDir::homePath() ).toString();
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Import Bookmarks" ), lastUsedDir,
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QFile f( fileName );
  if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    return;
  }

  QDomDocument doc;
  if ( !doc.setContent( &f ) )
  {
    return;
  }
  f.close();

  QDomElement docElem = doc.documentElement();
  QDomNodeList nodeList = docElem.elementsByTagName( "bookmark" );

  Q_ASSERT( mModel );

  QString queries;

  for ( int i = 0;i < nodeList.count(); i++ )
  {
    QDomNode bookmark = nodeList.at( i );
    QDomElement name = bookmark.firstChildElement( "name" );
    QDomElement prjname = bookmark.firstChildElement( "project" );
    QDomElement xmin = bookmark.firstChildElement( "xmin" );
    QDomElement xmax = bookmark.firstChildElement( "xmax" );
    QDomElement ymin = bookmark.firstChildElement( "ymin" );
    QDomElement ymax = bookmark.firstChildElement( "ymax" );
    QDomElement srid = bookmark.firstChildElement( "sr_id" );

    queries += "INSERT INTO tbl_bookmarks(bookmark_id,name,project_name,xmin,ymin,xmax,ymax,projection_srid)"
               "  VALUES (NULL,"
               "'" + name.text() + "',"
               "'" + prjname.text() + "',"
               + xmin.text() + ','
               + ymin.text() + ','
               + xmax.text() + ','
               + ymax.text() + ','
               + srid.text() + ");";
  }

  QStringList queriesList = queries.split( ';' );
  QSqlQuery query( mQgisModel->database() );

  Q_FOREACH ( const QString& queryTxt, queriesList )
  {
    if ( queryTxt.trimmed().isEmpty() )
    {
      continue;
    }
    if ( !query.exec( queryTxt ) )
    {
      QMessageBox::warning( this, tr( "Error" ), tr( "Unable to create the bookmark.\nDriver: %1\nDatabase: %2" )
                            .arg( query.lastError().driverText(),
                                  query.lastError().databaseText() ) );
    }
    query.finish();
  }
  mQgisModel->setSort( 0, Qt::AscendingOrder );
  mQgisModel->select();
}

void QgsBookmarks::exportToXML()
{
  QSettings settings;

  QString lastUsedDir = settings.value( "/Windows/Bookmarks/LastUsedDirectory", QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Export bookmarks" ), lastUsedDir,
                     tr( "XML files( *.xml *.XML )" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never ommited the extension from the file name
  if ( !fileName.endsWith( ".xml", Qt::CaseInsensitive ) )
  {
    fileName += ".xml";
  }

  QDomDocument doc( "qgis_bookmarks" );
  QDomElement root = doc.createElement( "qgis_bookmarks" );
  doc.appendChild( root );

  int rowCount = mModel->rowCount();
  int colCount = mModel->columnCount();

  QList<QString> headerList;
  headerList << "id" << "name" << "project" << "xmin"
  << "ymin" << "xmax" << "ymax" << "sr_id";

  for ( int i = 0; i < rowCount; ++i )
  {
    QDomElement bookmark = doc.createElement( "bookmark" );
    root.appendChild( bookmark );
    for ( int j = 0; j < colCount; j++ )
    {
      QModelIndex idx = mModel->index( i, j );
      if ( idx.isValid() )
      {
        QString value = idx.data( Qt::DisplayRole ).toString();
        QDomText idText = doc.createTextNode( value );
        QString header = headerList.at( j );
        QDomElement id = doc.createElement( header );
        id.appendChild( idText );
        bookmark.appendChild( id );
      }
    }
  }

  QFile f( fileName );
  if ( !f.open( QFile::WriteOnly ) )
  {
    f.close();
    return;
  }

  QTextStream out( &f );
  out.setCodec( "UTF - 8" );
  doc.save( out, 2 );
  f.close();

  settings.setValue( "/Windows/Bookmarks/LastUsedDirectory", QFileInfo( fileName ).path() );
}


QgsProjectBookmarksTableModel::QgsProjectBookmarksTableModel()
{
  QObject::connect(
    QgisApp::instance(), SIGNAL( projectRead() ),
    this, SLOT( projectRead() ) );
}

int QgsProjectBookmarksTableModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent );

  return QgsProject::instance()->readNumEntry( "Bookmarks", "/count" );
}

int QgsProjectBookmarksTableModel::columnCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent );

  return 8;
}

QVariant QgsProjectBookmarksTableModel::data( const QModelIndex& index, int role ) const
{
  Q_UNUSED( role );
  Q_ASSERT( role == Qt::DisplayRole );

  switch ( index.column() )
  {
    case 1:
      return QgsProject::instance()->readEntry( "Bookmarks", QString( "/Row-%1/Name" ).arg( index.row() ) );
    case 2:
      return QgsProject::instance()->readEntry( "Bookmarks", QString( "/Row-%1/Project" ).arg( index.row() ) );
    case 3:
      return QgsProject::instance()->readDoubleEntry( "Bookmarks", QString( "/Row-%1/MinX" ).arg( index.row() ) );
    case 4:
      return QgsProject::instance()->readDoubleEntry( "Bookmarks", QString( "/Row-%1/MinY" ).arg( index.row() ) );
    case 5:
      return QgsProject::instance()->readDoubleEntry( "Bookmarks", QString( "/Row-%1/MaxX" ).arg( index.row() ) );
    case 6:
      return QgsProject::instance()->readDoubleEntry( "Bookmarks", QString( "/Row-%1/MaxY" ).arg( index.row() ) );
    case 7:
      return QgsProject::instance()->readNumEntry( "Bookmarks", QString( "/Row-%1/SRID" ).arg( index.row() ) );
    default:
      return QVariant();
  }
}

bool QgsProjectBookmarksTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  Q_UNUSED( role );
  Q_ASSERT( role == Qt::EditRole );

  switch ( index.column() )
  {
    case 1:
      QgsProject::instance()->writeEntry( "Bookmarks", QString( "/Row-%1/Name" ).arg( index.row() ), value.value<QString>() );
      return true;
    case 2:
      QgsProject::instance()->writeEntry( "Bookmarks", QString( "/Row-%1/Project" ).arg( index.row() ), value.value<QString>() );
      return true;
    case 3:
      QgsProject::instance()->writeEntry( "Bookmarks", QString( "/Row-%1/MinX" ).arg( index.row() ), value.toDouble() );
      return true;
    case 4:
      QgsProject::instance()->writeEntry( "Bookmarks", QString( "/Row-%1/MinY" ).arg( index.row() ), value.toDouble() );
      return true;
    case 5:
      QgsProject::instance()->writeEntry( "Bookmarks", QString( "/Row-%1/MaxX" ).arg( index.row() ), value.toDouble() );
      return true;
    case 6:
      QgsProject::instance()->writeEntry( "Bookmarks", QString( "/Row-%1/MaxY" ).arg( index.row() ), value.toDouble() );
      return true;
    case 7:
      QgsProject::instance()->writeEntry( "Bookmarks", QString( "/Row-%1/SRID" ).arg( index.row() ), value.toInt() );
      return true;
    default:
      return false;
  }
}

bool QgsProjectBookmarksTableModel::insertRows( int row, int count, const QModelIndex& parent )
{
  Q_UNUSED( row );
  Q_UNUSED( parent );

  return QgsProject::instance()->writeEntry( "Bookmarks", "/count", QgsProject::instance()->readNumEntry( "Bookmarks", "/count" ) + count );
}

bool QgsProjectBookmarksTableModel::removeRows( int row, int count, const QModelIndex& parent )
{
  Q_UNUSED( parent );

  for ( int newRow = row ; newRow < rowCount() - count ; newRow++ )
  {
    for ( int column = 0 ; column < columnCount() ; column++ )
    {
      setData( index( newRow, column ), data( index( newRow + count, column ) ) );
    }
  }
  for ( int newRow = rowCount() - count ; newRow < rowCount() ; newRow++ )
  {
    QgsProject::instance()->removeEntry( "Bookmarks", QString( "/Row-%1" ).arg( newRow ) );
  }

  QgsProject::instance()->writeEntry( "Bookmarks", "/count", QgsProject::instance()->readNumEntry( "Bookmarks", "/count" ) - count );

  return true;
}

QgsMergedBookmarksTableModel::QgsMergedBookmarksTableModel( QAbstractTableModel& qgisTableModel, QAbstractTableModel& projectTableModel, QTreeView* treeView )
    : mQgisTableModel( qgisTableModel )
    , mProjectTableModel( projectTableModel )
    , mTreeView( treeView )
    , mProjectOpen( false )
{
  connect(
    QgisApp::instance(), SIGNAL( projectRead() ),
    this, SLOT( projectRead() ) );

  connect(
    &mQgisTableModel, SIGNAL( layoutChanged() ),
    this, SLOT( allLayoutChanged() ) );
  connect(
    &mQgisTableModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
    this, SLOT( qgisDataChanged( const QModelIndex&, const QModelIndex& ) ) );
  connect(
    &mQgisTableModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
    this, SLOT( allLayoutChanged() ) );
  connect(
    &mQgisTableModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
    this, SLOT( allLayoutChanged() ) );

  connect(
    &projectTableModel, SIGNAL( layoutChanged() ),
    this, SLOT( allLayoutChanged() ) );
}

int QgsMergedBookmarksTableModel::rowCount( const QModelIndex& parent ) const
{
  return mQgisTableModel.rowCount( parent ) + mProjectTableModel.rowCount( parent );
}

int QgsMergedBookmarksTableModel::columnCount( const QModelIndex& parent ) const
{
  return mQgisTableModel.columnCount( parent ) + 1;
}

QVariant QgsMergedBookmarksTableModel::data( const QModelIndex& index, int role ) const
{
  // is project or application
  if ( index.column() == mQgisTableModel.columnCount() )
  {
    if ( role == Qt::CheckStateRole )
    {
      return index.row() < mQgisTableModel.rowCount() ? Qt::Unchecked : Qt::Checked;
    }
    else
    {
      return QVariant();
    }
  }
  if ( index.row() < mQgisTableModel.rowCount() )
  {
    return mQgisTableModel.data( index, role );
  }
  else
  {
    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {
      return mProjectTableModel.data( this->index( index.row() - mQgisTableModel.rowCount(), index.column() ), role );
    }
    else
    {
      return mQgisTableModel.data( this->index( 0, index.column() ), role );
    }
  }
}

bool QgsMergedBookmarksTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  // last column triggers a move from QGIS to project bookmark
  if ( index.column() == mQgisTableModel.columnCount() )
  {
    if ( index.row() < mQgisTableModel.rowCount() )
    {
      moveBookmark( mQgisTableModel, mProjectTableModel, index.row() );
      mTreeView->scrollTo( this->index( rowCount() - 1, 1 ) );
      mTreeView->setCurrentIndex( this->index( rowCount() - 1, 1 ) );
      mTreeView->selectionModel()->select( this->index( rowCount() - 1, 1 ), QItemSelectionModel::Rows );
    }
    else
    {
      moveBookmark( mProjectTableModel, mQgisTableModel, index.row() - mQgisTableModel.rowCount() );
      mTreeView->scrollTo( this->index( mQgisTableModel.rowCount() - 1, 1 ) );
      mTreeView->setCurrentIndex( this->index( mQgisTableModel.rowCount() - 1, 1 ) );
      mTreeView->selectionModel()->select( this->index( mQgisTableModel.rowCount() - 1, 1 ), QItemSelectionModel::Rows );
    }
    return true;
  }

  if ( index.row() < mQgisTableModel.rowCount() )
  {
    return mQgisTableModel.setData( index, value, role );
  }
  else
  {
    return mProjectTableModel.setData( this->index( index.row() - mQgisTableModel.rowCount(), index.column() ), value, role );
  }
}

Qt::ItemFlags QgsMergedBookmarksTableModel::flags( const QModelIndex& index ) const
{
  Q_UNUSED( index );

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if ( index.column() == mQgisTableModel.columnCount() )
  {
    if ( !mProjectOpen )
    {
      return Qt::ItemIsSelectable;
    }
    flags |= Qt::ItemIsUserCheckable;
  }
  else
  {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

bool QgsMergedBookmarksTableModel::removeRows( int row, int count, const QModelIndex& parent )
{
  Q_ASSERT( count == 1 );

  if ( row < mQgisTableModel.rowCount() )
  {
    return mQgisTableModel.removeRows( row, count, parent );
  }
  else
  {
    return mProjectTableModel.removeRows( row - mQgisTableModel.rowCount(), count, parent );
  }
}

QVariant QgsMergedBookmarksTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( section == mQgisTableModel.columnCount() )
  {
    if ( role == Qt::DisplayRole )
    {
      return QVariant( tr( "In Project" ) );
    }
    else
    {
      return mQgisTableModel.headerData( 0, orientation, role );
    }
  }
  else
  {
    return mQgisTableModel.headerData( section, orientation, role );
  }
}

QAbstractTableModel* QgsMergedBookmarksTableModel::qgisModel()
{
  return &mQgisTableModel;
}

void QgsMergedBookmarksTableModel::moveBookmark( QAbstractTableModel& modelFrom, QAbstractTableModel& modelTo, int row )
{
  QSqlTableModel* qgisModel = dynamic_cast<QSqlTableModel*>( &modelTo );
  if ( qgisModel == NULL )
  {
    modelTo.insertRow( -1 );
    for ( int column = 1 ; column < modelFrom.columnCount() ; column++ )
    {
      modelTo.setData(
        modelTo.index( modelTo.rowCount() - 1, column ),
        modelFrom.data( modelFrom.index( row, column ) ) );
    }
  }
  else
  {
    QSqlQuery query( "INSERT INTO tbl_bookmarks(bookmark_id,name,project_name,xmin,ymin,xmax,ymax,projection_srid)"
                     "  VALUES (NULL,:name,:project_name,:xmin,:xmax,:ymin,:ymax,:projection_srid)",
                     qgisModel->database() );

    query.bindValue( ":name", modelFrom.data( modelFrom.index( row, 1 ) ).value<QString>() );
    query.bindValue( ":project_name", modelFrom.data( modelFrom.index( row, 2 ) ).value<QString>() );
    query.bindValue( ":xmin", modelFrom.data( modelFrom.index( row, 3 ) ).toDouble() );
    query.bindValue( ":ymin", modelFrom.data( modelFrom.index( row, 4 ) ).toDouble() );
    query.bindValue( ":xmax", modelFrom.data( modelFrom.index( row, 5 ) ).toDouble() );
    query.bindValue( ":ymax", modelFrom.data( modelFrom.index( row, 6 ) ).toDouble() );
    query.bindValue( ":projection_srid", modelFrom.data( modelFrom.index( row, 7 ) ).toInt() );

    if ( !query.exec() )
    {
      QgsDebugMsg( QString( "Could not move bookmark: %1" )
                   .arg( query.lastError().text() ) );
      return;
    }
    qgisModel->setSort( 0, Qt::AscendingOrder );
    qgisModel->select();
  }

  modelFrom.removeRow( row );
}
