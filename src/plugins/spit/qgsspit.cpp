/***************************************************************************
                         qgsspit.cpp  -  description
                            -------------------
   begin                : Fri Dec 19 2003
   copyright            : (C) 2003 by Denis Antipov
   email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <QMessageBox>
#include <QComboBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QRegExp>
#include <QFile>
#include <QSettings>
#include <QPixmap>
#include <QHeaderView>
#include <QTextCodec>
#include <QList>
#include <QTableWidgetItem>
#include <QInputDialog>

#include <iostream>

#include "qgsencodingfiledialog.h"

#include "qgspgutil.h"
#include "qgsspit.h"
#include "qgspgnewconnection.h"
#include "qgsdatasourceuri.h"
#include "qgsmessageviewer.h"
#include "qgslogger.h"

// Qt implementation of alignment() + changed the numeric types to be shown on the left as well
/* Is this still needed? Numbers in Qt4 table seem to be left justified by default.
int Q3TableItem::alignment() const
{
  bool num;
  bool ok1 = FALSE, ok2 = FALSE;
  ( void ) txt.toInt( &ok1 );
  if ( !ok1 )
    ( void ) txt.toDouble( &ok2 );
  num = ok1 || ok2;

  return ( num ? Qt::AlignLeft : Qt::AlignLeft ) | Qt::AlignVCenter;
}
*/

QgsSpit::QgsSpit( QWidget *parent, Qt::WFlags fl ) : QDialog( parent, fl )
{
  setupUi( this );

  // Set up the table column headers
  tblShapefiles->setColumnCount( 5 );
  QStringList headerText;
  headerText << tr( "File Name" ) << tr( "Feature Class" ) << tr( "Features" )
  << tr( "DB Relation Name" ) << tr( "Schema" );
  tblShapefiles->setHorizontalHeaderLabels( headerText );
  tblShapefiles->verticalHeader()->hide();
  tblShapefiles->horizontalHeader()->setStretchLastSection( true );

  populateConnectionList();
  defSrid = -1;
  defGeom = "the_geom";
  total_features = 0;

  chkUseDefaultSrid->setChecked( true );
  chkUseDefaultGeom->setChecked( true );
  useDefaultSrid();
  useDefaultGeom();

  txtPrimaryKeyName->setText( "gid" );

  schema_list << "public";
  conn = NULL;

  // Install a delegate that provides the combo box widget for
  // changing the schema (but there can only be one delegate per
  // table, so it also provides edit widgets for the textual columns).
  // This needs to be done after the call to getSchema() so that
  // schema_list is populated.
  ShapefileTableDelegate* delegate = new ShapefileTableDelegate( tblShapefiles, schema_list );
  tblShapefiles->setItemDelegate( delegate );

  // Now that everything is in the table, adjust the column sizes
  tblShapefiles->resizeColumnsToContents();
}

QgsSpit::~QgsSpit()
{
  if ( conn )
    PQfinish( conn );
}

void QgsSpit::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/PostgreSQL/connections" );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->addItem( *it );
    ++it;
  }
  settings.endGroup();

  btnConnect->setDisabled( cmbConnections->count() == 0 );
  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnRemove->setDisabled( cmbConnections->count() == 0 );

  cmbConnections->setDisabled( cmbConnections->count() == 0 );
}

void QgsSpit::newConnection()
{
  QgsPgNewConnection *nc = new QgsPgNewConnection( this );
  nc->exec();
  delete nc;

  populateConnectionList();
}

void QgsSpit::editConnection()
{
  QgsPgNewConnection *nc = new QgsPgNewConnection( this, cmbConnections->currentText() );
  nc->exec();
  delete nc;

  populateConnectionList();
}

void QgsSpit::removeConnection()
{
  QSettings settings;
  QString key = "/PostgreSQL/connections/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the [%1] connection and all associated settings?" ).arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result != QMessageBox::Ok )
    return;

  settings.remove( key + "/host" );
  settings.remove( key + "/database" );
  settings.remove( key + "/port" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/sslmode" );
  settings.remove( key + "/publicOnly" );
  settings.remove( key + "/geometryColumnsOnly" );
  settings.remove( key + "/save" );
  settings.remove( key );

  populateConnectionList();
}

void QgsSpit::addFile()
{
  QString error1 = "";
  QString error2 = "";
  bool exist;
  bool is_error = false;
  QSettings settings;

  QgsEncodingFileDialog dlg( this,
                             tr( "Add Shapefiles" ),
                             settings.value( "/Plugin-Spit/last_directory" ).toString(),
                             tr( "Shapefiles (*.shp);;All files (*.*)" ),
                             settings.value( "/Plugin-Spit/last_encoding" ).toString() );
  dlg.setFileMode( QFileDialog::ExistingFiles );

  if ( dlg.exec() != QDialog::Accepted )
    return;
  QStringList files = dlg.selectedFiles();

  if ( files.size() > 0 )
  {
    // Save the directory for future use
    QFileInfo fi( files[ 0 ] );
    settings.setValue( "/Plugin-Spit/last_directory", fi.absolutePath() );
    settings.setValue( "/Plugin-Spit/last_encoding", dlg.encoding() );
  }
  // Process the files
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
  {
    exist = false;
    is_error = false;

    // Check to ensure that we don't insert the same file twice
    QList<QTableWidgetItem*> items = tblShapefiles->findItems( *it,
                                     Qt::MatchExactly );
    if ( items.count() > 0 )
    {
      exist = true;
    }

    if ( !exist )
    {
      // check other files: file.dbf and file.shx
      QString name = *it;
      if ( !QFile::exists( name.left( name.length() - 3 ) + "dbf" ) )
      {
        is_error = true;
      }
      else if ( !QFile::exists( name.left( name.length() - 3 ) + "shx" ) )
      {
        is_error = true;
      }

      if ( !is_error )
      {
        QgsShapeFile * file = new QgsShapeFile( name, dlg.encoding() );
        if ( file->is_valid() )
        {
          /* XXX getFeatureClass actually does a whole bunch
           * of things and is probably better named
           * something else
           */
          QString featureClass = file->getFeatureClass();
          fileList.push_back( file );

          QTableWidgetItem *fileNameItem       = new QTableWidgetItem( name );
          QTableWidgetItem *featureClassItem   = new QTableWidgetItem( featureClass );
          QTableWidgetItem *featureCountItem   = new QTableWidgetItem( QString( "%1" ).arg( file->getFeatureCount() ) );
          // Sanitize the relation name to make it pg friendly
          QString relName = file->getTable().replace( QRegExp( "\\s" ), "_" );
          QTableWidgetItem *dbRelationNameItem = new QTableWidgetItem( relName );
          QTableWidgetItem *dbSchemaNameItem   = new QTableWidgetItem( cmbSchema->currentText() );

          // All items are editable except for these two
          fileNameItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
          featureCountItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

          int row = tblShapefiles->rowCount();
          tblShapefiles->insertRow( row );
          tblShapefiles->setItem( row, ColFILENAME, fileNameItem );
          tblShapefiles->setItem( row, ColFEATURECLASS, featureClassItem );
          tblShapefiles->setItem( row, ColFEATURECOUNT, featureCountItem );
          tblShapefiles->setItem( row, ColDBRELATIONNAME, dbRelationNameItem );
          tblShapefiles->setItem( row, ColDBSCHEMA, dbSchemaNameItem );

          total_features += file->getFeatureCount();
        }
        else
        {
          error1 += name + "\n";
          is_error = true;
          delete file;
        }
      }
      else
      {
        error2 += name + "\n";
      }
    }
  }

  tblShapefiles->resizeColumnsToContents();

  if ( error1 != "" || error2 != "" )
  {
    QString message = tr( "The following Shapefile(s) could not be loaded:\n\n" );
    if ( error1 != "" )
    {
      error1 += "----------------------------------------------------------------------------------------";
      error1 += "\n" + tr( "REASON: File cannot be opened" ) + "\n\n";
    }
    if ( error2 != "" )
    {
      error2 += "----------------------------------------------------------------------------------------";
      error2 += "\n" + tr( "REASON: One or both of the Shapefile files (*.dbf, *.shx) missing" ) + "\n\n";
    }
    QgsMessageViewer * e = new QgsMessageViewer( this );
    e->setMessageAsPlainText( message + error1 + error2 );
    e->exec(); // deletes itself on close
  }
}

void QgsSpit::removeFile()
{
  std::vector <int> temp;
  for ( int n = 0; n < tblShapefiles->rowCount(); n++ )
    if ( tblShapefiles->isItemSelected( tblShapefiles->item( n, 0 ) ) )
    {
      for ( std::vector<QgsShapeFile *>::iterator vit = fileList.begin(); vit != fileList.end(); vit++ )
      {
        if (( *vit ) ->getName() == tblShapefiles->item( n, 0 )->text() )
        {
          total_features -= ( *vit ) ->getFeatureCount();
          fileList.erase( vit );
          temp.push_back( n );
          break;
        }
      }
    }

  for ( int i = temp.size() - 1; i >= 0; --i )
    tblShapefiles->removeRow( temp[ i ] );

  QList<QTableWidgetItem*> selected = tblShapefiles->selectedItems();
  for ( int i = 0; i < selected.count(); ++i )
    selected[i]->setSelected( false );
}

void QgsSpit::removeAllFiles()
{
  int i = tblShapefiles->rowCount() - 1;
  for ( ; i >= 0; --i )
    tblShapefiles->removeRow( i );

  fileList.clear();
  total_features = 0;
}

void QgsSpit::useDefaultSrid()
{
  if ( chkUseDefaultSrid->isChecked() )
  {
    defaultSridValue = spinSrid->value();
    spinSrid->setValue( defSrid );
    spinSrid->setEnabled( false );
  }
  else
  {
    spinSrid->setEnabled( true );
    spinSrid->setValue( defaultSridValue );
  }
}

void QgsSpit::useDefaultGeom()
{
  if ( chkUseDefaultGeom->isChecked() )
  {
    defaultGeomValue = txtGeomName->text();
    txtGeomName->setText( defGeom );
    txtGeomName->setEnabled( false );
  }
  else
  {
    txtGeomName->setEnabled( true );
    txtGeomName->setText( defaultGeomValue );
  }
}

// TODO: make translation of helpinfo
void QgsSpit::helpInfo()
{
  QString message = tr( "General Interface Help:" ) + "\n\n";
  message += tr( "PostgreSQL Connections:" ) + "\n"
             + "----------------------------------------------------------------------------------------\n"
             + tr( "[New ...] - create a new connection" ) + "\n"
             + tr( "[Edit ...] - edit the currently selected connection" ) + "\n"
             + tr( "[Remove] - remove the currently selected connection" ) + "\n"
             + tr( "-you need to select a connection that works (connects properly) in order to import files" ) + "\n"
             + tr( "-when changing connections Global Schema also changes accordingly" ) + "\n\n"
             + tr( "Shapefile List:" ) + "\n"
             + "----------------------------------------------------------------------------------------\n"
             + tr( "[Add ...] - open a File dialog and browse to the desired file(s) to import" ) + "\n"
             + tr( "[Remove] - remove the currently selected file(s) from the list" ) + "\n"
             + tr( "[Remove All] - remove all the files in the list" ) + "\n"
             + tr( "[SRID] - Reference ID for the shapefiles to be imported" ) + "\n"
             + tr( "[Use Default (SRID)] - set SRID to -1" ) + "\n"
             + tr( "[Geometry Column Name] - name of the geometry column in the database" ) + "\n"
             + tr( "[Use Default (Geometry Column Name)] - set column name to \'the_geom\'" ) + "\n"
             + tr( "[Global Schema] - set the schema for all files to be imported into" ) + "\n\n"
             + "----------------------------------------------------------------------------------------\n"
             + tr( "[Import] - import the current shapefiles in the list" ) + "\n"
             + tr( "[Quit] - quit the program\n" )
             + tr( "[Help] - display this help dialog" ) + "\n\n";
  QgsMessageViewer * e = new QgsMessageViewer( this );
  e->setMessageAsPlainText( message );
  e->exec(); // deletes itself on close
}

void QgsSpit::dbConnect()
{
  if ( conn )
  {
    PQfinish( conn );
    conn = NULL;
  }

  QSettings settings;
  QString connName = cmbConnections->currentText();
  if ( connName.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Import Shapefiles" ), tr( "You need to specify a Connection first" ) );
    return;
  }

  QString key = "/PostgreSQL/connections/" + connName;
  QString database = settings.value( key + "/database" ).toString();
  QString username = settings.value( key + "/username" ).toString();
  QString password = settings.value( key + "/password" ).toString();

  bool makeConnection = true;

  if ( password.isEmpty() )
  {
    // get password from user
    password = QInputDialog::getText( this, tr( "Password for %1" ).arg( username ),
                                      tr( "Please enter your password:" ),
                                      QLineEdit::Password, QString::null, &makeConnection );
  }

  if ( makeConnection )
  {
    // allow null password entry in case its valid for the database
    QgsDataSourceURI uri;
    uri.setConnection( settings.value( key + "/host" ).toString(),
                       settings.value( key + "/port" ).toString(),
                       database,
                       settings.value( key + "/username" ).toString(),
                       password,
                       ( QgsDataSourceURI::SSLmode ) settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt() );

    conn = PQconnectdb( uri.connectionInfo().toUtf8() );
  }

  if ( conn == NULL || PQstatus( conn ) != CONNECTION_OK )
  {
    QMessageBox::warning( this, tr( "Import Shapefiles" ), tr( "Connection failed - Check settings and try again" ) );
    if ( conn )
    {
      PQfinish( conn );
      conn = 0;
    }
  }

  schema_list.clear();
  schema_list << "public";

  if ( conn )
  {
    int errcode = PQsetClientEncoding( conn, QString( "UNICODE" ).toLocal8Bit() );
    if ( errcode == 0 )
    {
      QgsDebugMsg( "encoding successfully set" );
    }
    else if ( errcode == -1 )
    {
      QgsDebugMsg( "error in setting encoding" );
    }
    else
    {
      QgsDebugMsg( "undefined return value from encoding setting" );
    }

    // Check that the database actually has postgis in it.
    QString sql1 = "SELECT postgis_lib_version()"; // available from v 0.9.0 onwards
    QString sql2 = "SELECT postgis_version()"; // depreciated

    PGresult* ver = PQexec( conn, sql1.toUtf8() );
    if ( PQresultStatus( ver ) != PGRES_TUPLES_OK )
    {
      // In case the version of postgis is older than 0.9.0, try the
      // depreciated call before erroring out.
      PQclear( ver );
      ver = PQexec( conn, sql2.toUtf8() );
      if ( PQresultStatus( ver ) != PGRES_TUPLES_OK )
      {
        QMessageBox::warning( this, tr( "PostGIS not available" ),
                              tr( "<p>The chosen database does not have PostGIS installed, "
                                  "but this is required for storage of spatial data.</p>" ) );
      }
    }

    QString schemaSql = "select nspname from pg_namespace where has_schema_privilege(nspname, 'CREATE')";
    PGresult *schemas = PQexec( conn, schemaSql.toUtf8() );
    // get the schema names
    if ( PQresultStatus( schemas ) == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < PQntuples( schemas ); i++ )
      {
        if ( QString( PQgetvalue( schemas, i, 0 ) ) != "public" )
          schema_list << QString( PQgetvalue( schemas, i, 0 ) );
      }
    }
    PQclear( schemas );
  }

  // install a new delegate with an updated schema list (rather than
  // update the existing delegate because delegates don't seem able to
  // store modifiable data).
  ShapefileTableDelegate* delegate = new ShapefileTableDelegate( tblShapefiles, schema_list );
  tblShapefiles->setItemDelegate( delegate );

  cmbSchema->clear();
  cmbSchema->insertItems( 0, schema_list );
  cmbSchema->setCurrentIndex( 0 ); // index 0 is always "public"
}

void QgsSpit::import()
{
  QList<QTableWidgetItem*> selected = tblShapefiles->selectedItems();
  for ( int i = 0; i < selected.count(); ++i )
    selected[i]->setSelected( false );

  QString connName = cmbConnections->currentText();
  QSettings settings;
  bool canceled = false;

  QString query;
  if ( total_features == 0 )
  {
    QMessageBox::warning( this, tr( "Import Shapefiles" ),
                          tr( "You need to add shapefiles to the list first" ) );
  }
  else if ( conn != NULL )
  {
    PGresult * res;
    QProgressDialog pro( tr( "Importing files" ), tr( "Cancel" ),
                         0, total_features, this );

    pro.setValue( 0 );
    pro.setLabelText( tr( "Progress" ) );
    pro.setAutoClose( true );
    //pro->show();
    qApp->processEvents();

    int count = fileList.size(), successes = 0;

    for ( std::vector<QgsShapeFile*>::size_type i = 0; i < fileList.size() ; i++ )
    {
      QString error = tr( "Problem inserting features from file:" ) + "\n" +
                      tblShapefiles->item( i, ColFILENAME )->text();

      // if a name starts with invalid character
      if ( ! tblShapefiles->item( i, ColDBRELATIONNAME )->text()[ 0 ].isLetter() )
      {
        QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                              tr( "%1\nInvalid table name." ).arg( error ) );
        pro.setValue( pro.value()
                      + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
        continue;
      }

      // if no fields detected
      if (( fileList[ i ] ->column_names ).size() == 0 )
      {
        QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                              tr( "%1\nNo fields detected." ).arg( error ) );
        pro.setValue( pro.value() +
                      tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
        continue;
      }

      // duplicate field check
      std::vector<QString> names_copy = fileList[ i ] ->column_names;
      names_copy.push_back( txtPrimaryKeyName->text() );
      names_copy.push_back( txtGeomName->text() );

      QString dupl = "";
      std::sort( names_copy.begin(), names_copy.end() );

      for ( std::vector<QString>::size_type k = 1; k < names_copy.size(); k++ )
      {
        QgsDebugMsg( QString( "Checking to see if %1 == %2" ).arg( names_copy[ k ] ).arg( names_copy[ k - 1 ] ) );
        if ( names_copy[ k ] == names_copy[ k - 1 ] )
          dupl += names_copy[ k ] + "\n";
      }

      // if duplicate field names exist
      if ( dupl != "" )
      {
        QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                              tr( "%1\nThe following fields are duplicates:\n%2" )
                              .arg( error ).arg( dupl ) );
        pro.setValue( pro.value() + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
        continue;
      }

      // Check and set destination table
      fileList[ i ] ->setTable( tblShapefiles->item( i, ColDBRELATIONNAME )->text() );
      pro.setLabelText( tr( "Importing files\n%1" ).arg( tblShapefiles->item( i, ColFILENAME )->text() ) );
      bool rel_exists1 = false;
      bool rel_exists2 = false;
      query = QString( "SELECT f_table_name FROM geometry_columns WHERE f_table_name=%1 AND f_table_schema=%2" )
              .arg( QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBRELATIONNAME )->text() ) )
              .arg( QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBSCHEMA )->text() ) );
      res = PQexec( conn, query.toUtf8() );
      rel_exists1 = ( PQntuples( res ) > 0 );

      if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
      {
        QString err = PQresultErrorMessage( res );
        QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                              tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                              .arg( error ).arg( query ).arg( err ) );
        pro.setValue( pro.value() + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
        PQclear( res );
        continue;
      }

      PQclear( res );

      query = QString( "SELECT tablename FROM pg_tables WHERE tablename=%1  AND schemaname=%2" )
              .arg( QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBRELATIONNAME )->text() ) )
              .arg( QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBSCHEMA )->text() ) );
      res = PQexec( conn, query.toUtf8() );

      rel_exists2 = ( PQntuples( res ) > 0 );

      if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
      {
        QString err = PQresultErrorMessage( res );
        QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                              tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                              .arg( error ).arg( query ).arg( err ) );

        pro.setValue( pro.value() + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
        PQclear( res );
        continue;
      }
      else
      {
        PQclear( res );
      }

      // begin session
      query = "BEGIN";
      res = PQexec( conn, query.toUtf8() );
      if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
      {
        QString err = PQresultErrorMessage( res );
        QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                              tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                              .arg( error ).arg( query ).arg( err ) );

        pro.setValue( pro.value() + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
        PQclear( res );
        continue;
      }
      else
      {
        PQclear( res );
      }

      query = "SET SEARCH_PATH TO ";
      if ( !tblShapefiles->item( i, ColDBSCHEMA )->text().isEmpty() &&
           tblShapefiles->item( i, ColDBSCHEMA )->text() != "public" )
        query += QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBSCHEMA )->text() ) + ",";
      query += QgsPgUtil::quotedValue( "public" );
      res = PQexec( conn, query.toUtf8() );

      if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
      {
        QString err = PQresultErrorMessage( res );
        QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                              error + "\n" +
                              tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                              .arg( error ).arg( query ).arg( err ) );

        pro.setValue( pro.value() + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
        PQclear( res );
        continue;
      }
      else
      {
        PQclear( res );
      }

      if ( rel_exists1 || rel_exists2 )
      {
        QMessageBox::StandardButton del_confirm = QMessageBox::warning( this,
            tr( "Import Shapefiles - Relation Exists" ),
            tr( "The Shapefile:\n%1\nwill use [%2] relation for its data,\n"
                "which already exists and possibly contains data.\n"
                "To avoid data loss change the \"DB Relation Name\"\n"
                "for this Shapefile in the main dialog file list.\n\n"
                "Do you want to overwrite the [%2] relation?" )
            .arg( tblShapefiles->item( i, 0 )->text() )
            .arg( tblShapefiles->item( i, ColDBRELATIONNAME )->text() ),
            QMessageBox::Ok | QMessageBox::Cancel );

        if ( del_confirm == QMessageBox::Ok )
        {
          if ( rel_exists2 )
          {
            query = QString( "DROP TABLE %1" )
                    .arg( QgsPgUtil::quotedIdentifier( tblShapefiles->item( i, ColDBRELATIONNAME )->text() ) );

            res = PQexec( conn, query.toUtf8() );
            if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
            {
              QString err = PQresultErrorMessage( res );
              QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                                    tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                                    .arg( error ).arg( query ).arg( err ) );
              pro.setValue( pro.value() + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
              PQclear( res );
              continue;
            }
            else
            {
              PQclear( res );
            }
          }

          if ( rel_exists1 )
          {
            query = QString( "SELECT f_geometry_column FROM geometry_columns WHERE f_table_schema=%1 AND f_table_name=%2" )
                    .arg( QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBSCHEMA )->text() ) )
                    .arg( QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBRELATIONNAME )->text() ) );

            QStringList columns;
            res = PQexec( conn, query.toUtf8() );
            if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
            {
              for ( int i = 0; i < PQntuples( res ); i++ )
                columns.append( PQgetvalue( res, i, 0 ) );
            }
            PQclear( res );

            for ( int i = 0; i < columns.size(); i++ )
            {
              query = QString( "SELECT DropGeometryColumn(%1,%2,%3)" )
                      .arg( QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBSCHEMA )->text() ) )
                      .arg( QgsPgUtil::quotedValue( tblShapefiles->item( i, ColDBRELATIONNAME )->text() ) )
                      .arg( QgsPgUtil::quotedValue( columns[i] ) );

              res = PQexec( conn, query.toUtf8() );
              if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
              {
                QString err = PQresultErrorMessage( res );
                QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                                      tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                                      .arg( error ).arg( query ).arg( err ) );
                pro.setValue( pro.value() + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
              }

              PQclear( res );
            }
          }
        }
        else
        {
          query = "ROLLBACK";
          res = PQexec( conn, query.toUtf8() );
          if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          {
            QString err = PQresultErrorMessage( res );
            QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                                  tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                                  .arg( error ).arg( query ).arg( err ) );
          }
          PQclear( res );

          pro.setValue( pro.value() + tblShapefiles->item( i, 2 )->text().toInt() );
          continue;
        }
      }

      // importing file here
      int temp_progress = pro.value();
      canceled = false;

      QString key = "/PostgreSQL/connections/" + connName;
      QString dbname = settings.value( key + "/database" ).toString();
      QString schema = tblShapefiles->item( i, ColDBSCHEMA )->text();
      QString srid = QString( "%1" ).arg( spinSrid->value() );
      QString errorText;

      bool layerInserted = fileList[i]->insertLayer( dbname, schema, txtPrimaryKeyName->text(), txtGeomName->text(), srid, conn, pro, canceled, errorText );
      if ( layerInserted && !canceled )
      { // if file has been imported successfully
        query = "COMMIT";
        res = PQexec( conn, query.toUtf8() );
        if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
        {
          QString err = PQresultErrorMessage( res );
          QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                                tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                                .arg( error ).arg( query ).arg( err ) );
          PQclear( res );
          continue;
        }

        PQclear( res );

        // remove file
        for ( int j = 0; j < tblShapefiles->rowCount(); j++ )
        {
          if ( tblShapefiles->item( j, ColFILENAME )->text() == QString( fileList[ i ] ->getName() ) )
          {
            tblShapefiles->selectRow( j );
            removeFile();
            i--;
            break;
          }
        }

        successes++;
      }
      else if ( !canceled )
      { // if problem importing file occured
        pro.setValue( temp_progress + tblShapefiles->item( i, ColFEATURECOUNT )->text().toInt() );
        QString errTxt = error + "\n" + errorText;
        QMessageBox::warning( this, tr( "Import Shapefiles" ), errTxt );
        query = "ROLLBACK";
        res = PQexec( conn, query.toUtf8() );
        if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
        {
          QString err = PQresultErrorMessage( res );
          QMessageBox::warning( &pro, tr( "Import Shapefiles" ),
                                tr( "%1\n<p>Error while executing the SQL:</p><p>%2</p><p>The database said:%3</p>" )
                                .arg( error ).arg( query ).arg( err ) );
        }

        PQclear( res );
      }
      else
      { // if import was actually canceled
        break;
      }
    }

    if ( successes == count )
      accept();
    else
      QMessageBox::information( &pro, tr( "Import Shapefiles" ),
                                tr( "%1 of %2 shapefiles could not be imported." ).arg( count - successes ).arg( count ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Import Shapefiles" ), tr( "You need to specify a Connection first" ) );
  }
}

QWidget *ShapefileTableDelegate::createEditor( QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex & index ) const
{
  switch ( index.column() )
  {
    case 4:
    {
      QComboBox* editor = new QComboBox( parent );
      editor->setSizeAdjustPolicy( QComboBox::AdjustToContents );
      editor->installEventFilter( const_cast<ShapefileTableDelegate*>( this ) );
      return editor;
      break;
    }
    case 1:
    case 3:
    {
      QLineEdit* editor = new QLineEdit( parent );
      editor->installEventFilter( const_cast<ShapefileTableDelegate*>( this ) );
      return editor;
      break;
    }
  }
  return NULL;
}

void ShapefileTableDelegate::setEditorData( QWidget *editor,
    const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case 4:
    {
      // Create a combobox and populate with the list of schemas
      QComboBox *comboBox = static_cast<QComboBox*>( editor );
      comboBox->insertItems( 0, mSchemaList );
      // Get the text from the table and use to set the selected text
      // in the combo box.
      QString text = index.model()->data( index, Qt::DisplayRole ).toString();
      comboBox->setCurrentIndex( mSchemaList.indexOf( text ) );
      break;
    }
    case 1:
    case 3:
    {
      QString text = index.model()->data( index, Qt::DisplayRole ).toString();
      QLineEdit *lineEdit = static_cast<QLineEdit*>( editor );
      lineEdit->setText( text );
      break;
    }
  }
}

void ShapefileTableDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case 4:
    {
      QComboBox *comboBox = static_cast<QComboBox*>( editor );
      QString text = comboBox->currentText();
      model->setData( index, text );
      break;
    }
    case 1:
    case 3:
    {
      QLineEdit *lineEdit = static_cast<QLineEdit*>( editor );
      QString text = lineEdit->text();

      model->setData( index, text );
      break;
    }
  }
}

void ShapefileTableDelegate::updateEditorGeometry( QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */ ) const
{
  editor->setGeometry( option.rect );
}
