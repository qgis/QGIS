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


#include <qlistbox.h>
#include <qtable.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qinputdialog.h>
#include <qfiledialog.h>
#include <qprogressdialog.h>
#include <qmemarray.h>
#include <qapplication.h>
#include <qfile.h>
#include <qsettings.h>
#include <qpixmap.h>
#include <iostream>
#include "../../src/qgspgutil.h"
#include "qgsspit.h"
#include "qgsconnectiondialog.h"
#include "qgseditreservedwordsdialog.h"
#include "qgsmessageviewer.h"
#include "spiticon.xpm"
#include "spit_icons.h"

// Qt implementation of alignment() + changed the numeric types to be shown on the left as well
int QTableItem::alignment() const
{
  bool num;
  bool ok1 = FALSE, ok2 = FALSE;
  ( void ) txt.toInt( &ok1 );
  if ( !ok1 )
    ( void ) txt.toDouble( &ok2 );
  num = ok1 || ok2;

  return ( num ? AlignLeft : AlignLeft ) | AlignVCenter;
}

QgsSpit::QgsSpit( QWidget *parent, const char *name ) : QgsSpitBase( parent, name )
{
  QPixmap icon;
  icon = QPixmap( spitIcon );
  setIcon( icon );

  populateConnectionList();
  defSrid = -1;
  defGeom = "the_geom";
  total_features = 0;
  //setFixedSize(QSize(605, 612));

  tblShapefiles->verticalHeader()->hide();
  //tblShapefiles->adjustColumn(3);
  tblShapefiles->setLeftMargin(0);
 
  // Set read only colums, the rest is editable
  tblShapefiles->setColumnReadOnly( ColFILENAME, true );
  tblShapefiles->setColumnReadOnly( ColFEATURECOUNT, true );

  chkUseDefaultSrid->setChecked( true );
  chkUseDefaultGeom->setChecked( true );
  useDefaultSrid();
  useDefaultGeom();

  schema_list << "public";
  gl_key = "/Qgis/connections/";
  getSchema();
  // init the geometry type list
}

QgsSpit::~QgsSpit()
{}

void QgsSpit::populateConnectionList()
{
  QSettings settings;
  QStringList keys = settings.subkeyList( "/Qgis/connections" );
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->insertItem( *it );
    ++it;
  }
}

void QgsSpit::newConnection()
{
  QgsConnectionDialog * con = new QgsConnectionDialog( this, "New Connection" );

  if ( con->exec() )
  {
    populateConnectionList();
    getSchema();
  }
}

void QgsSpit::editConnection()
{
  QgsConnectionDialog * con = new QgsConnectionDialog( this, cmbConnections->currentText() );
  if ( con->exec() )
  {
    con->saveConnection();
    getSchema();
  }
}

void QgsSpit::removeConnection()
{
  QSettings settings;
  QString key = "/Qgis/connections/" + cmbConnections->currentText();
  QString msg = "Are you sure you want to remove the [" + cmbConnections->currentText() + "] connection and all associated settings?";
  int result = QMessageBox::information( this, "Confirm Delete", msg, "Yes", "No" );
  if ( result == 0 )
  {
    settings.removeEntry( key + "/host" );
    settings.removeEntry( key + "/database" );
    settings.removeEntry( key + "/port" );
    settings.removeEntry( key + "/username" );
    settings.removeEntry( key + "/password" );
    settings.removeEntry( key + "/save" );

    cmbConnections->removeItem( cmbConnections->currentItem() );
  }
}

void QgsSpit::addFile()
{
  QString error1 = "";
  QString error2 = "";
  bool exist;
  bool is_error = false;
  QSettings settings;

  QStringList files = QFileDialog::getOpenFileNames(
                        "Shapefiles (*.shp)", settings.readEntry( "/Qgis/spit/last_directory" ), this, "add file dialog", "Add Shapefiles" );
  if ( files.size() > 0 )
  {
    // Save the directory for future use
    QFileInfo fi( files[ 0 ] );
    settings.writeEntry( "/Qgis/spit/last_directory", fi.dirPath( true ) );
  }
  // Process the files
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
  {
    exist = false;
    is_error = false;
    for ( int n = 0; n < tblShapefiles->numRows(); n++ )
    {
      if ( tblShapefiles->text( n, 0 ) == *it )
      {
        exist = true;
        break;
      }
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
        QgsShapeFile * file = new QgsShapeFile( name );
        if ( file->is_valid() )
        {
          /* XXX getFeatureClass actually does a whole bunch
           * of things and is probably better named 
           * something else
           */
          QString featureClass = file->getFeatureClass();
          int row = tblShapefiles->numRows();
          fileList.push_back( file );
          tblShapefiles->insertRows( row );
          tblShapefiles->setText( row, ColFILENAME, name );
          tblShapefiles->setText( row, ColFEATURECLASS, featureClass );
          tblShapefiles->setText( row, ColFEATURECOUNT, QString( "%1" ).arg( file->getFeatureCount() ) );
          tblShapefiles->setText( row, ColDBRELATIONNAME, file->getTable() );
          QComboTableItem* schema = new QComboTableItem( tblShapefiles, schema_list );
          schema->setCurrentItem( cmbSchema->currentText() );
          tblShapefiles->setItem( row, ColDBSCHEMA, schema );
          total_features += file->getFeatureCount();

          // check for postgresql reserved words
          // First get an instance of the PG utility class
          QgsPgUtil *pgu = QgsPgUtil::instance();
          bool hasReservedWords = false;
          // if a reserved word is found, set the flag so the "adjustment"
          // dialog can be presented to the user
          for ( int i = 0; i < file->column_names.size(); i++ )
          {
            if ( pgu->isReserved( file->column_names[ i ] ) )
            {
              hasReservedWords = true;
            }
          }
          // Why loop through all of them and then turn around and test
          // the flag? Because if there are reserved words, we want to
          // add all columns to the listview so the user can have the
          // opportunity to change them.
          if ( hasReservedWords )
          {
            // show the dialog for adjusting reserved words. Reserved
            // words are displayed differently so they are easy to spot
            QgsEditReservedWordsDialog * srw = new QgsEditReservedWordsDialog( this );
            srw->setCaption( file->getTable().upper() + " - Edit Column Names" );
            // load the reserved words list
            srw->setReservedWords( pgu->reservedWords() );
            // load the columns and set their status
            for ( int i = 0; i < file->column_names.size(); i++ )
            {
              srw->addColumn( file->column_names[ i ],
                              pgu->isReserved( file->column_names[ i ] ), i );
            }
            if ( srw->exec() )
            {
              // get the new column specs from the listview control
              // and replace the existing column spec for the shapefile
              file->setColumnNames( srw->columnNames() );
            }
          }


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

  if ( error1 != "" || error2 != "" )
  {
    QString message = tr("The following Shapefile(s) could not be loaded:\n\n");
    if ( error1 != "" )
    {
      error1 += "----------------------------------------------------------------------------------------";
      error1 += "\nREASON: File cannot be opened\n\n";
    }
    if ( error2 != "" )
    {
      error2 += "----------------------------------------------------------------------------------------";
      error2 += "\nREASON: One or both of the Shapefile files (*.dbf, *.shx) missing\n\n";
    }
    QgsMessageViewer * e = new QgsMessageViewer( this, "error" );
    e->setMessage( message + error1 + error2 );
    e->exec();
  }

  for (int i = 0; i < tblShapefiles->numCols(); i++)
  {
    tblShapefiles->adjustColumn( i );
  }  
//   tblShapefiles->adjustColumn( 0 );
//   tblShapefiles->adjustColumn( 1 );
//   tblShapefiles->adjustColumn( 2 );
//   tblShapefiles->adjustColumn( 3 );
//   tblShapefiles->adjustColumn( 4 );
//   tblShapefiles->setCurrentCell( -1, 0 );
}

void QgsSpit::removeFile()
{
  std::vector <int> temp;
  for ( int n = 0; n < tblShapefiles->numRows(); n++ )
    if ( tblShapefiles->isRowSelected( n ) )
    {
      for ( std::vector<QgsShapeFile *>::iterator vit = fileList.begin(); vit != fileList.end(); vit++ )
      {
        if ( ( *vit ) ->getName() == tblShapefiles->text( n, 0 ) )
        {
          total_features -= ( *vit ) ->getFeatureCount();
          fileList.erase( vit );
          break;
        }
      }
      temp.push_back( n );
    }
  QMemArray<int> array( temp.size() );
  for ( int i = 0; i < temp.size(); i++ )
    array[ i ] = temp[ i ];
  tblShapefiles->removeRows( array );
  tblShapefiles->setCurrentCell( -1, 0 );
}

void QgsSpit::removeAllFiles()
{
  QMemArray<int> array( tblShapefiles->numRows() );
  for ( int n = 0; n < tblShapefiles->numRows(); n++ )
    array[ n ] = n;

  fileList.clear();
  total_features = 0;
  tblShapefiles->removeRows( array );
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
  QString message = "General Interface Help:\n\n";
  message += QString(
               "PostgreSQL Connections:\n" ) + QString(
               "----------------------------------------------------------------------------------------\n" ) + QString(
               "[New ...] - create a new connection\n" ) + QString(
               "[Edit ...] - edit the currently selected connection\n" ) + QString(
               "[Remove] - remove the currently selected connection\n" ) + QString(
               "-you need to select a connection that works (connects properly) in order to import files\n" ) + QString(
               "-when changing connections Global Schema also changes accordingly\n\n" ) + QString(
               "Shapefile List:\n" ) + QString(
               "----------------------------------------------------------------------------------------\n" ) + QString(
               "[Add ...] - open a File dialog and browse to the desired file(s) to import\n" ) + QString(
               "[Remove] - remove the currently selected file(s) from the list\n" ) + QString(
               "[Remove All] - remove all the files in the list\n" ) + QString(
               "[SRID] - Reference ID for the shapefiles to be imported\n" ) + QString(
               "[Use Default (SRID)] - set SRID to -1\n" ) + QString(
               "[Geometry Column Name] - name of the geometry column in the database\n" ) + QString(
               "[Use Default (Geometry Column Name)] - set column name to \'the_geom\'\n" ) + QString(
               "[Glogal Schema] - set the schema for all files to be imported into\n\n" ) + QString(
               "----------------------------------------------------------------------------------------\n" ) + QString(
               "[Import] - import the current shapefiles in the list\n" ) + QString(
               "[Quit] - quit the program\n" ) + QString(
               "[Help] - display this help dialog\n\n" );
  QgsMessageViewer * e = new QgsMessageViewer( this, "HelpMessage" );
  e->setMessage( message );
  e->exec();
}

PGconn* QgsSpit::checkConnection()
{
  QSettings settings;
  PGconn * pd;
  bool result = true;
  QString connName = cmbConnections->currentText();
  if ( connName.isEmpty() )
  {
    QMessageBox::warning( this, tr("Import Shapefiles"), tr("You need to specify a Connection first") );
    result = false;
  }
  else
  {
    QString connInfo = "host=" + settings.readEntry( gl_key + connName + "/host" ) +
                       " dbname=" + settings.readEntry( gl_key + connName + "/database" ) +
                       " port=" + settings.readEntry( gl_key + connName + "/port" ) +
                       " user=" + settings.readEntry( gl_key + connName + "/username" ) +
                       " password=" + settings.readEntry( gl_key + connName + "/password" );
    pd = PQconnectdb( ( const char * ) connInfo );

    if ( PQstatus( pd ) != CONNECTION_OK )
    {
      QMessageBox::warning( this, tr("Import Shapefiles"), tr("Connection failed - Check settings and try again") );
      result = false;
    }
  }
  if ( result )
    return pd;
  else
    return NULL;
}

void QgsSpit::getSchema()
{
  QSettings settings;
  schema_list.clear();
  schema_list << "public";
  PGconn* pd = checkConnection();
  if ( pd != NULL )
  {
    QString connName = cmbConnections->currentText();
    QString user = settings.readEntry( gl_key + connName + "/username" );
    QString schemaSql = QString( "select nspname from pg_namespace,pg_user where nspowner = usesysid and usename = '%1'" ).arg( user );
    PGresult *schemas = PQexec( pd, ( const char * ) schemaSql );
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

  // update the schemas in the combo of all the shapefiles
  for ( int i = 0; i < tblShapefiles->numRows(); i++ )
  {
    tblShapefiles->clearCell( i, 4 );
    QComboTableItem* temp_schemas = new QComboTableItem( tblShapefiles, schema_list );
    temp_schemas->setCurrentItem( "public" );
    tblShapefiles->setItem( i, 4, temp_schemas );

  }

  cmbSchema->clear();
  cmbSchema->insertStringList( schema_list );
  cmbSchema->setCurrentText( "public" );
}

void QgsSpit::updateSchema()
{
  for ( int i = 0; i < tblShapefiles->numRows(); i++ )
  {
    tblShapefiles->clearCell( i, 4 );
    QComboTableItem* temp_schemas = new QComboTableItem( tblShapefiles, schema_list );
    temp_schemas->setCurrentItem( cmbSchema->currentText() );
    tblShapefiles->setItem( i, 4, temp_schemas );

  }
}

void QgsSpit::import()
{
  tblShapefiles->setCurrentCell( -1, 0 );

  QString connName = cmbConnections->currentText();
  QSettings settings;
  bool cancelled = false;
  PGconn* pd = checkConnection();
  QString query;

  if ( total_features == 0 )
  {
    QMessageBox::warning( this, "Import Shapefiles", 
                         "You need to add shapefiles to the list first" );
  }
  else if ( pd != NULL )
  {
    PGresult * res;
    QProgressDialog * pro = new QProgressDialog( tr("Importing files"), 
                            tr("Cancel"), total_features, 
                            this, tr("Progress"), true );
    pro->setProgress( 0 );
    pro->setAutoClose( true );
    pro->show();
    qApp->processEvents();

    for ( int i = 0; i < fileList.size() ; i++ )
    {
      QString error = "Problem inserting features from file:\n" + 
                      tblShapefiles->text( i, ColFILENAME );
                      
      // if a name starts with invalid character
      if ( !( tblShapefiles->text( i, ColDBRELATIONNAME ) ) [ 0 ].isLetter() )
      {
        QMessageBox::warning( pro, "Import Shapefiles", 
                             error + "\nInvalid table name." );
        pro->setProgress( pro->progress() 
                        + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
        continue;
      }

      // if no fields detected
      if ( ( fileList[ i ] ->column_names ).size() == 0 )
      {
        QMessageBox::warning( pro, "Import Shapefiles", 
                            error + "\nNo fields detected." );
        pro->setProgress( pro->progress() + 
                        ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
        continue;
      }

      // duplicate field check
      std::vector<QString> names_copy = fileList[ i ] ->column_names;
      std::cerr << "Size of names_copy before sort: " << names_copy.size() << std::endl;
      QString dupl = "";
      std::sort( names_copy.begin(), names_copy.end() );
      std::cerr << "Size of names_copy after sort: " << names_copy.size() << std::endl;

      for ( int k = 1; k < names_copy.size(); k++ )
      {
        std::cerr << "USING :" << names_copy[ k ] << " index " << k << std::endl;
        qWarning( "Checking to see if " + names_copy[ k ] + " == " + names_copy[ k - 1 ] );
        if ( names_copy[ k ] == names_copy[ k - 1 ] )
          dupl += names_copy[ k ] + "\n";
      }
      // if duplicate field names exist
      if ( dupl != "" )
      {
        QMessageBox::warning( pro, "Import Shapefiles", error +
                              "\nThe following fields are duplicates:\n" + dupl );
        pro->setProgress( pro->progress() + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
        continue;
      }

      // Check and set destination table 
      fileList[ i ] ->setTable( tblShapefiles->text( i, ColDBRELATIONNAME ) );
      pro->setLabelText( "Importing files\n" 
                      + tblShapefiles->text(i, ColFILENAME) );
      bool rel_exists1 = false;
      bool rel_exists2 = false;
      query = "SELECT f_table_name FROM geometry_columns WHERE f_table_name=\'" 
              + tblShapefiles->text( i, ColDBRELATIONNAME ) +
              "\' AND f_table_schema=\'" 
              + tblShapefiles->text( i, ColDBSCHEMA ) + "\'";
      res = PQexec( pd, ( const char * ) query );
      rel_exists1 = ( PQntuples( res ) > 0 );
      if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
      {
        qWarning( PQresultErrorMessage( res ) );
        QMessageBox::warning( pro, "Import Shapefiles", error );
        pro->setProgress( pro->progress() + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
        continue;
      }
      else
      {
        PQclear( res );
      }

      query = "SELECT tablename FROM pg_tables WHERE tablename=\'" + tblShapefiles->text( i, ColDBRELATIONNAME ) + "\' AND schemaname=\'" +
              tblShapefiles->text( i, ColDBSCHEMA ) + "\'";
      res = PQexec( pd, ( const char * ) query );
      qWarning( query );
      rel_exists2 = ( PQntuples( res ) > 0 );
      if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
      {
        qWarning( PQresultErrorMessage( res ) );
        QMessageBox::warning( pro, "Import Shapefiles", error );
        pro->setProgress( pro->progress() + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
        continue;
      }
      else
      {
        PQclear( res );
      }

      // begin session
      query = "BEGIN";
      res = PQexec( pd, query );
      if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
      {
        qWarning( PQresultErrorMessage( res ) );
        QMessageBox::warning( pro, "Import Shapefiles", error );
        pro->setProgress( pro->progress() + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
        continue;
      }
      else
      {
        PQclear( res );
      }

      query = "SET SEARCH_PATH TO \'";
      if ( tblShapefiles->text( i, ColDBSCHEMA ) == "public" )
        query += "public\'";
      else
        query += tblShapefiles->text( i, ColDBSCHEMA ) + "\', \'public\'";
      res = PQexec( pd, query );
      qWarning( query );
      if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
      {
        qWarning( PQresultErrorMessage( res ) );
        qWarning( PQresStatus( PQresultStatus( res ) ) );
        QMessageBox::warning( pro, "Import Shapefiles", error );
        pro->setProgress( pro->progress() + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
        continue;
      }
      else
      {
        PQclear( res );
      }

      QMessageBox *del_confirm;
      if ( rel_exists1 || rel_exists2 )
      {
        del_confirm = new QMessageBox( "Import Shapefiles - Relation Exists",
                                       "The Shapefile:\n" + tblShapefiles->text( i, 0 ) + "\nwill use [" +
                                       tblShapefiles->text( i, ColDBRELATIONNAME ) + "] relation for its data,\nwhich already exists and possibly contains data.\n" +
                                       "To avoid data loss change the \"DB Relation Name\" \nfor this Shapefile in the main dialog file list.\n\n" +
                                       "Do you want to overwrite the [" + tblShapefiles->text( i, ColDBRELATIONNAME ) + "] relation?",
                                       QMessageBox::Warning,
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No | QMessageBox::Escape,
                                       QMessageBox::NoButton, this, "Relation Exists" );

        if ( del_confirm->exec() == QMessageBox::Yes )
        {
          if ( rel_exists2 )
          {
            query = "DROP TABLE " + tblShapefiles->text( i, ColDBRELATIONNAME );
            qWarning( query );
            res = PQexec( pd, ( const char * ) query );
            if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
            {
              qWarning( PQresultErrorMessage( res ) );
              QMessageBox::warning( pro, "Import Shapefiles", error );
              pro->setProgress( pro->progress() + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
              continue;
            }
            else
            {
              PQclear( res );
            }
          }

          if ( rel_exists1 )
          {
            /*query = "SELECT DropGeometryColumn(\'"+QString(settings.readEntry(key + "/database"))+"\', \'"+
              fileList[i]->getTable()+"\', \'"+txtGeomName->text()+"')";*/
            query = "DELETE FROM geometry_columns WHERE f_table_schema=\'" + tblShapefiles->text( i, ColDBSCHEMA ) + "\' AND " +
                    "f_table_name=\'" + tblShapefiles->text( i, ColDBRELATIONNAME ) + "\'";
            qWarning( query );
            res = PQexec( pd, ( const char * ) query );
            if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
            {
              qWarning( PQresultErrorMessage( res ) );
              QMessageBox::warning( pro, "Import Shapefiles", error );
              pro->setProgress( pro->progress() + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
              continue;
            }
            else
            {
              PQclear( res );
            }
          }
        }
        else
        {
          query = "ROLLBACK";
          res = PQexec( pd, query );
          if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          {
            qWarning( PQresultErrorMessage( res ) );
            QMessageBox::warning( pro, "Import Shapefiles", error );
          }
          else
          {
            PQclear( res );
          }
          pro->setProgress( pro->progress() + ( tblShapefiles->text( i, 2 ) ).toInt() );
          continue;
        }
      }

      // importing file here
      int temp_progress = pro->progress();
      cancelled = false;
      if ( fileList[ i ] ->insertLayer( settings.readEntry( gl_key + connName + "/database" ), tblShapefiles->text( i, ColDBSCHEMA ),
                                        txtGeomName->text(), QString( "%1" ).arg( spinSrid->value() ), pd, pro, cancelled ) && !cancelled )
      { // if file has been imported successfully
        query = "COMMIT";
        res = PQexec( pd, query );
        if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
        {
          qWarning( PQresultErrorMessage( res ) );
          QMessageBox::warning( pro, "Import Shapefiles", error );
          continue;
        }
        else
        {
          PQclear( res );
        }

        // remove file
        for ( int j = 0; j < tblShapefiles->numRows(); j++ )
        {
          if ( tblShapefiles->text( j, ColFILENAME ) == QString( fileList[ i ] ->getName() ) )
          {
            tblShapefiles->selectRow( j );
            removeFile();
            i--;
            break;
          }
        }

      }
      else if ( !cancelled )
      { // if problem importing file occured
        pro->setProgress( temp_progress + ( tblShapefiles->text( i, ColFEATURECOUNT ) ).toInt() );
        QMessageBox::warning( this, "Import Shapefiles", error );
        query = "ROLLBACK";
        res = PQexec( pd, query );
        if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
        {
          qWarning( PQresultErrorMessage( res ) );
          QMessageBox::warning( pro, "Import Shapefiles", error );
        }
        else
        {
          PQclear( res );
        }
      }
      else
      { // if import was actually cancelled
        delete pro;
        break;
      }
    }
    delete pro;
  }
  PQfinish( pd );
}
void QgsSpit::editColumns( int row, int col, int button, const QPoint &mousePos )
{
  // get the shapefile - table row maps directly to the fileList array
  QgsPgUtil * pgu = QgsPgUtil::instance();
  // show the dialog for adjusting reserved words. Reserved
  // words are displayed differently so they are easy to spot
  QgsEditReservedWordsDialog *srw = new QgsEditReservedWordsDialog( this );
  srw->setCaption( fileList[ row ] ->getTable().upper() + " - Edit Column Names" );
  // load the reserved words list
  srw->setReservedWords( pgu->reservedWords() );
  // load the columns and set their status
  for ( int i = 0; i < fileList[ row ] ->column_names.size(); i++ )
  {
    srw->addColumn( fileList[ row ] ->column_names[ i ],
                    pgu->isReserved( fileList[ row ] ->column_names[ i ] ), i );
  }
  if ( srw->exec() )
  {
    // get the new column specs from the listview control
    // and replace the existing column spec for the shapefile
    fileList[ row ] ->setColumnNames( srw->columnNames() );
  }

}

void QgsSpit::editShapefile(int row, int col, int button, const QPoint& mousePos)
{
  if (ColFEATURECLASS == col || ColDBRELATIONNAME == col)
  {
    tblShapefiles->editCell(row, col, FALSE);
  }
}
