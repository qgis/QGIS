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
#include "qgsbookmarks.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"

#include "qgslogger.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>

QgsBookmarks *QgsBookmarks::sInstance = 0;

QgsBookmarks::QgsBookmarks( QWidget *parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  restorePosition();

  //
  // Create the zoomto and delete buttons and add them to the
  // toolbar
  //
  QPushButton *btnAdd    = new QPushButton( tr( "&Add" ) );
  QPushButton *btnDelete = new QPushButton( tr( "&Delete" ) );
  QPushButton *btnZoomTo = new QPushButton( tr( "&Zoom to" ) );
  QPushButton *btnImpExp = new QPushButton( tr( "&Share" ) );

  btnZoomTo->setDefault( true );
  buttonBox->addButton( btnAdd, QDialogButtonBox::ActionRole );
  buttonBox->addButton( btnDelete, QDialogButtonBox::ActionRole );
  buttonBox->addButton( btnZoomTo, QDialogButtonBox::ActionRole );
  buttonBox->addButton( btnImpExp, QDialogButtonBox::ActionRole );

  QMenu *share = new QMenu();
  QAction *btnExport = share->addAction( tr( "&Export" ) );
  QAction *btnImport = share->addAction( tr( "&Import" ) );
  connect( btnExport, SIGNAL( triggered() ), this, SLOT( exportToXML() ) );
  connect( btnImport, SIGNAL( triggered() ), this, SLOT( importFromXML() ) );
  btnImpExp->setMenu( share );

  connect( btnAdd, SIGNAL( clicked() ), this, SLOT( addClicked() ) );
  connect( btnDelete, SIGNAL( clicked() ), this, SLOT( deleteClicked() ) );
  connect( btnZoomTo, SIGNAL( clicked() ), this, SLOT( zoomToBookmark() ) );

  // open the database
  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "bookmarks" );
  db.setDatabaseName( QgsApplication::qgisUserDbFilePath() );
  if ( !db.open() )
  {
    QMessageBox::warning( this, tr( "Error" ),
                          tr( "Unable to open bookmarks database.\nDatabase: %1\nDriver: %2\nDatabase: %3" )
                          .arg( QgsApplication::qgisUserDbFilePath() )
                          .arg( db.lastError().driverText() )
                          .arg( db.lastError().databaseText() )
                        );
    deleteLater();
    return;
  }

  QSqlTableModel *model = new QSqlTableModel( this, db );
  model->setTable( "tbl_bookmarks" );
  model->setSort( 0, Qt::AscendingOrder );
  model->setEditStrategy( QSqlTableModel::OnFieldChange );
  model->select();

  // set better headers then column names from table
  model->setHeaderData( 0, Qt::Horizontal, tr( "ID" ) );
  model->setHeaderData( 1, Qt::Horizontal, tr( "Name" ) );
  model->setHeaderData( 2, Qt::Horizontal, tr( "Project" ) );
  model->setHeaderData( 3, Qt::Horizontal, tr( "xMin" ) );
  model->setHeaderData( 4, Qt::Horizontal, tr( "yMin" ) );
  model->setHeaderData( 5, Qt::Horizontal, tr( "xMax" ) );
  model->setHeaderData( 6, Qt::Horizontal, tr( "yMax" ) );
  model->setHeaderData( 7, Qt::Horizontal, tr( "SRID" ) );

  lstBookmarks->setModel( model );

  QSettings settings;
  lstBookmarks->header()->restoreState( settings.value( "/Windows/Bookmarks/headerstate" ).toByteArray() );

#ifndef QGISDEBUG
  lstBookmarks->setColumnHidden( 0, true );
#endif
}

QgsBookmarks::~QgsBookmarks()
{
  saveWindowLocation();
  sInstance = 0;
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

void QgsBookmarks::newBookmark()
{
  showBookmarks();
  sInstance->addClicked();
}

void QgsBookmarks::showBookmarks()
{
  if ( !sInstance )
  {
    sInstance = new QgsBookmarks( QgisApp::instance() );
    sInstance->setAttribute( Qt::WA_DeleteOnClose );
  }

  sInstance->show();
  sInstance->raise();
  sInstance->setWindowState( sInstance->windowState() & ~Qt::WindowMinimized );
  sInstance->activateWindow();
}

void QgsBookmarks::addClicked()
{
  QSqlTableModel *model = qobject_cast<QSqlTableModel *>( lstBookmarks->model() );
  Q_ASSERT( model );

  QgsMapCanvas *canvas = QgisApp::instance()->mapCanvas();
  Q_ASSERT( canvas );

  QSqlQuery query( "INSERT INTO tbl_bookmarks(bookmark_id,name,project_name,xmin,ymin,xmax,ymax,projection_srid)"
                   "  VALUES (NULL,:name,:project_name,:xmin,:xmax,:ymin,:ymax,:projection_srid)",
                   model->database() );

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
    model->setSort( 0, Qt::AscendingOrder );
    model->select();
    lstBookmarks->scrollToBottom();
    lstBookmarks->setCurrentIndex( model->index( model->rowCount() - 1, 1 ) );
    lstBookmarks->edit( model->index( model->rowCount() - 1, 1 ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Unable to create the bookmark.\nDriver:%1\nDatabase:%2" )
                          .arg( query.lastError().driverText() )
                          .arg( query.lastError().databaseText() ) );
  }
}

void QgsBookmarks::deleteClicked()
{
  QList<int> rows;
  foreach ( const QModelIndex &idx, lstBookmarks->selectionModel()->selectedIndexes() )
  {
    if ( idx.column() == 1 )
    {
      rows << idx.row();
    }
  }

  if ( rows.size() == 0 )
    return;

  // make sure the user really wants to delete these bookmarks
  if ( QMessageBox::Cancel == QMessageBox::information( this, tr( "Really Delete?" ),
       tr( "Are you sure you want to delete %n bookmark(s)?", "number of rows", rows.size() ),
       QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  int i = 0;
  foreach ( int row, rows )
  {
    lstBookmarks->model()->removeRow( row - i );
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
    QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( srid, QgsCoordinateReferenceSystem::InternalCrsId ),
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

  QString lastUsedDir = settings.value( "/Bookmark/LastUsedDirectory", QVariant() ).toString();
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

  QSqlTableModel *model = qobject_cast<QSqlTableModel *>( lstBookmarks->model() );
  Q_ASSERT( model );

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
               + xmin.text() + ","
               + ymin.text() + ","
               + xmax.text() + ","
               + ymax.text() + ","
               + srid.text() + ");";
  }

  QStringList queriesList = queries.split( ";" );
  QSqlQuery query( model->database() );

  foreach ( QString queryTxt, queriesList )
  {
    if ( queryTxt.trimmed().isEmpty() )
    {
      continue;
    }
    if ( !query.exec( queryTxt ) )
    {
      QMessageBox::warning( this, tr( "Error" ), tr( "Unable to create the bookmark.\nDriver: %1\nDatabase: %2" )
                            .arg( query.lastError().driverText() )
                            .arg( query.lastError().databaseText() ) );
    }
    query.finish();
  }
  model->setSort( 0, Qt::AscendingOrder );
  model->select();
}

void QgsBookmarks::exportToXML()
{
  QSettings settings;

  QString lastUsedDir = settings.value( "/Bookmark/LastUsedDirectory", QVariant() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Export bookmarks" ), lastUsedDir,
                     tr( "XML files( *.xml *.XML )" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never ommited the extension from the file name
  if ( !fileName.toLower().endsWith( ".xml" ) )
  {
    fileName += ".xml";
  }

  QDomDocument doc( "qgis_bookmarks" );
  QDomElement root = doc.createElement( "qgis_bookmarks" );
  doc.appendChild( root );

  int rowCount = lstBookmarks->model()->rowCount();
  int colCount = lstBookmarks->model()->columnCount();

  QList<QString> headerList;
  headerList << "id" << "name" << "project" << "xmin"
  << "ymin" << "xmax" << "ymax" << "sr_id";

  for ( int i = 0; i < rowCount; ++i )
  {
    QDomElement bookmark = doc.createElement( "bookmark" );
    root.appendChild( bookmark );
    for ( int j = 0; j < colCount; j++ )
    {
      QModelIndex idx = lstBookmarks->model()->index( i, j );
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

  settings.setValue( "/Bookmark/LastUsedDirectory", QFileInfo( fileName ).path() );
}
