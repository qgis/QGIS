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
#include <qgscoordinatereferencesystem.h>

#include "qgslogger.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>

//standard includes
#include <cassert>
#include <fstream>

QgsBookmarks::QgsBookmarks( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    mParent( parent )
{
  setupUi( this );

  restorePosition();

  // user database is created at QGIS startup in QgisApp::createDB
  // we just check whether there is our database [MD]
  QFileInfo myFileInfo;
  myFileInfo.setFile( QgsApplication::qgisSettingsDirPath() );
  if ( !myFileInfo.exists( ) )
  {
    QgsDebugMsg( "The qgis.db does not exist" );
  }

  // Note proper queens english on next line
  initialise();

  //
  // Create the zoomto and delete buttons and add them to the
  // toolbar
  //
  QPushButton * btnUpdate = new QPushButton( tr( "&Update" ) );
  QPushButton * btnDelete = new QPushButton( tr( "&Delete" ) );
  QPushButton * btnZoomTo = new QPushButton( tr( "&Zoom to" ) );
  btnZoomTo->setDefault( true );
  //buttonBox->addButton( btnUpdate, QDialogButtonBox::ActionRole );
  buttonBox->addButton( btnDelete, QDialogButtonBox::ActionRole );
  buttonBox->addButton( btnZoomTo, QDialogButtonBox::ActionRole );
  // connect the slot up to catch when a bookmark is updated
  connect( btnUpdate, SIGNAL( clicked() ), this, SLOT( on_btnUpdate_clicked() ) );
  // connect the slot up to catch when a bookmark is deleted
  connect( btnDelete, SIGNAL( clicked() ), this, SLOT( on_btnDelete_clicked() ) );
  // connect the slot up to catch when a bookmark is zoomed to
  connect( btnZoomTo, SIGNAL( clicked() ), this, SLOT( on_btnZoomTo_clicked() ) );
  // connect the slot up to catch when a new bookmark is added
  connect( mParent, SIGNAL( bookmarkAdded() ), this, SLOT( refreshBookmarks() ) );
}

// Destructor
QgsBookmarks::~QgsBookmarks()
{
  db.close();
  QSqlDatabase::removeDatabase(CRSDB);
  saveWindowLocation();
}

void QgsBookmarks::refreshBookmarks()
{
  //lstBookmarks->clear();
  initialise();
}
// Initialise the bookmark tree from the database
void QgsBookmarks::initialise()
{
  this->connectDb();
  QSqlTableModel *model = new QSqlTableModel(0, db);
  model->setTable( "tbl_bookmarks" );
  // this should work, but then the table is not editable anymore:
  //    QSqlQuery::value: not positioned on a valid record
  // seems a qt bug
  //model->removeColumn(0);
  //model->removeColumn(6);
  model->select();
  lstBookmarks->setModel( model );
  lstBookmarks->setColumnHidden(0, true);
  lstBookmarks->setColumnHidden(7, true);
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
}

// REMOVE ?
void QgsBookmarks::on_btnUpdate_clicked()
{
  // get the current item
}

void QgsBookmarks::on_btnDelete_clicked()
{
  QModelIndex index = lstBookmarks->currentIndex();
  if ( index.isValid() )
  {
      // make sure the user really wants to delete this bookmark
      if ( QMessageBox::Ok == QMessageBox::information( this, tr( "Really Delete?" ),
           tr( "Are you sure you want to delete the '%1' bookmark?" ).arg(
               index.sibling(index.row(), 1).data().toString()  ),
           QMessageBox::Ok | QMessageBox::Cancel ) )
      {
        lstBookmarks->model()->removeRow(index.row());
      }
  }
}

void QgsBookmarks::on_btnZoomTo_clicked()
{
  //zoomToBookmark();

    QModelIndex index = lstBookmarks->currentIndex();
    if ( index.isValid() )
    {
        double xmin = index.sibling( index.row(), 3).data().toDouble();
        double ymin = index.sibling( index.row(), 4).data().toDouble();
        double xmax = index.sibling( index.row(), 5).data().toDouble();
        double ymax = index.sibling( index.row(), 6).data().toDouble();
        int srid = index.sibling( index.row(), 7).data().toInt();

        QgsRectangle rect = QgsRectangle( xmin, ymin, xmax, ymax );
        if (srid > 0) // backwards compatibility, older version had -1 in the srid column
        {
            // we could also check if those are the same ... but..
            QgsCoordinateReferenceSystem srcMapcanvas = QgisApp::instance()->mapCanvas()->mapRenderer()->destinationCrs();
            QgsCoordinateReferenceSystem srsSource;
            //srsSource.createFromId( srid );  // gebruikt 2517 en wordt dan gauss
            srsSource.createFromSrsId( srid);
            QgsCoordinateTransform * coordTransform =  new QgsCoordinateTransform( srsSource, srcMapcanvas );
            rect = coordTransform->transform( rect );
            delete coordTransform;
        }

        // set the extent to the bookmark
        QgisApp::instance()->setExtent( rect );
        // redraw the map
        QgisApp::instance()->mapCanvas()->refresh();\
    }
}

// REMOVE?
/*void QgsBookmarks::on_lstBookmarks_itemDoubleClicked( QTreeWidgetItem *lvi )
{
  Q_UNUSED( lvi );
  zoomToBookmark();
}*/

// REMOVE?
void QgsBookmarks::zoomToBookmark()
{
  // Need to fetch the extent for the selected bookmark and then redraw
  // the map
  // get the current item
}

int QgsBookmarks::connectDb()
{
    if ( QSqlDatabase::contains( CRSDB ) && db.open()){
       QgsDebugMsg( QString( "Successfully returned open database: %1" ).arg( QgsApplication::qgisUserDbFilePath().toUtf8().data() ) );
    }
    else {
        db = QSqlDatabase::addDatabase("QSQLITE", CRSDB);
        db.setDatabaseName( QgsApplication::qgisUserDbFilePath().toUtf8().data() );
        if (db.open()) {
            QgsDebugMsg( QString( "Successfully opened database: %1" ).arg( QgsApplication::qgisUserDbFilePath().toUtf8().data() ) );
        }
        else {
            QgsDebugMsg( QString( "Can't open database: %1" ).arg( QgsApplication::qgisUserDbFilePath().toUtf8().data() ) );
            return 0;
        }
    }
    return 1;
}
