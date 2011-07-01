/*
** File: evisdatabaseconnectiongui.cpp
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-07
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
#include "evisdatabaseconnectiongui.h"

#include "qgscontexthelp.h"
#include "qgsapplication.h"

#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QUrl>

/**
* Constructor
* @param parent - Pointer the to parent QWidget for modality
* @param fl - Windown flags
*/
eVisDatabaseConnectionGui::eVisDatabaseConnectionGui( QList<QTemporaryFile*>* theTemoraryFileList, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  mTempOutputFileList = theTemoraryFileList;

  //Initialize varaibles
  mQueryDefinitionMap = new QMap<int, eVisQueryDefinition>;
  mDatabaseConnection = 0;

  //Create a new instance of the file selector
  mDatabaseLayerFieldSelector = new eVisDatabaseLayerFieldSelectionGui( this, fl );
  connect( mDatabaseLayerFieldSelector, SIGNAL( eVisDatabaseLayerFieldsSelected( QString, QString, QString ) ), this, SLOT( drawNewVectorLayer( QString, QString, QString ) ) );

  //Populate gui components
#ifdef WIN32
  cboxDatabaseType->insertItem( 0, "MSAccess" );
#endif
  cboxDatabaseType->insertItem( 0, "MYSQL" );
  cboxDatabaseType->insertItem( 0, "ODBC" );
  cboxDatabaseType->insertItem( 0, "PostGreSQL" );
  cboxDatabaseType->insertItem( 0, "SQLITE" );
  cboxDatabaseType->insertItem( 0, tr( "Undefined" ) );
  cboxDatabaseType->setCurrentIndex( 0 );
  cboxPredefinedQueryList->insertItem( 0, tr( "No predefined queries loaded" ) );

  //set icons
  QString myThemePath = QgsApplication::activeThemePath( );
  pbtnOpenFile->setIcon( QIcon( QPixmap( myThemePath + "/mActionFolder.png" ) ) );
  pbtnOpenFile->setToolTip( tr( "Open File" ) );
  pbtnLoadPredefinedQueries->setIcon( QIcon( QPixmap( myThemePath + "/mActionFolder.png" ) ) );
  pbtnLoadPredefinedQueries->setToolTip( tr( "Open File" ) );
}

/**
* Destructor
*/
eVisDatabaseConnectionGui::~eVisDatabaseConnectionGui( )
{
}


/*
 *
 * Public and Private Slots
 *
 */

/**
* Slot called after the user selects the x, y fields in the field selection gui component
* @param layerName - Name to display in the legend
* @param xCoordinate - Name of the field containing the x coordinate
* @param yCoordinate - Name of the field containing the y coordinate
*/
void eVisDatabaseConnectionGui::drawNewVectorLayer( QString layerName, QString xCoordinate, QString yCoordinate )
{
  //if coorindate fields are defined, load as a delimited text layer
  if ( !xCoordinate.isEmpty( ) && !yCoordinate.isEmpty( ) && mTempOutputFileList->size( ) > 0 )
  {
    //fileName is only available if the file is open
    //the last file in the list is always the newest
    mTempOutputFileList->last( )->open( );
    QUrl url = QUrl::fromLocalFile( mTempOutputFileList->last()->fileName() );
    url.addQueryItem( "delimiter", "\t" );
    url.addQueryItem( "delimiterType", "regexp" );
    url.addQueryItem( "xField", xCoordinate );
    url.addQueryItem( "yField", yCoordinate );
    emit drawVectorLayer( QString::fromAscii( url.toEncoded() ), layerName, "delimitedtext" );
    mTempOutputFileList->last( )->close( );
  }
}

/**
* Slot called when the accept button is pressed
*/
void eVisDatabaseConnectionGui::on_buttonBox_accepted( )
{
  //Deallocate memory, basically a predescructor
  if ( 0 != mDatabaseConnection )
  {
    mDatabaseConnection->close( );
    delete( mDatabaseConnection );
  }

  if ( 0 != mDatabaseLayerFieldSelector )
  {
    delete( mDatabaseLayerFieldSelector );
  }

  if ( 0 != mQueryDefinitionMap )
  {
    mQueryDefinitionMap->clear();
    delete mQueryDefinitionMap;
  }

  accept( );
}

/**
* Slot called when the cboxDatabaseType combo box index changes
* @param currentIndex - The new index of the currently selected field
*/
void eVisDatabaseConnectionGui::on_cboxDatabaseType_currentIndexChanged( int currentIndex )
{
  Q_UNUSED( currentIndex );
  if ( cboxDatabaseType->currentText( ) == "MYSQL" )
  {
    lblDatabaseHost->setEnabled( true );
    leDatabaseHost->setEnabled( true );
    lblDatabasePort->setEnabled( true );
    leDatabasePort->setText( "3306" );
    leDatabasePort->setEnabled( true );
    pbtnOpenFile->setEnabled( false );
    lblDatabaseUsername->setEnabled( true );
    leDatabaseUsername->setEnabled( true );
    lblDatabasePassword->setEnabled( true );
    leDatabasePassword->setEnabled( true );
    leDatabaseName->setText( "" );
  }
  else if ( cboxDatabaseType->currentText( ) == "PostGreSQL" )
  {
    lblDatabaseHost->setEnabled( true );
    leDatabaseHost->setEnabled( true );
    lblDatabasePort->setEnabled( true );
    leDatabasePort->setText( "5432" );
    leDatabasePort->setEnabled( true );
    pbtnOpenFile->setEnabled( false );
    lblDatabaseUsername->setEnabled( true );
    leDatabaseUsername->setEnabled( true );
    lblDatabasePassword->setEnabled( true );
    leDatabasePassword->setEnabled( true );
    leDatabaseName->setText( "" );
  }
  else if ( cboxDatabaseType->currentText( ) == "SQLITE" || cboxDatabaseType->currentText( ) == "MSAccess" )
  {
    lblDatabaseHost->setEnabled( false );
    leDatabaseHost->setText( "" );
    leDatabaseHost->setEnabled( false );
    lblDatabasePort->setEnabled( false );
    leDatabasePort->setText( "" );
    leDatabasePort->setEnabled( false );
    pbtnOpenFile->setEnabled( true );
    lblDatabaseUsername->setEnabled( false );
    leDatabaseUsername->setText( "" );
    leDatabaseUsername->setEnabled( false );
    lblDatabasePassword->setEnabled( false );
    leDatabasePassword->setText( "" );
    leDatabasePassword->setEnabled( false );
    leDatabaseName->setText( "" );
  }
  else
  {
    lblDatabaseHost->setEnabled( true );
    leDatabaseHost->setEnabled( true );
    lblDatabasePort->setEnabled( false );
    leDatabasePort->setText( "" );
    leDatabasePort->setEnabled( false );
    pbtnOpenFile->setEnabled( false );
    lblDatabaseUsername->setEnabled( true );
    leDatabaseUsername->setEnabled( true );
    lblDatabasePassword->setEnabled( true );
    leDatabasePassword->setEnabled( true );
    leDatabaseName->setText( "" );
  }
}

/**
* Slot called when pbtnConnect button pressed. This function does some basic error checking before
* requesting a new database connection
*/
void eVisDatabaseConnectionGui::on_pbtnConnect_clicked( )
{
  teditConsole->append( tr( "New Database connection requested..." ) );
  bool errors = false;

  if ( cboxDatabaseType->currentText( ) == tr( "Undefined" ) )
  {
    teditConsole->append( tr( "Error: You must select a database type" ) );
    errors = true;
  }

  if ( !errors && ( cboxDatabaseType->currentText( ) == "MYSQL" || cboxDatabaseType->currentText( ) == "PostGreSQL" ) )
  {
    if ( leDatabaseHost->text( ).isEmpty( ) )
    {
      teditConsole->append( tr( "Error: No host name entered" ) );
      errors = true;
    }
  }

  if ( !errors && leDatabaseName->text( ).isEmpty( ) )
  {
    teditConsole->append( tr( "Error: No database name entered" ) );
    errors = true;
  }

  //If no errors thus far, request a new database connection
  if ( !errors )
  {
    eVisDatabaseConnection::DATABASE_TYPE myDatabaseType;
    if ( cboxDatabaseType->currentText( ) == "MSAccess" )
    {
      myDatabaseType = eVisDatabaseConnection::MSACCESS;
    }
    else if ( cboxDatabaseType->currentText( ) == "MYSQL" )
    {
      myDatabaseType = eVisDatabaseConnection::QMYSQL;
    }
    else if ( cboxDatabaseType->currentText( ) == "ODBC" )
    {
      myDatabaseType = eVisDatabaseConnection::QODBC;
    }
    else if ( cboxDatabaseType->currentText( ) == "PostGreSQL" )
    {
      myDatabaseType = eVisDatabaseConnection::QPSQL;
    }
    else
    {
      myDatabaseType = eVisDatabaseConnection::QSQLITE;
    }

    //If there is aready a database connection object, reset with the current parameters
    if ( 0 != mDatabaseConnection )
    {
      mDatabaseConnection->resetConnectionParameters( leDatabaseHost->text( ), leDatabasePort->text( ).toInt( ), leDatabaseName->text( ), leDatabaseUsername->text( ), leDatabasePassword->text( ), myDatabaseType );
    }
    else //create a new database connection object
    {
      mDatabaseConnection = new eVisDatabaseConnection( leDatabaseHost->text( ), leDatabasePort->text( ).toInt( ), leDatabaseName->text( ), leDatabaseUsername->text( ), leDatabasePassword->text( ), myDatabaseType );
    }

    //Try to connect the database connection object
    if ( mDatabaseConnection->connect( ) )
    {
      teditConsole->append( tr( "Connection to [%1.%2] established" ).arg( leDatabaseHost->text() ).arg( leDatabaseName->text() ) );
      lblConnectionStatus->setText( tr( "connected" ) );

      //List the tables in the database
      teditConsole->append( tr( "Tables" ) + ":" );
      QStringList myTableList = mDatabaseConnection->tables( );
      for ( int myIterator = 0; myIterator < myTableList.size( ); myIterator++ )
      {
        teditConsole->append( "->" + myTableList[myIterator] );
      }
    }
    else
    {
      teditConsole->append( tr( "Connection to [%1.%2] failed: %3" )
                            .arg( leDatabaseHost->text() ).arg( leDatabaseName->text( ) ).arg( mDatabaseConnection->lastError() ) );
    }
  }
}

/**
* Slot called when pbtnLoadPredefinedQueries button is pressed. The method will open a file dialog and then
* try to parse through an XML file of predefined queries.
*/
void eVisDatabaseConnectionGui::on_pbtnLoadPredefinedQueries_clicked( )
{
  //There probably needs to be some more error checking, but works for now.

  //Select the XML file to parse
  QString myFilename = QFileDialog::getOpenFileName( this, tr( "Open File" ), ".", "XML ( *.xml )" );
  if ( myFilename != "" )
  {
    //Display the name of the file being parsed
    lblPredefinedQueryFilename->setText( myFilename );

    //If the file exists load it into a QDomDocument
    QFile myInputFile( myFilename );
    if ( myInputFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QString errorString;
      int errorLine;
      int errorColumn;
      QDomDocument myXmlDoc;
      if ( myXmlDoc.setContent( &myInputFile, &errorString, &errorLine, &errorColumn ) )
      {
        //clear any existing query descrptions
        cboxPredefinedQueryList->clear( );
        if ( !mQueryDefinitionMap->empty( ) )
        {
          delete( mQueryDefinitionMap );
          mQueryDefinitionMap = new QMap<int, eVisQueryDefinition>;
        }

        //Loop through each child looking for a query tag
        int myQueryCount = 0;
        QDomNode myNode = myXmlDoc.documentElement( ).firstChild( );
        while ( !myNode.isNull( ) )
        {
          if ( myNode.toElement( ).tagName( ) == "query" )
          {
            bool insert = false;
            eVisQueryDefinition myQueryDefinition;
            QDomNode myChildNodes = myNode.toElement( ).firstChild( );
            while ( !myChildNodes.isNull( ) )
            {
              QDomNode myDataNode = myChildNodes.toElement( ).firstChild( );
              QString myDataNodeContent = "";
              if ( !myDataNode.isNull( ) )
              {
                myDataNodeContent = myDataNode.toText( ).data( );
              }

              if ( myChildNodes.toElement( ).tagName( ) == "shortdescription" )
              {
                if ( myDataNodeContent != "" )
                {
                  myQueryDefinition.setShortDescription( myDataNodeContent );
                  myQueryCount++;
                  insert = true;
                }
              }
              else if ( myChildNodes.toElement( ).tagName( ) == "description" )
              {
                myQueryDefinition.setDescription( myDataNodeContent );
              }
              else if ( myChildNodes.toElement( ).tagName( ) == "databasetype" )
              {
                myQueryDefinition.setDatabaseType( myDataNodeContent );
              }
              else if ( myChildNodes.toElement( ).tagName( ) == "databasehost" )
              {
                myQueryDefinition.setDatabaseHost( myDataNodeContent );
              }
              else if ( myChildNodes.toElement( ).tagName( ) == "databaseport" )
              {
                myQueryDefinition.setDatabasePort( myDataNodeContent.toInt( ) );
              }
              else if ( myChildNodes.toElement( ).tagName( ) == "databasename" )
              {
                myQueryDefinition.setDatabaseName( myDataNodeContent );
              }
              else if ( myChildNodes.toElement( ).tagName( ) == "databaseusername" )
              {
                myQueryDefinition.setDatabaseUsername( myDataNodeContent );
              }
              else if ( myChildNodes.toElement( ).tagName( ) == "databasepassword" )
              {
                myQueryDefinition.setDatabasePassword( myDataNodeContent );
              }
              else if ( myChildNodes.toElement( ).tagName( ) == "sqlstatement" )
              {
                myQueryDefinition.setSqlStatement( myDataNodeContent );
              }

              myChildNodes = myChildNodes.nextSibling( );
            } //end  while( !myChildNodes.isNull( ) )

            if ( insert )
            {
              mQueryDefinitionMap->insert( myQueryCount - 1, myQueryDefinition );
              cboxPredefinedQueryList->insertItem( myQueryCount - 1, myQueryDefinition.shortDescription( ) );
            }
          } //end if( myNode.toElement( ).tagName( ) == "query" )
          myNode = myNode.nextSibling( );
        } // end  while( !myNode.isNull( ) )
      }
      else
      {
        teditConsole->append( tr( "Error: Parse error at line %1, column %2: %3" ).arg( errorLine ).arg( errorColumn ).arg( errorString ) );
      }
    }
    else
    {
      teditConsole->append( tr( "Error: Unabled to open file [%1]" ).arg( myFilename ) );
    }
  }
}

/**
* Slot called when cboxPredefinedQueryList combo box index changes
* @param index - The current index of the selected item
*/
void eVisDatabaseConnectionGui::on_cboxPredefinedQueryList_currentIndexChanged( int index )
{
  if ( !mQueryDefinitionMap->isEmpty( ) )
  {
    //get the query definition at the current index
    //NOTE: not really necessary to check to see if index is out of range from the query definition map because items cannot
    //be added to the combo box unless they are added to the query definition map
    eVisQueryDefinition myQueryDefinition = mQueryDefinitionMap->value( index );

    //Populate the GUI components with the values from the query definition
    teditQueryDescription->setText( myQueryDefinition.description( ) );
    cboxDatabaseType->setCurrentIndex( cboxDatabaseType->findText( myQueryDefinition.databaseType( ) ) );
    leDatabaseHost->setText( myQueryDefinition.databaseHost( ) );
    leDatabasePort->setText( QString( "%1" ).arg( myQueryDefinition.databasePort( ) ) );
    leDatabaseName->setText( myQueryDefinition.databaseName( ) );
    leDatabaseUsername->setText( myQueryDefinition.databaseUsername( ) );
    leDatabasePassword->setText( myQueryDefinition.databasePassword( ) );
    teditSqlStatement->setText( myQueryDefinition.sqlStatement( ) );
  }
}

/**
* Slot called when pbtnOpenFile button is pressed
*/
void eVisDatabaseConnectionGui::on_pbtnOpenFile_clicked( )
{
  if ( cboxDatabaseType->currentText( ) == "MSAccess" )
    leDatabaseName->setText( QFileDialog::getOpenFileName( this, tr( "Open File" ), ".", "MSAccess ( *.mdb )" ) );
  else
    leDatabaseName->setText( QFileDialog::getOpenFileName( this, tr( "Open File" ), ".", "Sqlite ( *.db )" ) );
}

/**
* Slot called when the pbtnRunQuery button is pressed
*/
void eVisDatabaseConnectionGui::on_pbtnRunQuery_clicked( )
{
  //Check to see if we have a query
  if ( !teditSqlStatement->toPlainText( ).isEmpty( ) )
  {
    //Verify that we have an active database connection
    if ( 0 != mDatabaseConnection )
    {
      //Execute query
      QSqlQuery* myResults = mDatabaseConnection->query( teditSqlStatement->toPlainText( ) );
      if ( 0 == myResults )
      {
        teditConsole->append( tr( "Error: Query failed: %1" ).arg( mDatabaseConnection->lastError( ) ) );
      }
      else if ( myResults->isSelect( ) )
      {
        //if valid and a select query, save results into temporary file and load as layer
        myResults->next( );
        if ( myResults->isValid( ) )
        {
          mTempOutputFileList->append( new QTemporaryFile( ) );
          if ( mTempOutputFileList->last( )->open( ) )
          {
            QTextStream outputStream( mTempOutputFileList->last( ) );
            QStringList fieldList;
            /*
             * Output column names
             */
            for ( int x = 0; x < myResults->record( ).count( ); x++ )
            {
              if ( 0 == x )
              {
                outputStream << myResults->record( ).fieldName( x );
              }
              else
              {
                outputStream << "\t" << myResults->record( ).fieldName( x );
              }
              fieldList << myResults->record( ).fieldName( x );
            }
            outputStream << endl;
            /*
             * Output Data
             */
            while ( myResults->isValid( ) )
            {
              for ( int x = 0; x < myResults->record( ).count( ); x++ )
              {
                if ( x == 0 )
                {
                  outputStream << myResults->value( x ).toString( );
                }
                else
                {
                  outputStream << "\t" << myResults->value( x ).toString( );
                }
              }
              outputStream << endl;
              myResults->next( );
            }
            mTempOutputFileList->last( )->close( );
            mDatabaseLayerFieldSelector->setFieldList( &fieldList );
            mDatabaseLayerFieldSelector->show( );
          }
          else
          {
            teditConsole->append( tr( "Error: Could not create temporary file, process halted" ) );
          }
        }
      }
    }
    else
    {
      teditConsole->append( tr( "Error: A database connection is not currently established" ) );
    }
  }
}
