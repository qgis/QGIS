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
#include "qgscrscache.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QModelIndex>
#include <QDoubleSpinBox>
#include <QAbstractTableModel>
#include <QToolButton>


const int QgsDoubleSpinBoxBookmarksDelegate::DECIMAL_PLACES = 6;

QgsBookmarks::QgsBookmarks( QWidget *parent )
    : QgsDockWidget( parent )
    , mQgisModel( nullptr )
    , mProjectModel( nullptr )
    , mMergedModel( nullptr )
    , mProxyModel( nullptr )
{
  setupUi( this );
  connect( lstBookmarks, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( lstBookmarks_doubleClicked( QModelIndex ) ) );
  restorePosition();

  bookmarksDockContents->layout()->setMargin( 0 );
  bookmarksDockContents->layout()->setContentsMargins( 0, 0, 0, 0 );
  static_cast< QGridLayout * >( bookmarksDockContents->layout() )->setVerticalSpacing( 0 );

  QToolButton *btnImpExp = new QToolButton;
  btnImpExp->setAutoRaise( true );
  btnImpExp->setToolTip( tr( "Import/Export Bookmarks" ) );
  btnImpExp->setIcon( QgsApplication::getThemeIcon( "/mActionSharing.svg" ) );
  btnImpExp->setPopupMode( QToolButton::InstantPopup );

  QMenu *share = new QMenu( this );
  QAction *btnExport = share->addAction( tr( "&Export" ) );
  QAction *btnImport = share->addAction( tr( "&Import" ) );
  btnExport->setIcon( QgsApplication::getThemeIcon( "/mActionSharingExport.svg" ) );
  btnImport->setIcon( QgsApplication::getThemeIcon( "/mActionSharingImport.svg" ) );

  connect( btnExport, SIGNAL( triggered() ), this, SLOT( exportToXml() ) );
  connect( btnImport, SIGNAL( triggered() ), this, SLOT( importFromXml() ) );
  btnImpExp->setMenu( share );


  connect( actionAdd, SIGNAL( triggered() ), this, SLOT( addClicked() ) );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteClicked() ) );
  connect( actionZoomTo, SIGNAL( triggered() ), this, SLOT( zoomToBookmark() ) );


  mBookmarkToolbar->addWidget( btnImpExp );

  // open the database
  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "bookmarks" );
  db.setDatabaseName( QgsApplication::qgisUserDbFilePath() );
  if ( ! db.open() )
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

  // Do not set parent or the destructor will try to write on the log viewer (and crash QGIS)
  mQgisModel = new QSqlTableModel( nullptr, db );
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

  mProjectModel = new QgsProjectBookmarksTableModel( this );
  mMergedModel = new QgsMergedBookmarksTableModel( *mQgisModel, *mProjectModel, lstBookmarks, this );

  mProxyModel = new QgsBookmarksProxyModel( );
  mProxyModel->setSourceModel( mMergedModel );
  mProxyModel->setSortCaseSensitivity( Qt::CaseInsensitive );

  lstBookmarks->setModel( mProxyModel );
  lstBookmarks->setItemDelegate( new QgsDoubleSpinBoxBookmarksDelegate( this ) );
  lstBookmarks->setSortingEnabled( true );
  lstBookmarks->sortByColumn( 1, Qt::AscendingOrder );

  connect( mMergedModel, SIGNAL( layoutChanged() ), mProxyModel, SLOT( _resetModel() ) );

  connect( mMergedModel, SIGNAL( &QgsMergedBookmarksTableModel::selectItem( const QModelIndex &index ) ), this, SLOT( scrollToIndex( const QModelIndex &index ) ) );

  QSettings settings;
  lstBookmarks->header()->restoreState( settings.value( "Windows/Bookmarks/headerstate" ).toByteArray() );

#ifndef QGISDEBUG
  lstBookmarks->setColumnHidden( 0, true );
#endif
}

QgsBookmarks::~QgsBookmarks()
{
  delete mQgisModel;
  delete mProxyModel;
  QSqlDatabase::removeDatabase( "bookmarks" );
  saveWindowLocation();
}

void QgsBookmarks::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "Windows/Bookmarks/geometry" ).toByteArray() );
}

void QgsBookmarks::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "Windows/Bookmarks/geometry" , saveGeometry() );
  settings.setValue( "Windows/Bookmarks/headerstate" , lstBookmarks->header()->saveState() );
}

void QgsBookmarks::addClicked()
{
  Q_ASSERT( mMergedModel );
  Q_ASSERT( mQgisModel );

  QgsMapCanvas *canvas = QgisApp::instance()->mapCanvas();
  Q_ASSERT( canvas );

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
      projStr = fi.exists() ? fi.fileName() : QLatin1String( "" );
    }
  }

  QSqlRecord record = mQgisModel->record();
  record.setValue( 1, QVariant( tr( "New bookmark" ) ) );
  record.setValue( 2, QVariant( projStr ) );
  record.setValue( 3, QVariant( canvas->extent().xMinimum() ) );
  record.setValue( 4, QVariant( canvas->extent().yMinimum() ) );
  record.setValue( 5, QVariant( canvas->extent().xMaximum() ) );
  record.setValue( 6, QVariant( canvas->extent().yMaximum() ) );
  record.setValue( 7, QVariant::fromValue( canvas->mapSettings().destinationCrs().srsid() ) );

  if ( mQgisModel->insertRecord( -1, record ) )
  {
    mQgisModel->setSort( 0, Qt::AscendingOrder );
    mQgisModel->select();
    QModelIndex newIdx = mProxyModel->mapFromSource( mMergedModel->index( mQgisModel->rowCount() - 1, 1 ) );
    // Edit new bookmark title
    lstBookmarks->scrollTo( newIdx );
    lstBookmarks->setCurrentIndex( newIdx );
    lstBookmarks->edit( newIdx );
  }
  else
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Unable to create the bookmark.\nDriver: %1\nDatabase: %2" )
                          .arg( mQgisModel->database().lastError().driverText(),
                                mQgisModel->database().lastError().databaseText() ) );
  }
}

void QgsBookmarks::deleteClicked()
{
  QItemSelection selection( mProxyModel->mapSelectionToSource( lstBookmarks->selectionModel()->selection() ) );
  std::vector<int> rows;
  Q_FOREACH ( const QModelIndex &idx, selection.indexes() )
  {
    if ( idx.column() == 1 )
    {
      rows.push_back( idx.row() );
    }
  }

  if ( rows.size() == 0 )
    return;

  // make sure the user really wants to delete these bookmarks
  if ( QMessageBox::Cancel == QMessageBox::information( this, tr( "Delete Bookmarks" ),
       tr( "Are you sure you want to delete %n bookmark(s)?", "number of rows", rows.size() ),
       QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  // Remove in reverse order to keep the merged model indexes
  std::sort( rows.begin(), rows.end(), std::greater<int>() );

  Q_FOREACH ( int row, rows )
  {
    mMergedModel->removeRow( row );
  }
  mProxyModel->_resetModel();
}

void QgsBookmarks::lstBookmarks_doubleClicked( const QModelIndex &index )
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
  QString authid = index.sibling( index.row(), 7 ).data().toString();

  QgsRectangle rect = QgsRectangle( xmin, ymin, xmax, ymax );

  if ( ! authid.isEmpty() &&
       authid != QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs().authid() )
  {
    QgsCoordinateTransform ct( QgsCRSCache::instance()->crsByOgcWmsCrs( authid ),
                               QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs() );
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
  QSettings settings;

  QString lastUsedDir = settings.value( "Windows/Bookmarks/LastUsedDirectory" , QDir::homePath() ).toString();
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

  Q_ASSERT( mMergedModel );

  QString queries;

  for ( int i = 0; i < nodeList.count(); i++ )
  {
    QDomNode bookmark = nodeList.at( i );
    QDomElement name = bookmark.firstChildElement( "name" );
    QDomElement prjname = bookmark.firstChildElement( "project" );
    QDomElement xmin = bookmark.firstChildElement( "xmin" );
    QDomElement ymin = bookmark.firstChildElement( "ymin" );
    QDomElement xmax = bookmark.firstChildElement( "xmax" );
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

  Q_FOREACH ( const QString &queryTxt, queriesList )
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
  mProxyModel->_resetModel();
}

void QgsBookmarks::exportToXml()
{
  QSettings settings;

  QString lastUsedDir = settings.value( "Windows/Bookmarks/LastUsedDirectory" , QDir::homePath() ).toString();
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

  QDomDocument doc( "qgis_bookmarks" );
  QDomElement root = doc.createElement( "qgis_bookmarks" );
  doc.appendChild( root );

  int rowCount = mMergedModel->rowCount();
  int colCount = mMergedModel->columnCount() - 1;  // exclude virtual "In project" column

  QList<QString> headerList;
  headerList
  <<  "id"
  <<  "name"
  <<  "project"
  <<  "xmin"
  <<  "ymin"
  <<  "xmax"
  <<  "ymax"
  <<  "sr_id";

  for ( int i = 0; i < rowCount; ++i )
  {
    QDomElement bookmark = doc.createElement( "bookmark" );
    root.appendChild( bookmark );
    for ( int j = 0; j < colCount; j++ )
    {
      QModelIndex idx = mMergedModel->index( i, j );
      if ( idx.isValid() )
      {
        QString value = idx.data( Qt::DisplayRole ).toString();
        QString header = headerList.at( j );
        // If it's the EPSG code, convert it to internal srid
        if ( header == "sr_id" )
        {
          QgsCoordinateReferenceSystem crs = QgsCRSCache::instance()->crsByOgcWmsCrs( value );
          if ( crs.isValid() )
            value = QString::number( crs.srsid() );
          else
            value = QString();
        }
        QDomText idText = doc.createTextNode( value );
        QDomElement id = doc.createElement( header );
        id.appendChild( idText );
        bookmark.appendChild( id );
      }
    }
  }

  QFile f( fileName );
  if ( !f.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    f.close();
    return;
  }

  QTextStream out( &f );
  out.setCodec( "UTF - 8" );
  doc.save( out, 2 );
  f.close();

  settings.setValue( "Windows/Bookmarks/LastUsedDirectory", QFileInfo( fileName ).path() );
}

void QgsBookmarks::scrollToIndex( const QModelIndex & index )
{
  QModelIndex proxyIndex( mProxyModel->mapFromSource( index ) );
  lstBookmarks->scrollTo( proxyIndex );
  lstBookmarks->setCurrentIndex( proxyIndex );
}

QgsProjectBookmarksTableModel::QgsProjectBookmarksTableModel( QObject *parent )
    : QAbstractTableModel( parent )
{
  QObject::connect(
    QgisApp::instance(), SIGNAL( projectRead() ),
    this, SLOT( projectRead() ) );
  QObject::connect(
    QgisApp::instance(), SIGNAL( newProject() ),
    this, SLOT( projectRead() ) );
}

int QgsProjectBookmarksTableModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return QgsProject::instance()->readNumEntry( "Bookmarks" ,  "/count" );
}

int QgsProjectBookmarksTableModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 8;
}


QVariant QgsProjectBookmarksTableModel::data( const QModelIndex& index, int role ) const
{
  if ( role != Qt::DisplayRole && role != Qt::EditRole )
  {
    return QVariant();
  }

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


bool QgsProjectBookmarksTableModel::insertRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent );
  Q_UNUSED( row );
  // append
  int oldCount = QgsProject::instance()->readNumEntry( "Bookmarks", "/count" );
  beginInsertRows( parent, oldCount, oldCount + count );
  bool result = QgsProject::instance()->writeEntry( "Bookmarks", "/count", oldCount + count );
  endInsertRows();
  return result;
}

bool QgsProjectBookmarksTableModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent );
  beginRemoveRows( parent, row, row + count );
  for ( int newRow = row ; newRow < rowCount() - count ; newRow++ )
  {
    for ( int column = 0; column < columnCount() ; column++ )
    {
      setData( index( newRow, column ), data( index( newRow + count, column ) ) );
    }
  }
  for ( int newRow = rowCount() - count ; newRow < rowCount() ; newRow++ )
  {
    QgsProject::instance()->removeEntry( "Bookmarks" , QString( "/Row-%1" ).arg( newRow ) );
  }

  QgsProject::instance()->writeEntry( "Bookmarks" , QString( "/count" ), QgsProject::instance()->readNumEntry( "Bookmarks" ,  "/count" ) - count );
  endRemoveRows();
  return true;
}

void QgsProjectBookmarksTableModel::projectRead()
{
  emit layoutChanged();
}


QgsMergedBookmarksTableModel::QgsMergedBookmarksTableModel( QAbstractTableModel &qgisTableModel, QAbstractTableModel &projectTableModel, QTreeView *treeView, QObject *parent )
    : QAbstractTableModel( parent )
    , mQgisTableModel( qgisTableModel )
    , mProjectTableModel( projectTableModel )
    , mTreeView( treeView )
{

  connect(
    &mQgisTableModel, SIGNAL( layoutChanged() ),
    this, SLOT( allLayoutChanged() ) );

  connect(
    &mQgisTableModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
    this, SLOT( allLayoutChanged() ) );

  connect(
    &mQgisTableModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
    this, SLOT( allLayoutChanged() ) );

  connect(
    &mProjectTableModel, SIGNAL( layoutChanged() ),
    this, SLOT( allLayoutChanged() ) );

  connect(
    &mProjectTableModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
    this, SLOT( allLayoutChanged() ) );

  connect(
    &mProjectTableModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
    this, SLOT( allLayoutChanged() ) );

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
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        value = mProjectTableModel.data( this->index( index.row() - mQgisTableModel.rowCount(), index.column() ), role );
      }
      else // Default roles from base model
      {
        value = mQgisTableModel.data( this->index( 0, index.column() ), role );
      }
    }
    // Is it the projection column ?
    if (( role == Qt::DisplayRole || role == Qt::EditRole ) && index.column( ) == mQgisTableModel.columnCount() - 1 )
    {
      QgsCoordinateReferenceSystem srs;
      if ( srs.createFromSrsId( value.toInt( ) ) )
        value = srs.authid( );
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
      // Move from SQLite storage to project
      moveBookmark( mQgisTableModel, mProjectTableModel, index.row() );
      emit selectItem( this->index( rowCount() - 1, 1 ) );
    }
    else
    {
      //Move from project to SQLite storage
      moveBookmark( mProjectTableModel, mQgisTableModel, index.row() - mQgisTableModel.rowCount() );
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
    QSqlTableModel *qgisModel = static_cast<QSqlTableModel *>( &mQgisTableModel );
    Q_ASSERT( qgisModel );
    result = qgisModel->removeRows( row, count, parent );
    qgisModel->select();
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

QAbstractTableModel *QgsMergedBookmarksTableModel::qgisModel()
{
  return &mQgisTableModel;
}

bool QgsMergedBookmarksTableModel::projectAvailable() const
{
  return ! QgsProject::instance()->fileName().isEmpty();
}

void QgsMergedBookmarksTableModel::moveBookmark( QAbstractTableModel &modelFrom, QAbstractTableModel &modelTo, int row )
{
  emit beginResetModel();
  QSqlTableModel *qgisModel = dynamic_cast<QSqlTableModel *>( &modelTo );
  if ( !qgisModel )
  {
    modelTo.insertRow( -1 );
    for ( int column = 1 ; column < modelFrom.columnCount() ; column++ )
    {
      Q_ASSERT( modelTo.index( modelTo.rowCount() - 1, column ).isValid( ) );
      modelTo.setData(
        modelTo.index( modelTo.rowCount() - 1, column ),
        modelFrom.data( modelFrom.index( row, column ) ) );
    }
    qgisModel = dynamic_cast<QSqlTableModel *>( &modelFrom );
    Q_ASSERT( qgisModel );
    qgisModel->removeRows( row, 1 );
    qgisModel->select();
  }
  else
  {
    QSqlRecord record = qgisModel->record();
    record.setValue( 1, modelFrom.data( modelFrom.index( row, 1 ) ).toString() );
    record.setValue( 2, modelFrom.data( modelFrom.index( row, 2 ) ).toString() );
    record.setValue( 3, modelFrom.data( modelFrom.index( row, 3 ) ).toDouble() );
    record.setValue( 4, modelFrom.data( modelFrom.index( row, 4 ) ).toDouble() );
    record.setValue( 5, modelFrom.data( modelFrom.index( row, 5 ) ).toDouble() );
    record.setValue( 6, modelFrom.data( modelFrom.index( row, 6 ) ).toDouble() );
    record.setValue( 7, modelFrom.data( modelFrom.index( row, 7 ) ).toInt() );

    if ( ! qgisModel->insertRecord( -1, record ) )
    {
      QgsDebugMsg( QString( "Could not move bookmark: %1" )
                   .arg( qgisModel->database().lastError().text() ) );
      return;
    }
    qgisModel->setSort( 0, Qt::AscendingOrder );
    qgisModel->select();
    modelFrom.removeRows( row, 1 );
  }
  emit endResetModel();
  emit layoutChanged();
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
