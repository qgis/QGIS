/***************************************************************************
             QgsSpatialiteSridsDialog.cpp -  Dialog for selecting an
                  SRID from a Spatialite spatial_ref_sys table
                             -------------------
    begin                : 2010-04-03
    copyright            : (C) 2010 by Gary Sherman
    email                : gsherman@geoapt.com
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
#include "qgsspatialitesridsdialog.h"
QgsSpatialiteSridsDialog::QgsSpatialiteSridsDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  db = 0;
  pbnFilter->setDefault( true );
  leSearch->setFocus();
}

QgsSpatialiteSridsDialog::~QgsSpatialiteSridsDialog()
{
  sqlite3_close( db );
}
QString QgsSpatialiteSridsDialog::selectedSrid()
{
  return twSrids->currentItem()->text( 0 );
}
bool QgsSpatialiteSridsDialog::load( QString dbName )
{
  mDbName = dbName;
  bool status = true;
  if ( !db )
  {
    int rc = sqlite3_open_v2( dbName.toUtf8(), &db, SQLITE_OPEN_READONLY, NULL );
    if ( rc != SQLITE_OK )
    {
      QMessageBox::warning( this, tr( "SpatiaLite Database" ), tr( "Unable to open the database" ) );
      return false;
    }
  }
  // load up the srid table
  // prepare the sql statement
  const char *pzTail;
  sqlite3_stmt *ppStmt;
  QString sql = "select auth_srid, auth_name, ref_sys_name from spatial_ref_sys order by srid asc";

  int rc = sqlite3_prepare( db, sql.toUtf8(), sql.toUtf8().length(), &ppStmt, &pzTail );
  // XXX Need to free memory from the error msg if one is set
  if ( rc == SQLITE_OK )
  {
    // get the first row of the result set
    while ( sqlite3_step( ppStmt ) == SQLITE_ROW )
    {
      QTreeWidgetItem *item = new QTreeWidgetItem( twSrids );
      // srid
      item->setText( 0, QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 0 ) ) );
      item->setText( 1, QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 1 ) ) );
      // name
      item->setText( 2, QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 2 ) ) );
      // proj4 text
      item->setText( 3, QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 3 ) ) );

    }
    twSrids->sortByColumn( 0, Qt::AscendingOrder );
  }
  else
  {
    // XXX query failed -- warn the user some how
    QMessageBox::warning( 0, tr( "Error" ),  tr( "Failed to load SRIDS: %1" ).arg( sqlite3_errmsg( db ) ) );
    status = false;
  }
  // close the statement
  sqlite3_finalize( ppStmt );
  return status;
}
void QgsSpatialiteSridsDialog::on_pbnFilter_clicked()
{
  if ( leSearch->text().length() == 0 )
  {
    load( mDbName );
  }
  else
  {
    const char *pzTail;
    sqlite3_stmt *ppStmt;
    QString search;
    if ( radioButtonSrid->isChecked() )
    {
      search = "srid";
    }
    else
    {
      search = "ref_sys_name";
    }
    QString sql = QString( "select auth_srid, auth_name, ref_sys_name from spatial_ref_sys where %1 like '%" ).arg( search ) + leSearch->text() + QString( "%' order by srid asc" );
    //QMessageBox::information(0, "Sql", sql);

    int rc = sqlite3_prepare( db, sql.toUtf8(), sql.toUtf8().length(), &ppStmt, &pzTail );
    // XXX Need to free memory from the error msg if one is set
    if ( rc == SQLITE_OK )
    {
      // clear the display
      twSrids->clear();
      // get the first row of the result set
      while ( sqlite3_step( ppStmt ) == SQLITE_ROW )
      {
        QTreeWidgetItem *item = new QTreeWidgetItem( twSrids );
        // srid
        item->setText( 0, QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 0 ) ) );
        item->setText( 1, QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 1 ) ) );
        // name
        item->setText( 2, QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 2 ) ) );
        // proj4 text
        item->setText( 3, QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 3 ) ) );

      }
      twSrids->sortByColumn( 0, Qt::AscendingOrder );
    }
    else
    {
      // XXX query failed -- warn the user some how
      QMessageBox::warning( 0, tr( "Error" ),  tr( "Failed to load SRIDS: %1" ).arg( sqlite3_errmsg( db ) ) );
    }
    // close the statement
    sqlite3_finalize( ppStmt );
  }
}
