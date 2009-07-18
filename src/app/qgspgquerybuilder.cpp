/***************************************************************************
                qgspgquerybuilder.cpp - PostgreSQL Query Builder
                     --------------------------------------
               Date                 : 2004-11-19
               Copyright            : (C) 2004 by Gary E.Sherman
               Email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include "qgspgquerybuilder.h"
#include "qgslogger.h"
#include <QListView>
#include <QMessageBox>
#include <QRegExp>
#include "qgslogger.h"

#if QT_VERSION < 0x040300
#define toPlainText() text()
#endif


// default constructor
QgsPgQueryBuilder::QgsPgQueryBuilder( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  setupGuiViews();
}
// constructor used when the query builder must make its own
// connection to the database
QgsPgQueryBuilder::QgsPgQueryBuilder( QgsDataSourceURI *uri,
                                      QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mUri( uri )
{
  setupUi( this );
  setupGuiViews();
  // The query builder must make its own connection to the database when
  // using this constructor
  QString connectionInfo = mUri->connectionInfo();

  QgsDebugMsg( "Attempting connect using: " + connectionInfo );

  mPgConnection = PQconnectdb( connectionInfo.toLocal8Bit().data() ); // use what is set based on locale; after connecting, use Utf8
  // check the connection status
  if ( PQstatus( mPgConnection ) == CONNECTION_OK )
  {
    QString datasource = tr( "Table <b>%1</b> in database <b>%2</b> on host <b>%3</b>, user <b>%4</b>" )
                         .arg( mUri->table() )
                         .arg( PQdb( mPgConnection ) )
                         .arg( PQhost( mPgConnection ) )
                         .arg( PQuser( mPgConnection ) );
    mOwnConnection = true; // we own this connection since we created it
    // tell the DB that we want text encoded in UTF8
    PQsetClientEncoding( mPgConnection, "UNICODE" );

    lblDataUri->setText( datasource );
    populateFields();
  }
  else
  {
    QString err = QString::fromLocal8Bit( PQerrorMessage( mPgConnection ) );
    QMessageBox::critical( this, tr( "Connection Failed" ), tr( "Connection to the database failed:\n%1" ).arg( err ) );
  }
}
// Constructor when using an existing connection (ie from the add postgis layer dialog). This
// is useful since the query builder doesn't have to make a connection to the database for each
// layer being customized prior to adding to the map canvas. Since the table name passed to this
// constructor from the add table dialog is in the form schema.table, these components are
// parsed out and populated in the mURI structure prior to performing any operations against the database.
QgsPgQueryBuilder::QgsPgQueryBuilder( QString tableName, PGconn *con,
                                      QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mPgConnection( con )
{
  setupUi( this );
  setupGuiViews();
  mOwnConnection = false; // we don't own this connection since it was passed to us
  mUri = new QgsDataSourceURI( "table=" + tableName );
  QString datasource = tr( "Table <b>%1</b> in database <b>%2</b> on host <b>%3</b>, user <b>%4</b>" )
                       .arg( tableName )
                       .arg( PQdb( mPgConnection ) )
                       .arg( PQhost( mPgConnection ) )
                       .arg( PQuser( mPgConnection ) );

  lblDataUri->setText( datasource );
  populateFields();
}
QgsPgQueryBuilder::~QgsPgQueryBuilder()
{
  if ( mOwnConnection )
  {
    PQfinish( mPgConnection );
  }
}

void QgsPgQueryBuilder::populateFields()
{
  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  QString sql = "select * from " + mUri->quotedTablename() + " limit 1";
  PGresult *result = PQexec( mPgConnection, sql.toUtf8() );
  QgsLogger::debug( "Query executed: " + sql );
  if ( PQresultStatus( result ) == PGRES_TUPLES_OK )
  {
    //--std::cout << "Field: Name, Type, Size, Modifier:" << std::endl;

    for ( int i = 0; i < PQnfields( result ); i++ )
    {

      QString fieldName = QString::fromUtf8( PQfname( result, i ) );
      int fldtyp = PQftype( result, i );
      QString typOid = QString().setNum( fldtyp );
      QgsLogger::debug( "typOid is: " + typOid );
      //int fieldModifier = PQfmod(result, i);
      QString sql = QString( "select typelem from pg_type where typelem=%1 and typlen=-1" ).arg( typOid );
      //  //--std::cout << sql << std::endl;

      PGresult *oidResult = PQexec( mPgConnection, sql.toUtf8() );
      if ( PQresultStatus( oidResult ) == PGRES_TUPLES_OK )
        QgsLogger::debug( "Ok fetching typelem using\n" + sql );

      // get the oid of the "real" type
      QString poid = QString::fromUtf8( PQgetvalue( oidResult, 0,
                                        PQfnumber( oidResult,
                                                   "typelem" ) ) );
      QgsLogger::debug( "poid is: " + poid );
      PQclear( oidResult );
      sql = QString( "select typname, typlen from pg_type where oid=%1" ).arg( poid );
      // //--std::cout << sql << std::endl;

      oidResult = PQexec( mPgConnection, sql.toUtf8() );
      if ( PQresultStatus( oidResult ) == PGRES_TUPLES_OK )
        QgsLogger::debug( "Ok fetching typenam,etc\n" );

      QString fieldType = QString::fromUtf8( PQgetvalue( oidResult, 0, 0 ) );
      QString fieldSize = QString::fromUtf8( PQgetvalue( oidResult, 0, 1 ) );
      PQclear( oidResult );
      QgsDebugMsg( QString( "Field parms: Name = %1, Type = %2" ).arg( fieldName ).arg( fieldType ) );
      QVariant::Type type = QVariant::String; // TODO: should be set correctly [MD]
      mFieldMap[fieldName] = QgsField( fieldName, type, fieldType );
      QStandardItem *myItem = new QStandardItem( fieldName );
      myItem->setEditable( false );
      mModelFields->insertRow( mModelFields->rowCount(), myItem );
    }
    // All fields get ... setup
    setupLstFieldsModel();
  }
  else
  {
    QgsDebugMsg( "Error fetching a row from " + mUri->table() );
  }
  PQclear( result );
}

void QgsPgQueryBuilder::setupLstFieldsModel()
{
  lstFields->setModel( mModelFields );
}

void QgsPgQueryBuilder::setupGuiViews()
{
  //Initialize the models
  mModelFields = new QStandardItemModel();
  mModelValues = new QStandardItemModel();
  // Modes
  lstFields->setViewMode( QListView::ListMode );
  lstValues->setViewMode( QListView::ListMode );
  lstFields->setSelectionBehavior( QAbstractItemView::SelectRows );
  lstValues->setSelectionBehavior( QAbstractItemView::SelectRows );
  // Performance tip since Qt 4.1
  lstFields->setUniformItemSizes( true );
  lstValues->setUniformItemSizes( true );
  // Colored rows
  lstFields->setAlternatingRowColors( true );
  lstValues->setAlternatingRowColors( true );
}

void QgsPgQueryBuilder::fillValues( QString theSQL )
{
  // clear the model
  mModelValues->clear();

  // determine the field type
  QgsField field = mFieldMap[mModelFields->data( lstFields->currentIndex() ).toString()];

  PGresult *result = PQexec( mPgConnection, theSQL.toUtf8() );

  if ( PQresultStatus( result ) == PGRES_TUPLES_OK )
  {
    int rowCount =  PQntuples( result );
    for ( int i = 0; i < rowCount; i++ )
    {
      QString value = QString::fromUtf8( PQgetvalue( result, i, 0 ) );
      QStandardItem *myItem = new QStandardItem( value );
      myItem->setEditable( false );
      mModelValues->insertRow( mModelValues->rowCount(), myItem );
    }
  }
  else
  {
    QMessageBox::warning( this, tr( "Database error" ),
                          tr( "<p>Failed to get sample of field values using SQL:</p><p>%1</p><p>Error message was: %2</p>" )
                          .arg( theSQL ).arg( QString::fromUtf8( PQerrorMessage( mPgConnection ) ) ) );
  }
  // free the result set
  PQclear( result );
}

void QgsPgQueryBuilder::on_btnSampleValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );
  QString myFieldName = mModelFields->data( lstFields->currentIndex() ).toString();
  if ( myFieldName.isEmpty() )
    return;

  QgsField field = mFieldMap[mModelFields->data( lstFields->currentIndex() ).toString()];

  QString sql = QString( "SELECT DISTINCT \"%1\" FROM (SELECT \"%1\" FROM %2 LIMIT 5000) AS foo ORDER BY \"%1\" LIMIT 25" )
                .arg( myFieldName ).arg( mUri->quotedTablename() );

  //delete connection mModelValues and lstValues
  QStandardItemModel *tmp = new QStandardItemModel();
  lstValues->setModel( tmp );
  //Clear and fill the mModelValues
  fillValues( sql );
  lstValues->setModel( mModelValues );
  lstValues->setCursor( Qt::ArrowCursor );
  //delete the tmp
  delete tmp;

}

void QgsPgQueryBuilder::on_btnGetAllValues_clicked()
{
  lstValues->setCursor( Qt::WaitCursor );
  QString myFieldName = mModelFields->data( lstFields->currentIndex() ).toString();
  if ( myFieldName.isEmpty() )
    return;

  QString sql = QString( "select distinct \"%1\" from %2 order by \"%1\"" )
                .arg( myFieldName ).arg( mUri->quotedTablename() );

  //delete connection mModelValues and lstValues
  QStandardItemModel *tmp = new QStandardItemModel();
  lstValues->setModel( tmp );
  //Clear and fill the mModelValues
  fillValues( sql );
  lstValues->setModel( mModelValues );
  lstValues->setCursor( Qt::ArrowCursor );
  //delete the tmp
  delete tmp;
}

void QgsPgQueryBuilder::on_btnTest_clicked()
{
  // test the sql statement to see if it works
  // by counting the number of records that would be
  // returned

  // if there is no sql, issue a warning
  if ( txtSQL->toPlainText().isEmpty() )
  {
    QMessageBox::information( this, tr( "No Query" ), tr( "You must create a query before you can test it" ) );
  }
  else
  {
    QString numRows;
    QString sql = QString( "select count(*) from %1 where %2" )
                  .arg( mUri->quotedTablename() ).arg( txtSQL->toPlainText() );
    PGresult *result = PQexec( mPgConnection, sql.toUtf8() );
    if ( PQresultStatus( result ) == PGRES_TUPLES_OK )
    {
      numRows = QString::fromUtf8( PQgetvalue( result, 0, 0 ) );
      QMessageBox::information( this, tr( "Query Result" ),
                                tr( "The where clause returned %n row(s).", "returned test rows", numRows.toInt() ) );
    }
    else
    {
      QMessageBox::warning( this, tr( "Query Failed" ),
                            tr( "An error occurred when executing the query:\n%1" )
                            .arg( QString::fromUtf8( PQresultErrorMessage( result ) ) ) );
    }
    // free the result set
    PQclear( result );
  }
}
// This method tests the number of records that would be returned by the
// query
// XXX This should really throw an exception
long QgsPgQueryBuilder::countRecords( QString where )
{
  QString sql = QString( "select count(*) from %1 where %2" ).arg( mUri->quotedTablename() ).arg( where );

  long numRows;
  PGresult *result = PQexec( mPgConnection, sql.toUtf8() );
  if ( PQresultStatus( result ) == PGRES_TUPLES_OK )
  {
    QString rowCount = QString::fromUtf8( PQgetvalue( result, 0, 0 ) );
    numRows = rowCount.toLong();
  }
  else
  {
    numRows = -1;
    mPgErrorMessage = QString::fromUtf8( PQresultErrorMessage( result ) );

  }
  // free the result set
  PQclear( result );
  return numRows;
}

void QgsPgQueryBuilder::setConnection( PGconn *con )
{
  // store the connection
  mPgConnection = con;
}

void QgsPgQueryBuilder::on_btnOk_clicked()
{
  // if user hits Ok and there is no query, skip the validation
  if ( txtSQL->toPlainText().trimmed().length() > 0 )
  {
    // test the query to see if it will result in a valid layer
    long numRecs = countRecords( txtSQL->toPlainText() );
    if ( numRecs == -1 )
    {
      //error in query - show the problem
      QMessageBox::warning( this, tr( "Error in Query" ), mPgErrorMessage );
    }
    else
    {
#if 0 // when is this necessary? [jef]
      if ( numRecs == 0 )
      {
        QMessageBox::warning( this, tr( "No Records" ), tr( "The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature." ) );
      }
      else
      {
#endif
        this->accept();
#if 0
      }
#endif
    }
  }
  else
  {
    this->accept();
  }
}

void QgsPgQueryBuilder::on_btnEqual_clicked()
{
  txtSQL->insertPlainText( " = " );
}

void QgsPgQueryBuilder::on_btnLessThan_clicked()
{
  txtSQL->insertPlainText( " < " );
}

void QgsPgQueryBuilder::on_btnGreaterThan_clicked()
{
  txtSQL->insertPlainText( " > " );
}

void QgsPgQueryBuilder::on_btnPct_clicked()
{
  txtSQL->insertPlainText( "%" );
}

void QgsPgQueryBuilder::on_btnIn_clicked()
{
  txtSQL->insertPlainText( " IN " );
}

void QgsPgQueryBuilder::on_btnNotIn_clicked()
{
  txtSQL->insertPlainText( " NOT IN " );
}

void QgsPgQueryBuilder::on_btnLike_clicked()
{
  txtSQL->insertPlainText( " LIKE " );
}

QString QgsPgQueryBuilder::sql()
{
  return txtSQL->toPlainText();
}

void QgsPgQueryBuilder::setSql( QString sqlStatement )
{
  txtSQL->setText( sqlStatement );
}

void QgsPgQueryBuilder::on_lstFields_clicked( const QModelIndex &index )
{
  if ( mPreviousFieldRow != index.row() )
  {
    mPreviousFieldRow = index.row();

    // If type is gemetry .. normal users don't want to get values?
    QgsField field = mFieldMap[mModelFields->data( lstFields->currentIndex() ).toString()];
    if ( field.typeName().indexOf( "geometry" ) > -1 )
    {
      btnSampleValues->setEnabled( false );
      btnGetAllValues->setEnabled( false );
    }
    else
    {
      btnSampleValues->setEnabled( true );
      btnGetAllValues->setEnabled( true );
    }
    mModelValues->clear();
  }
}

void QgsPgQueryBuilder::on_lstFields_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertPlainText( "\"" + mModelFields->data( index ).toString() + "\"" );
}

void QgsPgQueryBuilder::on_lstValues_doubleClicked( const QModelIndex &index )
{
  txtSQL->insertPlainText( "'" + mModelValues->data( index ).toString() + "'" );
}

void QgsPgQueryBuilder::on_btnLessEqual_clicked()
{
  txtSQL->insertPlainText( " <= " );
}

void QgsPgQueryBuilder::on_btnGreaterEqual_clicked()
{
  txtSQL->insertPlainText( " >= " );
}

void QgsPgQueryBuilder::on_btnNotEqual_clicked()
{
  txtSQL->insertPlainText( " != " );
}

void QgsPgQueryBuilder::on_btnAnd_clicked()
{
  txtSQL->insertPlainText( " AND " );
}

void QgsPgQueryBuilder::on_btnNot_clicked()
{
  txtSQL->insertPlainText( " NOT " );
}

void QgsPgQueryBuilder::on_btnOr_clicked()
{
  txtSQL->insertPlainText( " OR " );
}

void QgsPgQueryBuilder::on_btnClear_clicked()
{
  txtSQL->clear();
}

void QgsPgQueryBuilder::on_btnILike_clicked()
{
  txtSQL->insertPlainText( " ILIKE " );
}
void QgsPgQueryBuilder::setDatasourceDescription( QString uri )
{
  lblDataUri->setText( uri );
}
